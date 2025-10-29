#include "Thread.h"
#include <QDebug>

Thread::Thread(QObject *parent)
	: QThread(parent)
	, pauseFlag(false)
	, stopFlag(false)
{
}

Thread::~Thread()
{
	stop();
	wait();
}

Thread::State Thread::state() const
{
	if (stopFlag) return Stoped;
	if (pauseFlag) return Paused;
	return Running;
}

void Thread::start(Priority pri)
{
	stopFlag = false;
	pauseFlag = false;
	QThread::start(pri);
}

void Thread::stop()
{
	stopFlag = true;
	pauseFlag = false;
	condition.wakeAll();  // 唤醒所有等待的线程
}

void Thread::pause()
{
	if (!pauseFlag && !stopFlag) {
		pauseFlag = true;
	}
}

void Thread::resume()
{
	if (pauseFlag && !stopFlag) {
		pauseFlag = false;
		condition.wakeAll();  // 唤醒线程继续执行
	}
}
void Thread::run()
{
	while (!stopFlag) {
		// 检查暂停状态
		if (pauseFlag) {
			QMutexLocker locker(&mutex);
			condition.wait(&mutex);  // 等待恢复信号
		}

		if (stopFlag) break;

		// 执行具体的业务处理
		try
		{
			process();
		}
		catch (const std::exception& e) 
		{
			stop();
			emit errorStr(QString::fromLocal8Bit(e.what()));
		}
	}
}