#pragma once
#pragma execution_character_set("utf-8")
#include <QDialog>
class QTableView;
class QStandardItemModel;
class LogDlg : public QDialog
{
	Q_OBJECT

public:
	LogDlg(QWidget *parent = Q_NULLPTR);
	~LogDlg();

private:
	void getLogData(QString date);
	void showChooseData(QString date);
private:
	QVector<QPair<QString, QString>> m_logDataVec;
	QTableView *m_tableView = nullptr;
	QStandardItemModel* m_model = nullptr;
};
