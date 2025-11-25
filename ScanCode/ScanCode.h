#pragma once
#include <QtWidgets/QMainWindow>
#pragma execution_character_set("utf-8")
#include "soapH.h"
#include "soapRibbonIntelPaperWsServiceServiceSoapBindingProxy.h"
#include "DHCamera.h"
#include "windows.h"
#include <algorithm>
#include <memory>
#include <QPushButton>
#include <QMessageBox>


using namespace std;

class SettingDlg;
class CameraDlg;
class LogDlg;
class WorkThread;
class QSerialPort;
class QLabel;
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
	bool getShkString();
	void photoGraph();
	void read4Com();
	QString write2Com(const QByteArray data);
	void setUIinfo();

	QString getToken();
	void pushData();
	bool getSignal();

	void errMsg(QString errStr);
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
	QLabel* m_labelPaper = nullptr;
	QLabel* m_ribbonNum = nullptr;

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
		connect(this, &WorkThread::setUIinfo, m_scanCode, &ScanCode::setUIinfo);
		connect(this, &WorkThread::writeComInfo, m_scanCode, &ScanCode::write2Com);
	};
protected:
	void process()override;
private:
	void getComString();
private:
	ScanCode* m_scanCode = nullptr;
signals:
	void writeComInfo(const QByteArray);
	void setUIinfo();
};