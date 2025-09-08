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

#include "LogDlg.h"
ScanCode::ScanCode(QWidget *parent)
    : QMainWindow(parent)
{
	initSql();
	//initCamera();
	resize(800, 500);
	QWidget* centralWidget = new QWidget(this);
	setCentralWidget(centralWidget);
	// 菜单栏
	QMenuBar* menuBar = new QMenuBar(this);
	setMenuBar(menuBar);
	QMenu* settingMenu = new QMenu("设置", this);
	menuBar->addMenu(settingMenu);
	QAction* logMenu = new QAction("日志", this);
	menuBar->addAction(logMenu);
	connect(logMenu, &QAction::triggered, this, &ScanCode::showLogDlg);
	m_shkWindow = new ShkImageWindow(centralWidget);
	QString str;
	m_shkWindow->getCodeString(str);
	//saveLog(str);
}

//ScanCode::~ScanCode()
//{
//	if(m_cameraHandle)
//		::DHClose(m_cameraHandle);
//}

void ScanCode::showLogDlg()
{
	if (!m_logDlg) 
		m_logDlg = new LogDlg(this);
	m_logDlg->show();
}

void ScanCode::saveLog(QString logContent)
{
	QDateTime currentTime = QDateTime::currentDateTime();
	QString time = currentTime.toString("yyyy/MM/dd hh:mm:ss");
	QString sql = QString("INSERT INTO LOG(TIME,MESSAGE)"
		"VALUES('%1', '%2')").arg(time).arg(logContent);
	executeSQL(sql.toStdString());
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

//bool ScanCode::initCamera()
//{
//	QString strCameraName = "Camera";
//	wchar_t* wchar_tCameraName = new wchar_t[strCameraName.length() + 1];
//	strCameraName.toWCharArray(wchar_tCameraName);
//	wchar_tCameraName[strCameraName.length()] = L'\0';
//	m_cameraHandle = ::DHCreate(wchar_tCameraName);
//	delete[] wchar_tCameraName;
//	Sleep(100);
//	if (m_cameraHandle == NULL)
//	{
//		QString strErr = QString::fromWCharArray(::DHGetError());
//		QMessageBox::warning(nullptr, "警告", strErr);
//		return FALSE;
//	}
//	return TRUE;
//}

extern void executeSQL(string sql, vector<pair<string, string>>& logData)
{
	static sqlite3* m_db;
	char* errMessage = 0;
	int rc;
	// 打开数据库
	rc = sqlite3_open("DATA.db", &m_db);
	if (rc) {
	}
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
	{
		rc = sqlite3_exec(m_db, sql.c_str(), 0, 0, &errMessage);
	}
	// 关闭数据库连接
	sqlite3_close(m_db);
	if (rc != SQLITE_OK) {
	}
}

ShkImageWindow::ShkImageWindow(QWidget *parent /*= Q_NULLPTR*/) : QWidget(parent)
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
}

ShkImageWindow::~ShkImageWindow()
{
	::JQSHKSetMode(JQSHK_MODE_HALT);
	::JQSHKUninitialize();
}

void ShkImageWindow::getCodeString(QString& str)
{
	QFileInfo file("Code.ivs");
	QString filePath = file.absoluteFilePath();
	filePath.replace("/", "\\");
	auto path = filePath.toStdWString();
	LPCTSTR lpszFile = path.c_str();
	::JQSHKLoad(lpszFile);
	::JQSHKExecute();
	LPTSTR lptstr = new TCHAR[255];
	::JQSHKGetString(_T("code"), lptstr);
	str = QString::fromWCharArray(lptstr);
	delete[] lptstr;
}