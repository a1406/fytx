#include <ctime>
#include <iostream>
#include <string>

#include <boost/asio.hpp>

#include "login_server.h"
#include "login_handler.h"

void sig_usr1(int sig_num)
{
	exit(0);
}

int main(int argc, char *argv[])
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
		std::string cfg = "./instance/";
		if(argc>1)
		{
			cfg += argv[1];
			
		}
		else
		{
			cfg += "login_config.json";
		}
//		LogI << "loading config:\t" << cfg << LogEnd;
		logger.readConfig(cfg);
		//LogT<<  argv[0] << LogEnd;
		login_svr->init(cfg);
		login_svr->run();
		while(1)
		{
			sleep(100000);
/*			
			std::string input_str;
			getline(std::cin,input_str);
			if(input_str == "q")
				break;
*/
				
//			if(!login_svr->deal_cmd_str(input_str))
				//LogW << color_red(input_str) << " is not a correct commands." << LogEnd;
		}
		login_svr->stop();
	}
	catch (std::exception& e)
	{
		e.what();
//		LogE <<  e.what() << LogEnd;
	}
}
