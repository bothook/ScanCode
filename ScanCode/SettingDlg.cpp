#include "SettingDlg.h"
#include "ScanCode.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QMessageBox>
#include <QShowEvent>


SettingDlg::SettingDlg(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle("设置");
	setFixedSize(400, 250);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(110, 30, 110, 10);
	
	QHBoxLayout* machineLayout = new QHBoxLayout();
	QLabel* machineTitle = new QLabel("机台号：", this);
	m_machine = new QLineEdit(this);
	m_machine->setFixedWidth(75);
	machineLayout->addWidget(machineTitle);
	machineLayout->addWidget(m_machine);
	machineLayout->addStretch();

	QHBoxLayout* portLayout = new QHBoxLayout();
	QLabel* portTitle = new QLabel("扫码串口号：", this);
	m_port = new QLineEdit(this);
	m_port->setFixedWidth(25);
	m_port->setMaxLength(1);
	portLayout->addWidget(portTitle);
	portLayout->addWidget(m_port);
	portLayout->addStretch();

	QHBoxLayout* urlLayout = new QHBoxLayout();
	QLabel* urlTitle = new QLabel("URL：", this);
	m_url = new QLineEdit(this);
	urlLayout->addWidget(urlTitle);
	urlLayout->addWidget(m_url);
	urlLayout->addStretch();

	QHBoxLayout* delayLayout = new QHBoxLayout();
	QLabel* delayTitle = new QLabel("扫描时长(ms)：", this);
	m_delayTime = new QLineEdit(this);
	m_delayTime->setFixedWidth(50);
	delayLayout->addWidget(delayTitle);
	delayLayout->addWidget(m_delayTime);
	delayLayout->addStretch();

	QHBoxLayout* changLayuot = new QHBoxLayout();
	QPushButton* changeBtn = new QPushButton("修改", this);
	changeBtn->setFixedWidth(50);
	connect(changeBtn, &QPushButton::clicked, this, &SettingDlg::saveConfig);
	changLayuot->addWidget(changeBtn);
	
	layout->addLayout(machineLayout);
	layout->addSpacing(10);
	layout->addLayout(portLayout);
	layout->addSpacing(10);
	layout->addLayout(urlLayout);
	layout->addSpacing(10);
	layout->addLayout(delayLayout);
	layout->addSpacing(15);
	layout->addLayout(changLayuot);
	setLayout(layout);
	layout->addStretch();
}

SettingDlg::~SettingDlg()
{
}

void initConfig()
{
	QString filename = "config.ini";
	QFile file(filename);
	if (!file.exists())
	{
		QSettings settings(filename, QSettings::IniFormat);
		settings.beginGroup("Default");
		settings.setValue("Machine", "");
		settings.setValue("ComPort", 0);
		settings.setValue("URL", "");
		settings.setValue("DelayTime", 0);
		settings.endGroup();
	}
	else
	{
		QSettings settings(filename, QSettings::IniFormat);
		m_setting.machine = settings.value("Default/Machine").toString();
		m_setting.com = settings.value("Default/ComPort").toInt();
		m_setting.url = settings.value("Default/Url").toString();
		m_setting.delay = settings.value("Default/DelayTime").toInt();
	}
}

void SettingDlg::saveConfig()
{
	QSettings settings("config.ini", QSettings::IniFormat);
	settings.beginGroup("Default");
	settings.setValue("Machine", m_machine->text());
	settings.setValue("ComPort", m_port->text());
	settings.setValue("URL", m_url->text());
	settings.setValue("DelayTime", m_delayTime->text());
	settings.endGroup();
	saveLog(QString("修改机台号为：%1,COM为：%2,URL为：%3,DelayTime为：%4")
		.arg(m_machine->text())
		.arg(m_port->text())
		.arg(m_url->text())
		.arg(m_delayTime->text()));
	initConfig();
	QMessageBox::information(this, "提示", "修改完成");
	this->hide();
}

void SettingDlg::showEvent(QShowEvent * e)
{
	m_machine->setText(m_setting.machine);
	m_port->setText(QString::number(m_setting.com));
	m_url->setText(m_setting.url);
	m_delayTime->setText(QString::number(m_setting.delay));
}
