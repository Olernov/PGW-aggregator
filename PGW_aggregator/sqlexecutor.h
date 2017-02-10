#pragma once
#include "OTL_Header.h"
#include "Common.h"
#include "DBConnect.h"

class SQLExecutor
{
public:

    SQLExecutor(DBConnect& conn, const std::string& stmt) :
        dbConnect(const_cast<DBConnect&>(conn)),
        sqlStatement(stmt)
    {
        dbStream.open(1, stmt.c_str(), conn);
    }

    ~SQLExecutor()
    {
        dbStream.close();
    }


    template<typename T> void PassInArgs(T& arg)
    {
        dbStream << arg;
    }

    template<typename First, typename... Rest> void PassInArgs(First& first, Rest&... rest)
    {
        dbStream << first;
        PassInArgs(rest...);
    }

    template<typename T>
        void GetOutArgs(T& arg)
    {
std::cout << "get out args single" << std::endl;
        dbStream >> arg;
    }

    template<typename First, typename... Rest>
        void GetOutArgs(First& first, Rest&... rest)
    {
std::cout << "get out args multiple" << std::endl;
        dbStream >> first;
        GetOutArgs(rest...);
    }

private:
    DBConnect& dbConnect;
    otl_stream dbStream;
    const std::string& sqlStatement;

};


class ReconSQLExecutor
{
public:
    ReconSQLExecutor(DBConnect& conn, const std::string& stmt) :
        //sqlExecutor(conn, stmt),
        dbConnect(conn),
        statement(stmt)
    {}

    template<typename... Args> void PassInArgs(Args&... args)
    {
        int attemptCount = 0;
        SQLExecutor sqlExecutor(dbConnect, statement);
        while (attemptCount++ < maxAttemptsToAccessDB) {
            try {
                sqlExecutor.PassInArgs(args...);
                break;
            }
            catch(const otl_exception& ex) {
                if (attemptCount >= maxAttemptsToAccessDB) {
                    throw;
                }
                dbConnect.reconnect();
            }
        }
    }

    template<typename... Args> void GetOutArgs(Args&... args)
    {
        SQLExecutor sqlExecutor(dbConnect, statement);
        int attemptCount = 0;
        while (attemptCount++ < maxAttemptsToAccessDB) {
std::cout << "get out args attemptCount: " << attemptCount << std::endl;
            try {
                sqlExecutor.GetOutArgs(args...);
                break;
            }
            catch(const otl_exception& ex) {
std::cout << "otl exception: " << std::endl;
                if (attemptCount >= maxAttemptsToAccessDB) {
std::cout << "throw otl exception: " << std::endl;
                    throw;
                }
std::cout << "before reconnect" << std::endl;
                dbConnect.reconnect();
std::cout << "after reconnect" << std::endl;
            }
        }
    }


//    template<typename First, typename... Rest> void PassInArgs(First& first, Rest&... rest)
//    {
//        sqlExecutor.PassInArgs(first, rest...);
//    }

private:
    //SQLExecutor sqlExecutor;
    DBConnect& dbConnect;
    std::string statement;


    const int maxAttemptsToAccessDB = 3;
};
