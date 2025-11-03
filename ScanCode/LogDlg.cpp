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
#include <QCalendarWidget>
#include <Qdebug>

#define COUNT 50

LogDlg::LogDlg(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle("日志");
	resize(500,400);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	QVBoxLayout* layout = new QVBoxLayout(this);
	m_dateEdit = new QDateEdit(this);
	m_dateEdit->setFixedHeight(30);
	m_dateEdit->setCalendarPopup(true);
	m_dateEdit->setMaximumDate(QDate::currentDate());
	m_dateEdit->setDisplayFormat("yyyy/MM/dd");
	connect(m_dateEdit, &QDateEdit::dateChanged, [=](const QDate date) {
		
		getLogData(date.toString("yyyy/MM/dd"));
		showChooseData(0);
	});
	layout->addWidget(m_dateEdit);
	m_lastBtn = new QPushButton("上一页", this);
	m_lastBtn->setFixedSize(50, 30);
	connect(m_lastBtn, &QPushButton::clicked, [=] {showChooseData(-1);});
	m_nextBtn = new QPushButton("下一页",this);
	connect(m_nextBtn, &QPushButton::clicked, [=] {showChooseData(1);});
	m_nextBtn->setFixedSize(50, 30);
	QHBoxLayout* indexLayout = new QHBoxLayout();
	indexLayout->addWidget(m_lastBtn);
	indexLayout->addWidget(m_nextBtn);

	m_tableView = new QTableView(this);
	m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_model = new QStandardItemModel(this);
	m_tableView->setModel(m_model);
	m_tableView->verticalHeader()->hide();
	
	layout->addWidget(m_tableView);
	layout->addLayout(indexLayout);
}

LogDlg::~LogDlg()
{
}

void LogDlg::getLogData(QString date)
{
	QVector<QPair<QString, QString>>().swap(m_logDataVec);
	if (date.isEmpty())
		return;
	QString sql = QString("SELECT * FROM LOG WHERE TIME LIKE '%1%'").arg(date);
	vector<pair<string, string>> logData;
	executeSQL(sql.toStdString(), logData);
	int count = logData.size();
	for (int i = count -1; i >= 0; --i)
		m_logDataVec.append(qMakePair(QString::fromStdString(logData[i].first)
			, QString::fromStdString(logData[i].second)));
	m_index = 0;
}

void LogDlg::showChooseData(int num)
{
	m_index += num;
	if (m_index<0) 
	{
		m_index = 0;
		return;
	}
	int size = m_logDataVec.size();
	if (size - (m_index + 1) * COUNT >= COUNT)
		m_model->setRowCount(COUNT);
	else if (size - (m_index + 1) * COUNT < -COUNT)
	{
		m_index--;
		return;
	}
	else
		m_model->setRowCount(-size - (m_index + 1) * COUNT);
	m_model->clear();
	m_model->setColumnCount(2);
	// 设置表头
	m_model->setHeaderData(0, Qt::Horizontal, tr("日期"));
	m_model->setHeaderData(1, Qt::Horizontal, tr("内容"));
	// 填充数据
	for (int row = 0; row < COUNT; ++row)
	{
		int count = row + m_index * COUNT;
		if (count >= size) 
			return;
		m_model->setItem(row, 0, new QStandardItem(m_logDataVec[count].first));
		m_model->setItem(row, 1, new QStandardItem(m_logDataVec[count].second));
	}
}

void LogDlg::showEvent(QShowEvent *e)
{
	m_dateEdit->setDate(QDate::currentDate());
	getLogData(QDate::currentDate().toString("yyyy/MM/dd"));
	showChooseData(0);
}
