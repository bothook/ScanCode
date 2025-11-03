#include "ScanCode.h"
#include "JQSHK2.h"

#include "tchar.h"
#include "sqlite3.h"

#if _DEBUG //利用vld测试内存泄漏
#include "vld.h"
#pragma comment(lib,"jqshk2.d.lib")
#pragma comment(lib,"DHCamera.d.lib")
#else
#pragma comment(lib,"jqshk2.lib")
#pragma comment(lib,"DHCamera.lib")
#endif
#pragma comment(lib,"sqlite3.lib")

#include <QWindow>
#include <QFileInfo>
#include <QMenuBar>
#include <QAction>
#include <QDateTime>
#include <QMessageBox>
#include <QApplication>
#include <QVBoxLayout>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QLabel>
#include <QSettings>
#include <QLineEdit>
#include <QPushButton>
#include "LogDlg.h"
#include "SettingDlg.h"
#include "CameraDlg.h"
#include "DHCamera.h"
#include <QTimer>
#include <QDebug>


extern void executeSQL(string sql, vector<pair<string, string>>& logData)
{
	static sqlite3* m_db;
	char* errMessage = 0;
	int rc;
	// 打开数据库
	rc = sqlite3_open("DATA.db", &m_db);
	if (sql.substr(0, 6) == "SELECT")
	{
		sqlite3_stmt *stmt;
		rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, NULL);
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			const char *date = (const char*)sqlite3_column_text(stmt, 1);  // 读取第1列（日期）
			const char *content = (const char*)sqlite3_column_text(stmt, 2);  // 读取第2列（日志内容）
			logData.push_back(make_pair(date, content));
		}
		sqlite3_finalize(stmt);
	}
	else
		rc = sqlite3_exec(m_db, sql.c_str(), 0, 0, &errMessage);
	// 关闭数据库连接
	sqlite3_close(m_db);
	if (rc != SQLITE_OK) {
		saveLog(errMessage);
	}
}

extern void saveLog(const QString logContent)
{
	QDateTime currentTime = QDateTime::currentDateTime();
	QString time = currentTime.toString("yyyy/MM/dd hh:mm:ss");
	QString sql = QString("INSERT INTO LOG(TIME,MESSAGE)"
		"VALUES('%1', '%2')").arg(time).arg(logContent);
	executeSQL(sql.toStdString());
}

SETTING m_setting;
ScanCode::ScanCode(QWidget *parent)
    : QMainWindow(parent)
{
	saveLog("软件打开");
	initSql();
	initConfig();
	//initShk();
	//initCamera();
	m_workthread = new WorkThread(this);

	resize(800, 500);
	QWidget* centralWidget = new QWidget(this);
	setCentralWidget(centralWidget);
	// 菜单栏
	m_menuBar = new QMenuBar(this);
	setMenuBar(m_menuBar);
	QStringList menuList = { "设置","相机","日志" };
	createMenu(menuList);

	QHBoxLayout* layout = new QHBoxLayout(centralWidget);
	QWidget* inputWidget = new QWidget(centralWidget);
	QWidget* btnWidget = new QWidget(centralWidget);
	layout->addWidget(inputWidget);
	layout->addWidget(btnWidget);
	centralWidget->setLayout(layout);
	//输入框界面
	QVBoxLayout* inputLayout = new QVBoxLayout(inputWidget);
	inputLayout->setContentsMargins(50, 30, 0, 10);
	QHBoxLayout* workOrderLayout = new QHBoxLayout();
	QLabel* workOrderTitle = new QLabel("工单号:", inputWidget);
	QLineEdit* workOrderEdit = new QLineEdit(inputWidget);
	workOrderEdit->setFixedWidth(150);
	workOrderLayout->addWidget(workOrderTitle);
	workOrderLayout->addWidget(workOrderEdit);
	workOrderLayout->addStretch();
	inputLayout->addLayout(workOrderLayout);
	inputLayout->addStretch();
	inputWidget->setLayout(inputLayout);
	//按钮界面
	QVBoxLayout* btnLayout = new QVBoxLayout(btnWidget);
	btnLayout->setContentsMargins(50, 30, 50, 10);
	QHBoxLayout* printBtnLayout = new QHBoxLayout();
	m_printBtn = new QPushButton("开始打印", this);
	m_printBtn->setFixedSize(200, 100);
	connect(m_printBtn, &QPushButton::clicked, [=] 
	{
		if (!initComport())
			return;
		static bool workThread = false;
		if (!workThread) 
		{
			workThread = true;
			m_workthread->start();
			m_printBtn->setText("暂停打印");
			return;
		}
		Thread::State state = m_workthread->state();
		switch (state)
		{
		case Thread::Running:
			m_printBtn->setText("恢复打印");
			m_workthread->pause();
			break;
		case Thread::Paused:
			m_printBtn->setText("暂停打印");
			m_workOrder = workOrderEdit->text();
			m_workthread->resume();
			break;
		}
	});
	printBtnLayout->addWidget(m_printBtn);
	printBtnLayout->addStretch();
	btnLayout->addLayout(printBtnLayout);
	btnLayout->addStretch();
	btnWidget->setLayout(btnLayout);
}

