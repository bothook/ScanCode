#include "ScanCode.h"
#include <QtWidgets/QApplication>
#include <QSharedMemory>
#include <DbgHelp.h>
#include <QLocalServer>
#include <QLocalSocket>
#pragma comment(lib,"DbgHelp.lib")

//保存程序异常崩溃的信息
LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{
	//创建 Dump 文件
	HANDLE hDumpFile = CreateFile(L"crash.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDumpFile != INVALID_HANDLE_VALUE)
	{
		//Dump 信息
		MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
		dumpInfo.ExceptionPointers = pException;
		dumpInfo.ThreadId = GetCurrentThreadId();
		dumpInfo.ClientPointers = TRUE;

		// 写入 dump 文件内容
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
	}

	//弹出一个错误对话框
	QMessageBox msgBox;
	msgBox.setText("application crash!");
	msgBox.exec();

	return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	QSharedMemory sharedMemory(QString("%1.exe").arg(QCoreApplication::applicationName()));
	if (!sharedMemory.create(1))
	{
		QLocalSocket socket;
		socket.connectToServer("myService");
		socket.waitForConnected();
		return 0;
	}
	//注册异常捕获函数
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
	ScanCode w;
	QLocalServer server;
	if (server.listen("myService")) {
		QApplication::connect(&server, &QLocalServer::newConnection, [&] {
			w.show();
			});
	}
    w.show();
	return a.exec();
}
