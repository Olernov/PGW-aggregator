#pragma once
#include "OTL_Header.h"

class DBConnect : public otl_connect
{
public:
    void logon(const std::string& connectStr, int autocommit = 0)
    {
        connectString = connectStr;
        autoCommit = autocommit;
        otl_connect::rlogon(connectString.c_str(), autoCommit, nullptr, nullptr);
    }

    void reconnect() {
        try {
            logoff();
            connected = false;
            otl_connect::rlogon(connectString.c_str(), autoCommit);
            connected = true;
        }
        catch(const otl_exception& ex) {
            // don't react on possible exception
        }
    }

private:
    std::string connectString;
    int autoCommit;
    // hide base class's rlogon function to avoid errors
    void rlogon(const char *connect_str, const int aauto_commit = 0,
                const char *xa_server_external_name = nullptr,
                const char *xa_server_internal_name = nullptr
  #if defined(OTL_ORA_OCI_ENV_CREATE)
                ,
                bool threaded_mode = false
  #endif
                ) {};
};
