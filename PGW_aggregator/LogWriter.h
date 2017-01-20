#pragma once
#include <queue>
#include <thread>
#include <time.h>
#include <fstream>
#include <mutex>
#include <exception>
#include <stdexcept>
#include <atomic>
#include <boost/lockfree/queue.hpp>
#include "Common.h"

class LogWriterException 
{
public:
	LogWriterException (const std::string& message) 
		: m_message(message) 
		{}
	std::string m_message;
};

class LogMessage
{
public:
	LogMessage();
	LogMessage(time_t time, short threadNum, std::string message)
		: m_time(time), m_threadNum(threadNum), m_message(message)
		{}
	time_t m_time;
	short m_threadNum;
	std::string m_message;
};

class LogWriter
{
public:
	LogWriter();
	~LogWriter();
    bool Initialize(bool logToStdout, const std::string& logPath);
	bool Write(std::string message, short threadIndex = MAIN_THREAD_NUM);
	bool Write(const LogMessage&);
	void operator<<(const std::string&);
	inline std::exception_ptr GetException() { return m_excPointer; }
	void ClearException();
	bool Stop();
private:
	static const int queueSize = 128;
    static const int sleepWhenQueueEmpty = 3;
	std::exception_ptr m_excPointer;
    bool logToStdout;
	std::string m_logPath;
	boost::lockfree::queue<LogMessage*> messageQueue;
	std::atomic<bool> m_stopFlag;
	std::thread m_writeThread;
	std::mutex m_exceptionMutex;
	std::ofstream m_logStream;
	std::string m_logFileDate;
	void WriteThreadFunction();
    void SetLogStream(time_t messageTime);
};
