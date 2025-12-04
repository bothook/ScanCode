#include "SettingDlg.h"
#include "ScanCode.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QShowEvent>


SettingDlg::SettingDlg(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle("ÉèÖÃ");
	setFixedSize(600, 350);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(110, 30, 110, 10);

	QHBoxLayout* printerLayout = new QHBoxLayout();
	QLabel* printerTitle = new QLabel("µçÄÔÃû£º", this);
	m_printerName = new QLineEdit(this);
	m_printerName->setFixedWidth(75);
	printerLayout->addWidget(printerTitle);
	printerLayout->addWidget(m_printerName);
	printerLayout->addStretch();

	QHBoxLayout* portLayout = new QHBoxLayout();
	QLabel* portTitle = new QLabel("É¨ÂëÇ¹´®¿ÚºÅ£º", this);
	m_port = new QLineEdit(this);
	m_port->setFixedWidth(25);
	portLayout->addWidget(portTitle);
	portLayout->addWidget(m_port);
	portLayout->addStretch();

	QHBoxLayout* urlLayout = new QHBoxLayout();
	QLabel* urlTitle = new QLabel("URL£º", this);
	m_url = new QLineEdit(this);
	m_url->setFixedWidth(300);
	urlLayout->addWidget(urlTitle);
	urlLayout->addWidget(m_url);
	urlLayout->addStretch();

	QHBoxLayout* loginUrlLayout = new QHBoxLayout();
	QLabel* loginUrlTitle = new QLabel("µÇÂ¼URL£º", this);
	m_loginUrl = new QLineEdit(this);
	m_loginUrl->setFixedWidth(300);
	loginUrlLayout->addWidget(loginUrlTitle);
	loginUrlLayout->addWidget(m_loginUrl);
	loginUrlLayout->addStretch();

	QHBoxLayout* loginlayout = new QHBoxLayout();
	QLabel* loginAccountTitle = new QLabel("µÇÂ¼ÕËºÅ£º", this);
	m_loginAccount = new QLineEdit(this);
	m_loginAccount->setFixedWidth(75);
	loginlayout->addWidget(loginAccountTitle);
	loginlayout->addWidget(m_loginAccount);

	QLabel* loginPasswordTitle = new QLabel("µÇÂ¼ÃÜÂë£º", this);
	m_loginPassword = new QLineEdit(this);
	m_loginPassword->setEchoMode(QLineEdit::Password);
	m_loginPassword->setFixedWidth(150);
	loginlayout->addWidget(loginPasswordTitle);
	loginlayout->addWidget(m_loginPassword);
	loginlayout->addStretch();

	QHBoxLayout* delayLayout = new QHBoxLayout();
	QLabel* delayTitle = new QLabel("É¨ÂëÇ¹É¨ÂëÊ±³¤(ms)£º", this);
	m_delayTime = new QLineEdit(this);
	m_delayTime->setFixedWidth(50);
	delayLayout->addWidget(delayTitle);
	delayLayout->addWidget(m_delayTime);
	delayLayout->addStretch();

	QHBoxLayout* changLayuot = new QHBoxLayout();
	QPushButton* changeBtn = new QPushButton("ÐÞ¸Ä", this);
	changeBtn->setFixedWidth(50);
	connect(changeBtn, &QPushButton::clicked, this, &SettingDlg::saveConfig);
	changLayuot->addWidget(changeBtn);

	layout->addLayout(printerLayout);
	layout->addSpacing(10);
	layout->addLayout(portLayout);
	layout->addSpacing(10);
	layout->addLayout(urlLayout);
	layout->addSpacing(10);
	layout->addLayout(loginUrlLayout);
	layout->addSpacing(10);
	layout->addLayout(loginlayout);
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
		settings.setValue("LoginURL", "");
		settings.setValue("LoginAc", "");
		settings.setValue("LoginPas", "");
		settings.setValue("DelayTime", 0);
		settings.setValue("Printer", "");
		settings.endGroup();
	}
	else
	{
		QSettings settings(filename, QSettings::IniFormat);
		m_setting.com = settings.value("Default/ComPort").toInt();
		m_setting.url = settings.value("Default/Url").toString();
		m_setting.loginUrl = settings.value("Default/LoginURL").toString();
		m_setting.loginAc = settings.value("Default/LoginAc").toString();
		m_setting.loginPas = settings.value("Default/LoginPas").toString();
		m_setting.delay = settings.value("Default/DelayTime").toInt();
		m_setting.printer = settings.value("Default/Printer").toString();
	}
}

void SettingDlg::saveConfig()
{
	QSettings settings("config.ini", QSettings::IniFormat);
	settings.beginGroup("Default");
	settings.setValue("ComPort", m_port->text());
	settings.setValue("URL", m_url->text());
	settings.setValue("LoginURL", m_loginUrl->text());
	settings.setValue("LoginAc", m_loginAccount->text());
	settings.setValue("LoginPas", m_loginPassword->text());
	settings.setValue("DelayTime", m_delayTime->text());
	settings.setValue("Printer", m_printerName->text());
	settings.endGroup();
	saveLog(QString("ÐÞ¸Ä´òÓ¡»úÃû³Æ: %1,COMÎª£º%2,URLÎª£º%3,µÇÂ¼URL: %4,µÇÂ¼ÕËºÅ: %5,É¨ÃèÑÓÊ±Îª£º%6")
		.arg(m_printerName->text())
		.arg(m_port->text())
		.arg(m_url->text())
		.arg(m_loginUrl->text())
		.arg(m_loginAccount->text())
		.arg(m_delayTime->text()));
	initConfig();
	QMessageBox::information(this, "ÌáÊ¾", "ÐÞ¸ÄÍê³É");
	hide();
}

void SettingDlg::showEvent(QShowEvent * e)
{
	m_port->setText(QString::number(m_setting.com));
	m_url->setText(m_setting.url);
	m_loginUrl->setText(m_setting.loginUrl);
	m_loginAccount->setText(m_setting.loginAc);
	m_loginPassword->setText(m_setting.loginPas);
	m_delayTime->setText(QString::number(m_setting.delay));
	m_printerName->setText(m_setting.printer);
}
