#pragma once
#include <QDialog>

extern void initConfig();
struct SETTING
{
	int com;
	QString url;
	QString loginUrl;
	QString loginAc;
	QString loginPas;
	QString machine;
	int delay;
	QString printer;
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
	QLineEdit* m_loginUrl = nullptr;
	QLineEdit* m_loginAccount = nullptr;
	QLineEdit* m_loginPassword = nullptr;
	QLineEdit* m_delayTime = nullptr;
	QLineEdit* m_printerName = nullptr;
};
