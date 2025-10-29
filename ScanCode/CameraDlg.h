#pragma once
#include <QDialog>


class QHBoxLayout;
class CameraDlg :public QDialog
{
	Q_OBJECT
public:
	CameraDlg(QVector<HWND> windows, QWidget *parent = Q_NULLPTR);
	~CameraDlg();
protected:
	void closeEvent(QCloseEvent *e)override;
};