ScanCode::~ScanCode()
{
	if (::JQSHKIsValid()) 
	{
		::JQSHKSetMode(JQSHK_MODE_HALT);
		::JQSHKUninitialize();
	}
	if (m_camHandle)
		::DHClose(m_camHandle);
	if(m_serial)
		m_serial->close();
	saveLog("软件关闭");
}

void ScanCode::createMenu(QStringList menuList)
{
	for (auto it : menuList) 
	{
		QAction* action = new QAction(it, m_menuBar);
		m_menuBar->addAction(action);
		connect(action, &QAction::triggered, [=]
		{
			showDlg(menuList.indexOf(it));
		});
	}
}

void ScanCode::showDlg(int index)
{
	switch (index)
	{
	case 0:
		if (!m_settingDlg)
			m_settingDlg = new SettingDlg(this);
		m_settingDlg->exec();
		break;
	case 1:
		if(!m_cameraDlg)
		{
			QVector<HWND> windows = { m_camWindow, m_imgWindow };
			m_cameraDlg = new CameraDlg(windows, this); 
		}
		m_cameraDlg->show();
		break;
	case 2:
		if (!m_logDlg)
			m_logDlg = new LogDlg(this);
		m_logDlg->exec();
		break;
	default:
		break;
	}
}

void ScanCode::initSql()
{
	string sql = "CREATE TABLE IF NOT EXISTS LOG ("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
		"TIME			TEXT	NOT NULL,"
		"MESSAGE		TEXT	);";
	executeSQL(sql);
	sql = "CREATE TRIGGER IF NOT EXISTS limit_log_rows "  // 注意结尾空格
		"AFTER INSERT ON LOG "                          // 保持空格分隔
		"BEGIN "
		"DELETE FROM LOG "
		"WHERE ID IN ( "
		"SELECT ID FROM LOG "
		"ORDER BY ID ASC "
		"LIMIT ( "
		"SELECT MAX(0, (SELECT COUNT(*) FROM LOG) - 10000) "//满10000条迭代最旧的
		") "
		"); "
		"END;";
	executeSQL(sql);
}

void ScanCode::initShk()
{
	::JQSHKInitialize();
	UINT ss = JQSHK_STYLE_DEFAULT - JQSHK_DISABLE_HIDE - JQSHK_SONAME_CHOOSE;
	m_imgWindow = ::JQSHKGetImageWindow(_T("imgA"), ss);
	QFileInfo file("Code.ivs");
	QString filePath = file.absoluteFilePath();
	filePath.replace("/", "\\");
	auto path = filePath.toStdWString();
	LPCTSTR lpszFile = path.c_str();
	::JQSHKLoad(lpszFile);
}

