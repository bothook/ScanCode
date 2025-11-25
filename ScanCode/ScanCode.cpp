#include "ScanCode.h"

#include "tchar.h"
#include "sqlite3.h"

#include "RibbonIntelPaperWsServiceServiceSoapBinding.nsmap"

#if _DEBUG //利用vld测试内存泄漏
#include "vld.h"
#pragma comment(lib,"jqshk2.d.lib")
#pragma comment(lib,"DHCamera3.d.lib")
#pragma comment(lib,"AzSfWs_Login.d.lib")
#else
#pragma comment(lib,"jqshk2.lib")
#pragma comment(lib,"DHCamera3.lib")
#pragma comment(lib,"AzSfWs_Login.lib")
#endif
#pragma comment(lib,"sqlite3.lib")

#include "JQSHK2.h"
#include "LogDlg.h"
#include "SettingDlg.h"
#include "CameraDlg.h"
#include "login.h"


#include <QWindow>
#include <QFileInfo>
#include <QMenuBar>
#include <QAction>
#include <QDateTime>
#include <QApplication>
#include <QVBoxLayout>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QLabel>
#include <QSettings>
#include <QLineEdit>
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
	initSql();
	saveLog("软件打开");
	initConfig();
	initShk();
	initCamera();
	m_workthread = new WorkThread(this);

	resize(400, 250);
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
	inputLayout->setContentsMargins(50, 30, 50, 10);
	m_labelPaper = new QLabel("标签料号:", inputWidget);
	m_ribbonNum = new QLabel("碳带:", inputWidget);
	inputLayout->addWidget(m_labelPaper);
	inputLayout->addWidget(m_ribbonNum);
	inputLayout->addStretch();
	inputWidget->setLayout(inputLayout);
	//按钮界面
	QVBoxLayout* btnLayout = new QVBoxLayout(btnWidget);
	btnLayout->setContentsMargins(50, 30, 50, 10);
	QHBoxLayout* printBtnLayout = new QHBoxLayout();
	m_printBtn = new QPushButton("开始", this);
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
			m_printBtn->setText("暂停");
			return;
		}
		Thread::State state = m_workthread->state();
		switch (state)
		{
		case Thread::Running:
			m_printBtn->setText("恢复");
			m_workthread->pause();
			break;
		case Thread::Paused:
			m_printBtn->setText("暂停");
			m_workthread->resume();
			break;
		case Thread::Stoped:
			m_printBtn->setText("暂停");
			m_workthread->start();
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
	if(m_printBtn->text()=="暂停")
		m_printBtn->click();
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
	m_camHandle = ::DHCreate(_T("Camera"));
	Sleep(100);
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
	saveLog("串口连接成功");
	return true;
}

bool ScanCode::getShkString()
{
	photoGraph();
	::JQSHKExecute();
	LPTSTR lptstr = new TCHAR[255];
	::JQSHKGetString(_T("code"), lptstr);
	m_codeFromShk = QString::fromWCharArray(lptstr);
	static int times = 0;
	delete[] lptstr;
	if (m_codeFromShk.isEmpty())
	{
		++times;
		if (times > 2)
		{
			times = 0;
			return false;
		}
		return getShkString();
	}
	times = 0;
	return true;
}

void ScanCode::photoGraph()
{
	if (!m_camHandle)
		throw runtime_error(QString("拍照失败：相机不存在").toLocal8Bit());
	if(!::DHSnapImage(m_camHandle,_T("Camera.bmp")))
	{
		QString strMsg = QString::fromWCharArray(::DHGetError());
		strMsg = "拍照失败：" + strMsg;
		saveLog(strMsg);
		throw runtime_error(strMsg.toLocal8Bit());
	}
}

void ScanCode::read4Com()
{
	m_codeFromCom.clear();
	QByteArray buffer = m_serial->readAll();
	m_codeFromCom = buffer;
	if (m_codeFromCom.isEmpty()) 
	{
		write2Com("-");
		return;
	}
}

QString ScanCode::write2Com(const QByteArray data)
{
	if (!m_serial->isOpen())
		return QString("串口连接断开");
	m_serial->write(data);
	return 0;
}

QString ScanCode::getToken()
{
	AzSfWs_Login_SetUrl(reinterpret_cast<LPCWSTR>(m_setting.loginUrl.utf16()));
	AzSfWs_Login_SetTimeout(5000,5000,5000);
	if (AzSfWs_Login(reinterpret_cast<LPCWSTR>(m_setting.loginAc.utf16()), 
		reinterpret_cast<LPCWSTR>(m_setting.loginPas.utf16())) != 0)
	{
		QString err = QString::fromWCharArray(AzSfWs_Login_GetLastError());
		throw runtime_error(err.toLocal8Bit());
	}
	return QString::fromWCharArray(AzSfWs_Login_Token()).toLocal8Bit();
}

