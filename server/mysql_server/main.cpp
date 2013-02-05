#include <ctime>
#include <iostream>
#include <string>
#include "mysql_server.h"
#include "mysql_handler.h"

#include "cmdline.h"
#include <mysql++.h>
#include <iomanip>
#include <ssqls.h>
#include "logSave_system.h"
#include <core.h>
#include <json/json.h>
#include <file_system.h>
//using namespace std;

void sig_usr1(int sig_num)
{
	exit(0);
}

int main()
{
	int i;
	pid_t pc;

	signal(SIGUSR1, sig_usr1);


	pc = fork(); //第一步
	if(pc<0){
		printf("error fork\n");
		exit(1);
	}
	else if(pc>0)
		exit(0);
	setsid(); //第二步
//	chdir("/"); //第三步
	umask(0); //第四步
	for(i=0;i<10;i++) //第五步
		close(i);

	
	try
	{
		logger.readConfig("./instance/mysql_cfg.json");
		logSave_sys;
		net_core;
		Json::Value port = na::file_system::load_jsonfile_val("./instance/server.json");
		mysql_svr.init(port["logServerPort"].asUInt());
		mysql_svr.run();
		while(1)
		{
			sleep(100000);
/*			
			char c = getchar();
			if(c=='q')
				break;
*/				
		}
		net_core.stop();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}

	return 0;
}