void ScanCode::initCamera()
{
	QString strCameraName = "Camera";
	wchar_t* wchar_tCameraName = new wchar_t[strCameraName.length() + 1];
	strCameraName.toWCharArray(wchar_tCameraName);
	wchar_tCameraName[strCameraName.length()] = L'\0';
	m_camHandle = ::DHCreate(wchar_tCameraName);
	Sleep(100);
	delete[] wchar_tCameraName;
	if (!m_camHandle)
	{
		QString strErr = QString::fromWCharArray(::DHGetError());
		saveLog(strErr);
		QMessageBox::warning(nullptr, "相机", strErr);
		return;
	}
	m_camWindow = ::DHGetVideoHwnd(m_camHandle);
}

bool ScanCode::initComport()
{
	if (!m_serial) 
		m_serial = new QSerialPort(this);
	QString portName = "COM" + QString::number(m_setting.com);
	if (m_serial->portName() == portName && m_serial->isOpen())
		return true;
	m_serial->close();
	m_serial->setPort(QSerialPortInfo(portName));
	//设置波特率
	m_serial->setBaudRate(QSerialPort::Baud9600);
	//设置奇偶校验位
	m_serial->setParity(QSerialPort::Parity(0));
	//设置停止位
	m_serial->setStopBits(QSerialPort::OneStop);
	if (!m_serial->open(QIODevice::ReadWrite))
	{
		QString setErr = "串口打开失败";
		saveLog(setErr);
		QMessageBox::warning(nullptr, "串口", setErr);
		return false;
	}
	m_serial->setRequestToSend(true);
	connect(m_workthread, &WorkThread::getComInfo, this, &ScanCode::getComInfo);
	saveLog("串口连接成功");
	return true;
}

void ScanCode::getCodeString()
{
	::JQSHKExecute();
	LPTSTR lptstr = new TCHAR[255];
	::JQSHKGetString(_T("code"), lptstr);
	m_codeFromShk = QString::fromWCharArray(lptstr);
	saveLog(QString("解码获得的数据为：")+ m_codeFromShk);
	delete[] lptstr;
}

void ScanCode::photoGraph()
{
	if (!m_camHandle) 
	{
		throw runtime_error(QString("拍照失败：相机不存在").toLocal8Bit());
	}
	QFileInfo file("photo.jpg");
	QString filePath = file.absoluteFilePath();
	filePath.replace("/", "\\");
	auto path = filePath.toStdWString();
	LPCTSTR photoPath = path.c_str();
	if (!::DHSnapImage(m_camHandle, photoPath))
	{
		QString strMsg = QString::fromWCharArray(::DHGetError());
		strMsg = "拍照失败：" + strMsg;
		saveLog(strMsg);
		throw runtime_error(strMsg.toLocal8Bit());
	}
	::Sleep(100);
}

void ScanCode::read4Com()
{
	QByteArray buffer = m_serial->readAll();
	m_codeFromCom = buffer;
	if (m_codeFromCom.isEmpty()) 
	{
		write2Com("-");
	}
}

QString ScanCode::write2Com(const QByteArray data)
{
	if (!m_serial->isOpen())
		return QString("串口连接断开");
	m_serial->write(data);
	return 0;
}

void ScanCode::pushData()
{
	m_setting.machine;
	m_workOrder;
	m_codeFromCom;
	m_codeFromShk;
	QString err = "系统比对料号准确性";
	throw runtime_error(err.toLocal8Bit());
}

void ScanCode::errMsg(QString errStr)
{
	saveLog(errStr);
	QMessageBox::information(this, "警告", errStr);
	m_printBtn->setText("恢复打印");
}

void ScanCode::getComInfo(QString &errmsg)
{
	errmsg = write2Com("+");
}

void WorkThread::comWork()
{
	QString errmsg;
	emit getComInfo(errmsg);
	if (!errmsg.isEmpty())
		throw runtime_error(errmsg.toLocal8Bit());
	QTimer::singleShot(m_setting.delay, m_scanCode, &ScanCode::read4Com);
	Sleep(m_setting.delay);
	if (m_scanCode->m_codeFromCom.isEmpty())
		throw runtime_error(QString("扫码枪未扫到数据或已断开连接").toLocal8Bit());
}

void WorkThread::process()
{
	m_scanCode->photoGraph();
	m_scanCode->getCodeString();
	comWork();
	m_scanCode->pushData();
}


