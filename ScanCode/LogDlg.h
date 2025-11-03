#pragma once
#include <QDialog>
class QTableView;
class QStandardItemModel;
class QDateEdit;
class QLineEdit;
class LogDlg : public QDialog
{
	Q_OBJECT

public:
	LogDlg(QWidget *parent = Q_NULLPTR);
	~LogDlg();
private:
	void getLogData(QString date);
	void showChooseData(int num);
	void showEvent(QShowEvent * e)override;
private:
	QVector<QPair<QString, QString>> m_logDataVec;
	QTableView *m_tableView = nullptr;
	QStandardItemModel* m_model = nullptr;
	QDateEdit *m_dateEdit = nullptr;
	QPushButton* m_nextBtn = nullptr;
	QPushButton* m_lastBtn = nullptr;
	int m_index = 0;
};
