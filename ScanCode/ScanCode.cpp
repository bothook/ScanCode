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
#include "LogDlg.h"
#include "SettingDlg.h"
#include "DHCamera.h"
ScanCode::ScanCode(QWidget *parent)
    : QMainWindow(parent)
{
	initSql();
	saveLog("软件打开");
	initConfig();
	resize(800, 500);
	QWidget* centralWidget = new QWidget(this);
	setCentralWidget(centralWidget);
	// 菜单栏
	QMenuBar* menuBar = new QMenuBar(this);
	setMenuBar(menuBar);
	QAction* settingMenu = new QAction("设置", menuBar);
	menuBar->addAction(settingMenu);
	connect(settingMenu, &QAction::triggered, this, &ScanCode::showSettingDlg);
	QAction* logMenu = new QAction("日志", menuBar);
	menuBar->addAction(logMenu);
	connect(logMenu, &QAction::triggered, this, &ScanCode::showLogDlg);
	QVBoxLayout* layout = new QVBoxLayout(centralWidget);
	QHBoxLayout* Hlayout = new QHBoxLayout(centralWidget);
	m_shkWindow = new ShkImageWindow(centralWidget);
	m_cameraWindow = new CameraWindow(centralWidget);
	m_comPortWindow = new ComPortWindow(centralWidget);
	Hlayout->addWidget(m_shkWindow);
	Hlayout->addWidget(m_cameraWindow);
	layout->addLayout(Hlayout);
	layout->addWidget(m_comPortWindow);
	layout->addStretch();
	centralWidget->setLayout(layout);
	//QString str;
	//m_shkWindow->getCodeString(str);
	//saveLog(str);
}

ScanCode::~ScanCode()
{
	saveLog("软件关闭");
}

void ScanCode::showLogDlg()
{
	if (!m_logDlg) 
		m_logDlg = new LogDlg(this);
	m_logDlg->show();
}

void ScanCode::showSettingDlg()
{
	if (!m_settingDlg)
		m_settingDlg = new SettingDlg(this);
	m_settingDlg->exec();
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
		"SELECT MAX(0, (SELECT COUNT(*) FROM LOG) - 1000) "//满1000条迭代最旧的
		") "
		"); "
		"END;";
	executeSQL(sql);
}

void ScanCode::initConfig()
{
	QString filename = "config.ini";
	QFile file(filename);
	if (!file.exists()) 
	{
		QSettings settings(filename, QSettings::IniFormat);
		settings.beginGroup("Default");
		settings.setValue("Machine", "");
		settings.setValue("ComPort", "");
		settings.setValue("URL", "");
		settings.endGroup();
	}
}

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
			logData.push_back(make_pair(date,content));
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

ShkImageWindow::ShkImageWindow(QWidget *parent /*= Q_NULLPTR*/)
	: QWidget(parent)
{
	::JQSHKInitialize();
	UINT ss = -1 - JQSHK_SONAME_CHOOSE - JQSHK_MENU_CAMERA_CHOOSE;
	HWND imgWindow = ::JQSHKGetImageWindow(_T("imgA"), ss);
	::JQSHKShowToolBar(imgWindow, false);
	::JQSHKSetStyle(imgWindow, 0, JQSHK_MENU_ROI);
	QWindow* qwindow = QWindow::fromWinId((WId)imgWindow);
	QWidget* container = QWidget::createWindowContainer(qwindow, this);
	container->setFixedSize(qwindow->width(), qwindow->height());
	container->show();
	setFixedSize(container->width(), container->height());
	QFileInfo file("Code.ivs");
	QString filePath = file.absoluteFilePath();
	filePath.replace("/", "\\");
	auto path = filePath.toStdWString();
	LPCTSTR lpszFile = path.c_str();
	::JQSHKLoad(lpszFile);
}

ShkImageWindow::~ShkImageWindow()
{
	::JQSHKSetMode(JQSHK_MODE_HALT);
	::JQSHKUninitialize();
}

void ShkImageWindow::getCodeString(QString& str)
{
	::JQSHKExecute();
	LPTSTR lptstr = new TCHAR[255];
	::JQSHKGetString(_T("code"), lptstr);
	str = QString::fromWCharArray(lptstr);
	delete[] lptstr;
}

CameraWindow::CameraWindow(QWidget *parent /*= Q_NULLPTR*/)
	: QWidget(parent)
{
	QString strCameraName = "Camera";
	wchar_t* wchar_tCameraName = new wchar_t[strCameraName.length() + 1];
	strCameraName.toWCharArray(wchar_tCameraName);
	wchar_tCameraName[strCameraName.length()] = L'\0';
	m_cameraHandle = ::DHCreate(wchar_tCameraName);
	delete[] wchar_tCameraName;
	Sleep(100);
	if (!m_cameraHandle)
	{
		QString strErr = QString::fromWCharArray(::DHGetError());
		saveLog(strErr);
		QMessageBox::warning(nullptr, "相机", strErr);
		return;
	}
	HWND cameraWindow = ::DHGetVideoHwnd(m_cameraHandle);
	QWindow* qwindow = QWindow::fromWinId((WId)cameraWindow);
	QWidget* container = QWidget::createWindowContainer(qwindow, this);
	container->setFixedSize(qwindow->width(), qwindow->height());
	container->show();
	setFixedSize(container->width(), container->height());
}

CameraWindow::~CameraWindow()
{
	if (m_cameraHandle)
		::DHClose(m_cameraHandle);
}

bool CameraWindow::photoGraph()
{
	if (!m_cameraHandle)
		return FALSE;
	QFileInfo file("photo.jpg");
	QString filePath = file.absoluteFilePath();
	filePath.replace("/", "\\");
	auto path = filePath.toStdWString();
	LPCTSTR photoPath = path.c_str();
	if (!::DHSnapImage(m_cameraHandle, photoPath))
	{
		QString strMsg = QString::fromWCharArray(::DHGetError());
		strMsg = "拍照失败：" + strMsg;
		saveLog(strMsg);
		QMessageBox::warning(nullptr, "相机", strMsg);
		return FALSE;
	}
	::Sleep(100);
	return TRUE;
}

ComPortWindow::ComPortWindow(QWidget *parent /*= Q_NULLPTR*/)
	: QWidget(parent)
{
	QHBoxLayout* codeLayout = new QHBoxLayout(this);
	QLabel* m_codeTitle = new QLabel("扫码枪数据：",this);
	m_codeContent = new QLabel(this);
	codeLayout->addWidget(m_codeTitle);
	codeLayout->addWidget(m_codeContent);
	setLayout(codeLayout);
	codeLayout->addStretch();
	m_serial = new QSerialPort(this);
	QSettings settings("config.ini", QSettings::IniFormat);
	m_serial->setPort(QSerialPortInfo(settings.value("Default/ComPort").toString()));
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
		return;
	}
	connect(m_serial, &QSerialPort::readyRead, this, &ComPortWindow::readComPort);
	saveLog("串口连接成功");
}

ComPortWindow::~ComPortWindow()
{
	m_serial->close();
}

void ComPortWindow::readComPort()
{
	m_codeFromCom = m_serial->readAll();
	resolveCode();
	m_codeContent->setText(m_codeFromCom);
}

void ComPortWindow::writeComPort(const QByteArray data)
{
	m_serial->write(data);
}

void ComPortWindow::resolveCode()
{

}
