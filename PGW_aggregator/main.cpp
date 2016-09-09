#include <iostream>
#include <cassert>
//#include <boost/lockfree/queue.hpp>

#include "ConfigContainer.h"
#include "GPRSRecord.h"
#include "OTL_Header.h"
#include "Utils.h"
#include "Session.h"
#include "Common.h"
#include "Aggregator.h"
#include "Parser.h"

#define LOG_ERROR 3

using namespace std;



void log(short msgType, string msgText)
{
    cout << msgText << endl;

}



int main(int argc, const char* argv[])
{
	// Run tests
	Utils::RunAllTests();
	otl_connect::otl_initialize();
	otl_connect dbConnect;
	try {
		dbConnect.rlogon("aggregator/aggregator@192.168.100.109:1521/irbistst");
		Aggregator aggregator(dbConnect);
		aggregator.RunAllTests();
		Parser parser(aggregator);
		parser.RunPerFileAggregationTest("../CDR/");

		dbConnect.commit();
		dbConnect.logoff();
	}
	catch(otl_exception& otlEx) {
		dbConnect.rollback();
		// TODO: add correct processing
		cout << "DB error: " << endl
			 << otlEx.msg << endl
			 << otlEx.stm_text << endl
			 << otlEx.var_info << endl;
		exit(-1);
	}
    return 0;
}
