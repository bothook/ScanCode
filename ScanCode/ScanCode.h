#pragma once
#include <QtWidgets/QMainWindow>
#pragma execution_character_set("utf-8")
#include "windows.h"
#include <algorithm>
#include <memory>
using namespace std;

class SettingDlg;
class CameraDlg;
class LogDlg;
class WorkThread;
class QSerialPort;
class QPushButton;
extern void executeSQL(string sql, vector<pair<string, string>>& logData 
	= *make_shared<vector<pair<string, string>>>());
extern void saveLog(const QString logContent);
typedef HANDLE DHANDLE;
class ScanCode : public QMainWindow
{
	Q_OBJECT
public:
	friend class WorkThread;
	ScanCode(QWidget *parent = Q_NULLPTR);
	~ScanCode();
private:
	void createMenu(QStringList menuList);
	void showDlg(int index);
	void initSql();
	void initShk();
	void initCamera();
	bool initComport();
protected:
	void getCodeString();
	void photoGraph();
	void read4Com();
	QString write2Com(const QByteArray data);
	void pushData();
	void errMsg(QString errStr);
	void getComInfo(QString &errmsg);
	enum COM
	{
		ribbon,
		label
	};
private:
	QMenuBar* m_menuBar = nullptr;
	HWND m_imgWindow = nullptr;
	HWND m_camWindow = nullptr;
	DHANDLE m_camHandle = nullptr;
	SettingDlg* m_settingDlg = nullptr;
	CameraDlg* m_cameraDlg = nullptr;
	LogDlg* m_logDlg = nullptr;
	WorkThread* m_workthread = nullptr;
	QPushButton* m_printBtn = nullptr;

	QSerialPort* m_serial = nullptr;
	QString m_workOrder;
	QString m_codeFromShk;
	QString m_codeFromCom;
};

#include "Thread.h"
class QTimer;
class WorkThread : public Thread 
{
	Q_OBJECT
public:
	WorkThread(QWidget *parent = Q_NULLPTR)
		:m_scanCode((ScanCode*)parent), Thread(parent) {
		connect(this, &WorkThread::errorStr, m_scanCode, &ScanCode::errMsg);
	};
protected:
	void process()override;
private:
	void comWork();
private:
	ScanCode* m_scanCode = nullptr;
signals:
	void getComInfo(QString &errmsg);
};