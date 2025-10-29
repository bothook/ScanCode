#include "CameraDlg.h"
#include "ScanCode.h"
#include "QWindow"
#include "QVBoxLayout"
#include <QCloseEvent>


CameraDlg::CameraDlg(QVector<HWND> windows, QWidget *parent /*= Q_NULLPTR*/)
	: QDialog(parent)
{
	setWindowTitle("相机");
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	QVBoxLayout* layout = new QVBoxLayout(this);
	QHBoxLayout* Hlayout = new QHBoxLayout();
	for (auto it : windows)
	{
		if (!IsWindow((HWND)it)) 
			continue;
		QWindow* window = QWindow::fromWinId((WId)it);
		if (window && window->handle())
		{
			QWidget* widget = QWidget::createWindowContainer(window, this);
			widget->setFixedSize(window->width(), window->height());
			Hlayout->addWidget(widget);
		}
	}
	Hlayout->addStretch();
	layout->addLayout(Hlayout);
	setLayout(layout);
}

CameraDlg::~CameraDlg()
{
}

void CameraDlg::closeEvent(QCloseEvent *e)
{
	hide();
	e->ignore(); // 阻止默认关闭行为
}
