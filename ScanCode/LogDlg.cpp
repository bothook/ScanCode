#include "LogDlg.h"
#include "ScanCode.h"
#include <QStandardItemModel>
#include <QTableView>
#include <QDateEdit>
#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDateTime>
#include <QPushButton>

LogDlg::LogDlg(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle("日志");
	resize(400,250);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	QVBoxLayout* layout = new QVBoxLayout(this);
	m_dateEdit = new QDateEdit(this);
	m_dateEdit->setMinimumDate(QDate::currentDate().addDays(-10));
	m_dateEdit->setMaximumDate(QDate::currentDate());
	connect(m_dateEdit, &QDateEdit::dateChanged, [=](const QDate &date) {
		showChooseData(date.toString("yyyy/MM/dd"));
	});
	m_tableView = new QTableView(this);
	m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_model = new QStandardItemModel(this);
	m_tableView->setModel(m_model);
	layout->addWidget(m_dateEdit);
	layout->addWidget(m_tableView);
	QDate today = QDate::currentDate();
	m_dateEdit->setDate(today);
}

void LogDlg::getLogData(QString date)
{
	m_logDataVec.clear();
	if (date.isEmpty())
		return;
	QString sql = QString("SELECT * FROM LOG WHERE TIME LIKE '%1%'").arg(date);
	vector<pair<string, string>> logData;
	executeSQL(sql.toStdString(), logData);
	for (int i = 0; i < logData.size(); ++i) 
		m_logDataVec.append(qMakePair(QString::fromStdString(logData[i].first)
			, QString::fromStdString(logData[i].second)));
}

void LogDlg::showChooseData(QString date)
{
	getLogData(date);
	m_model->clear();
	m_model->setColumnCount(2);
	m_model->setRowCount(m_logDataVec.size());
	// 设置表头
	m_model->setHeaderData(0, Qt::Horizontal, tr("日期"));
	m_model->setHeaderData(1, Qt::Horizontal, tr("内容"));
	// 填充数据
	for (int row = 0; row < m_logDataVec.size(); ++row)
	{
		m_model->setItem(row, 0, new QStandardItem(m_logDataVec[row].first));
		m_model->setItem(row, 1, new QStandardItem(m_logDataVec[row].second));
	}
}

void LogDlg::showEvent(QShowEvent *e)
{
	showChooseData((m_dateEdit->date().toString("yyyy/MM/dd")));
}
