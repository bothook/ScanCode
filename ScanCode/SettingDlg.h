#pragma once
#include <QDialog>

extern void initConfig();
struct SETTING
{
	SETTING() :com(0), url(""), machine(""), delay(0) {};
	int com;
	QString url;
	QString machine;
	int delay;
};
extern SETTING m_setting;
class QShowEvent;
class QLineEdit;
class SettingDlg : public QDialog
{
	Q_OBJECT

public:
	SettingDlg(QWidget *parent = Q_NULLPTR);
	~SettingDlg();
private:
	void saveConfig();
	void showEvent(QShowEvent * e)override;
private:
	QLineEdit* m_machine = nullptr;
	QLineEdit* m_port = nullptr;
	QLineEdit* m_url = nullptr;
	QLineEdit* m_delayTime = nullptr;
};
