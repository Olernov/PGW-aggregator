#include <stdio.h>
//#include <time.h>
//#include <math.h>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
//#include <cmath>
//#include <map>

using namespace std;


struct FtpSetting
{
	string ftpServer;
	string ftpUsername;
	string ftpPassword;
	string ftpPort;
	string ftpDirectory;
};

struct Config
{
public:
	Config() {};
	Config(ifstream& cfgStream);

	void ReadConfigFile(ifstream& cfgStream);
	string m_connectString;
	string m_outputDirectory;
    unsigned short numOfParsers;
    unsigned short numOfAggregators;
    FtpSetting m_ftpSettings;
};
