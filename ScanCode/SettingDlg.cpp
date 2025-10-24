#include "SettingDlg.h"
#include "ScanCode.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QMessageBox>

SettingDlg::SettingDlg(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle("设置");
	resize(400, 250);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(100, 10, 100, 10);
	QString com, url,machine;
	getConfig(com, url, machine);
	QHBoxLayout* machineLayout = new QHBoxLayout(this);
	QLabel* machineTitle = new QLabel("机台号：", this);
	m_machine = new QLineEdit(machine, this);
	m_machine->setFixedWidth(75);
	machineLayout->addWidget(machineTitle);
	machineLayout->addWidget(m_machine);
	machineLayout->addStretch();
	QHBoxLayout* portLayout = new QHBoxLayout(this);
	QLabel* portTitle = new QLabel("串口号：", this);
	portTitle->setFixedWidth(50);
	m_port = new QLineEdit(com,this);
	m_port->setFixedWidth(25);
	m_port->setMaxLength(1);
	portLayout->addWidget(portTitle);
	portLayout->addWidget(m_port);
	portLayout->addStretch();
	QHBoxLayout* urlLayout = new QHBoxLayout(this);
	QLabel* urlTitle = new QLabel("URL：", this);
	m_url = new QLineEdit(url,this);
	urlLayout->addWidget(urlTitle);
	urlLayout->addWidget(m_url);
	urlLayout->addStretch();
	layout->addLayout(machineLayout);
	layout->addSpacing(10);
	layout->addLayout(portLayout);
	layout->addSpacing(10);
	layout->addLayout(urlLayout);
	setLayout(layout);
	layout->addStretch();
	QPushButton* confirmBtn = new QPushButton("修改", this);
	confirmBtn->setFixedWidth(50);
	confirmBtn->move(175, 120);
	connect(confirmBtn, &QPushButton::clicked, this, &SettingDlg::saveConfig);
}

SettingDlg::~SettingDlg()
{
}

void SettingDlg::saveConfig()
{
	QSettings settings("config.ini", QSettings::IniFormat);
	settings.beginGroup("Default");
	settings.setValue("Machine", m_machine->text());
	settings.setValue("ComPort", "COM" + m_port->text());
	settings.setValue("URL", m_url->text());
	settings.endGroup();
	saveLog(QString("修改机台号为%1,COM为%2,URL为%3")
		.arg(m_machine->text()).arg(m_port->text()).arg(m_url->text()));
}

void SettingDlg::getConfig(QString& com, QString& url, QString& machine)
{
	QSettings settings("config.ini", QSettings::IniFormat);
	machine = settings.value("Default/Machine").toString();
	com = settings.value("Default/ComPort").toString().right(1);
	url = settings.value("Default/Url").toString();
}
