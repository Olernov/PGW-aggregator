#include <iostream>
#include <string>
#include "LogWriter.h"


void LogWriter::SetLogStream(time_t messageTime)
{
    if (logToStdout) {
        //m_logStream = std::cout;
        return;
    }
    tm messageTimeTm;
    localtime_r(&messageTime, &messageTimeTm);
	char dateBuf[30];
    snprintf(dateBuf, 30, "%4.4d%2.2d%2.2d", messageTimeTm.tm_year+1900, messageTimeTm.tm_mon+1, messageTimeTm.tm_mday);
	if(std::string(dateBuf) != m_logFileDate) {
		if(m_logStream.is_open()) {
			m_logStream.close();
		}
        char logName[maxPath];
        if(!m_logPath.empty()) {
            snprintf(logName, maxPath, "%s/pgw_%s.log", m_logPath.c_str(), dateBuf);
        }
        else {
            snprintf(logName, maxPath, "pgw_%s.log", dateBuf);
        }
		m_logStream.open(logName, std::fstream::app | std::fstream::out);
        if (!m_logStream.is_open()) {
			throw std::runtime_error(std::string("Unable to open log file ") + std::string(logName));
        }
		m_logFileDate = dateBuf;
	}
}


void LogWriter::WriteThreadFunction()
{
	while (true) {
		try {
			while (!messageQueue.empty()) {
				LogMessage* pMessage;
				if (messageQueue.pop(pMessage)) {
					tm messageTime;
                    localtime_r(&pMessage->m_time, &messageTime);
					char timeBuf[30];
					strftime(timeBuf, 29, "%H:%M:%S", &messageTime);
                    SetLogStream(pMessage->m_time);
					m_logStream << timeBuf << "  |  "
                        << (pMessage->m_threadNum != mainThreadIndex ? std::to_string(pMessage->m_threadNum) : " ")
						<< "  |  " << pMessage->m_message.c_str() << std::endl;
					delete pMessage;
				}
			}
			if (m_stopFlag && messageQueue.empty())
				return;
            if (messageQueue.empty()) {
                std::this_thread::sleep_for(std::chrono::seconds(secondsToSleepWhenNothingToDo));
            }
		}
		catch (...) {
			std::lock_guard<std::mutex> lock(m_exceptionMutex);
			if (m_excPointer == nullptr)
				// if exception is not set or previous exception is cleared then set new
				m_excPointer = std::current_exception();
		}
	}
}

LogWriter::LogWriter() 
	: messageQueue(queueSize), 
    logToStdout(true),
	m_stopFlag(false),
	m_excPointer(nullptr)
{
}


bool LogWriter::Initialize(const std::string& logPath, LogLevel level)
{
    this->logToStdout = logPath.empty();
    m_logPath = logPath;
    logLevel = level;
	time_t now;
	time(&now);
    SetLogStream(now);
	m_writeThread = std::thread(&LogWriter::WriteThreadFunction, this);
	return true;
}

bool LogWriter::Write(const LogMessage& message)
{
	try {
		LogMessage* pnewMessage = new LogMessage(message);
		if (!messageQueue.push(pnewMessage))
			throw LogWriterException("Unable to add message to log queue");
	}
	catch(...){
		std::cerr << "exception when LogWriter::Write" << std::endl;
	}
	return true;
}

bool LogWriter::Write(std::string message, short threadIndex, LogLevel msgLevel)
{
    if (msgLevel >= logLevel) {
        time_t now;
        time(&now);
        LogMessage* pnewMessage = new LogMessage(now, threadIndex, message);
        Write(*pnewMessage);
    }
	return true;
}

void LogWriter::operator<<(const std::string& message)
{
    Write(message, mainThreadIndex);
}

bool LogWriter::LogOtlException(const std::string& header, const otl_exception& otlEx, short threadIndex)
{
    std::string message = reinterpret_cast<const char*>(otlEx.msg);
    Write(header, threadIndex, error);
    Write(message, threadIndex, error);
    Write(reinterpret_cast<const char*>(otlEx.stm_text), threadIndex, error);
    Write(reinterpret_cast<const char*>(otlEx.var_info), threadIndex, error);
}


void LogWriter::ClearException()
{
	std::lock_guard<std::mutex> lock(m_exceptionMutex);
	m_excPointer = nullptr;
}

LogWriter::~LogWriter()
{
    m_stopFlag = true;
    if (m_writeThread.joinable())
        m_writeThread.join();
    m_logStream.close();
}
