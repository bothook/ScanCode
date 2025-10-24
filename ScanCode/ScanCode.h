#pragma once
#include <QtWidgets/QMainWindow>
#pragma execution_character_set("utf-8")
#include "windows.h"
#include <memory>
using namespace std;

class LogDlg;
class SettingDlg;
class ShkImageWindow;
class CameraWindow;
class ComPortWindow;
extern void executeSQL(string sql, vector<pair<string, string>>& logData = *make_shared<vector<pair<string, string>>>());
extern void saveLog(const QString logContent);
class ScanCode : public QMainWindow
{
    Q_OBJECT
public:
	ScanCode(QWidget *parent = Q_NULLPTR);
	~ScanCode();
private:
	void showLogDlg();
	void showSettingDlg();
	void initSql();
	void initConfig();
private:
	LogDlg* m_logDlg = nullptr;
	SettingDlg* m_settingDlg = nullptr;
	ShkImageWindow* m_shkWindow = nullptr;
	CameraWindow* m_cameraWindow = nullptr;
	ComPortWindow* m_comPortWindow = nullptr;
};

class ShkImageWindow :public QWidget
{
	Q_OBJECT
public:
	ShkImageWindow(QWidget *parent = Q_NULLPTR);
	~ShkImageWindow();
	void getCodeString(QString& str);
private:
};
typedef HANDLE DHANDLE;
class CameraWindow : public QWidget
{
	Q_OBJECT
public:
	CameraWindow(QWidget *parent = Q_NULLPTR);
	~CameraWindow();
	bool photoGraph();
private:
	DHANDLE m_cameraHandle = nullptr;
};

class QSerialPort;
class QLabel;
class ComPortWindow : public QWidget 
{
	Q_OBJECT
	//friend class ScanCode;
public:
	ComPortWindow(QWidget *parent = Q_NULLPTR);
	~ComPortWindow();
private:
	void readComPort();
	void writeComPort(const QByteArray data);
	void resolveCode();
private:
	QSerialPort* m_serial = nullptr;
	QLabel* m_codeContent = nullptr;
	QString m_codeFromCom;
};