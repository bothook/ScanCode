#pragma once
#include <QDialog>


class QLineEdit;
class SettingDlg : public QDialog
{
	Q_OBJECT

public:
	SettingDlg(QWidget *parent = Q_NULLPTR);
	~SettingDlg();
private:
	void saveConfig();
	void getConfig(QString& com, QString& url, QString& machine);
private:
	QLineEdit* m_port = nullptr;
	QLineEdit* m_url = nullptr;
	QLineEdit* m_machine = nullptr;
};
