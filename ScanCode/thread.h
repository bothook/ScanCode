#pragma once
#include <QThread>
#include <atomic>
#include <QMutex>
#include <QWaitCondition>

class Thread : public QThread
{
	Q_OBJECT
public:
	Thread(QObject *parent = nullptr);
	virtual~Thread();

	enum State {
		Stoped,  ///< 停止状态
		Running, ///< 运行状态  
		Paused   ///< 暂停状态
	};

	State state() const;

	public slots:
	void start(Priority pri = InheritPriority);
	void stop();
	void pause();
	void resume();

protected:
	virtual void run() override final;
	virtual void process() = 0;  // 纯虚函数，子类实现具体业务

private:
	std::atomic_bool pauseFlag;
	std::atomic_bool stopFlag;
	QMutex mutex;
	QWaitCondition condition;
signals:
	void errorStr(QString);
};