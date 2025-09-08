#pragma once
#include <QtWidgets/QMainWindow>
#pragma execution_character_set("utf-8")
#include "windows.h"
#include "memory"
#include "DHCamera.h"
using namespace std;

class ShkImageWindow;
class LogDlg;
extern void executeSQL(string sql, vector<pair<string, string>>& logData = *make_shared<vector<pair<string, string>>>());
class ScanCode : public QMainWindow
{
    Q_OBJECT
public:
	ScanCode(QWidget *parent = Q_NULLPTR);
	//virtual ~ScanCode();
private:
	void showLogDlg();
	void saveLog(QString logContent);
	void initSql();
	//bool initCamera();
private:
	ShkImageWindow* m_shkWindow = nullptr;
	LogDlg* m_logDlg = nullptr;
	//DHANDLE m_cameraHandle = nullptr;
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