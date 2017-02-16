#pragma once
#include "OTL_Header.h"

class DBConnect : public otl_connect
{
public:
//    void logon(const std::string& connectStr, int autocommit = 0)
//    {
//        connectString = connectStr;
//        autoCommit = autocommit;
//        otl_connect::rlogon(connectString.c_str(), autoCommit, nullptr, nullptr);
//    }

    void rlogon(const char *connect_str, const int aauto_commit = 0,
                const char *xa_server_external_name = nullptr,
                const char *xa_server_internal_name = nullptr
  #if defined(OTL_ORA_OCI_ENV_CREATE)
                ,
                bool threaded_mode = false
  #endif
                )
    {
        connectString = connect_str;
        autoCommit = aauto_commit;
        if (xa_server_external_name != nullptr) {
            xaExternalName = xa_server_external_name;
        }
        else {
            xaExternalName.empty();
        }
        if (xa_server_internal_name != nullptr) {
            xaInternalName = xa_server_internal_name;
        }
        else {
            xaInternalName.empty();
        }
#if defined(OTL_ORA_OCI_ENV_CREATE)

        threadedMode = threaded_mode;
#endif

        otl_connect::rlogon(connect_str, aauto_commit, xa_server_external_name, xa_server_internal_name
#if defined(OTL_ORA_OCI_ENV_CREATE)
              ,
              threaded_mode
#endif
        );


    }

    void reconnect() {
        try {
            logoff();
            connected = false;
            otl_connect::rlogon(connectString.c_str(), autoCommit, xaExternalName.c_str(), xaInternalName.c_str()
#if defined(OTL_ORA_OCI_ENV_CREATE)
              ,
              threadedMode
#endif
            );
            connected = true;
        }
        catch(const otl_exception& ex) {
            // don't react on possible exception
        }
    }

private:
    std::string connectString;
    int autoCommit;
    std::string xaExternalName;
    std::string xaInternalName;
    bool threadedMode;

};