void ScanCode::pushData()
{
	ns1__ribbonPaperIn ribbonPaperIn;
	shared_ptr<ns1__ribbonIntelPaperRequest>ribbonIntelPaperRequest(new ns1__ribbonIntelPaperRequest);
	ribbonPaperIn.ribbonPaperInRequest = ribbonIntelPaperRequest.get();

	QByteArray hostName = m_setting.machine.toUtf8();
	ribbonIntelPaperRequest->hostname = hostName.data();

	QByteArray labelPaper = m_codeFromCom.toUtf8();
	ribbonIntelPaperRequest->labelPaper = labelPaper.data();

	QByteArray ribbonNum = m_codeFromShk.toUtf8();
	ribbonIntelPaperRequest->ribbonNum = ribbonNum.data();

	QByteArray token = getToken().toUtf8();
	ribbonIntelPaperRequest->token = token.data();

	QByteArray userName = m_setting.loginAc.toUtf8();
	ribbonIntelPaperRequest->username = userName.data();

	QByteArray tcp = m_setting.url.toUtf8();
	RibbonIntelPaperWsServiceServiceSoapBindingProxy service(tcp.data(), SOAP_C_UTFSTRING);
	ns1__ribbonPaperInResponse response;
	int nResult = service.ribbonPaperIn(&static_cast<ns1__ribbonPaperIn>(ribbonPaperIn), response);
	if (nResult == SOAP_OK)
	{
		QString upLog = QString("上传记录机台名:%1,标签料号:%2,碳带号:%3,登录用户名:%4,url:%5")
			.arg(m_setting.machine)
			.arg(m_codeFromCom)
			.arg(m_codeFromShk)
			.arg(m_setting.loginAc)
			.arg(m_setting.url);
		saveLog(upLog);
		if (!*response.ribbonPaperInResponse->result) 
		{
			QString error_message(response.ribbonPaperInResponse->error_USCOREmessage);
			throw runtime_error(error_message.toLocal8Bit());
		}
	}
	else 
	{
		throw runtime_error(QString("连接服务器失败,错误编号：%1").arg(nResult).toLocal8Bit());
	}
}

bool ScanCode::getSignal()
{
	ns1__ribbonPaperCheck ribbonPaperCheck;
	shared_ptr<ns1__ribbonPaperCheckRequest> ribbonPaperCheckRequest(new ns1__ribbonPaperCheckRequest);
	ribbonPaperCheck.ribbonPaperCheckRequest = ribbonPaperCheckRequest.get();
	QByteArray hostName = m_setting.printer.toUtf8();
	ribbonPaperCheckRequest->hostName = hostName.data();
	QByteArray tcp = m_setting.url.toUtf8();
	RibbonIntelPaperWsServiceServiceSoapBindingProxy service(tcp.data(), SOAP_C_UTFSTRING);
	ns1__ribbonPaperCheckResponse response;
	int nResult = service.ribbonPaperCheck(&static_cast<ns1__ribbonPaperCheck>(ribbonPaperCheck), response);
	if (nResult == SOAP_OK)
	{
		if (!*response.ribbonPaperCheckResponse->result)
			return false;
	}
	else
	{
		throw runtime_error(QString("连接服务器失败,错误编号：%1").arg(nResult).toLocal8Bit());
	}
	return true;
}

void ScanCode::errMsg(QString errStr)
{
	saveLog(errStr);
	QMessageBox::information(this, "警告", errStr);
	m_printBtn->setText("恢复");
}

void ScanCode::setUIinfo()
{
	m_labelPaper->setText("标签料号:" + m_codeFromCom);
	m_ribbonNum->setText("碳带:" + m_codeFromShk);
}

void WorkThread::getComString()
{
	emit writeComInfo("+");
	QTimer::singleShot(m_setting.delay, m_scanCode, &ScanCode::read4Com);
	Sleep(m_setting.delay+100);
}

void WorkThread::process()
{
	if (!m_scanCode->getSignal())
	{
		Sleep(1000);
		return;
	}
	bool success = m_scanCode->getShkString();
	getComString();
	emit setUIinfo();
	if (!success)
		throw runtime_error(QString("二次解码仍未获得数据").toLocal8Bit());
	if (m_scanCode->m_codeFromCom.isEmpty())
		throw runtime_error(QString("扫码枪未扫到数据或已断开连接").toLocal8Bit());
	m_scanCode->pushData();
}


