#include <ctime>
#include <iostream>
#include <string>
#include "commom.h"

#include <boost/asio.hpp>

#include "game_server.h"
#include "game_handler.h"

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
			cfg += "game_cfg.json";
		}
		logger.readConfig(cfg);
		LogI << "loading config:\t" << cfg << LogEnd;
		//LogT<<  argv[0] << LogEnd;
		game_svr->init(cfg);
		game_svr->run();
		while(1)
		{
			sleep(100000);
/*			
			std::string input_str;
			getline(std::cin,input_str);
			if(input_str == "q")
				break;
			if(!game_svr->deal_cmd_str(input_str))
				LogW << color_red(input_str) << " is not a correct commands." << LogEnd;
*/				
		}
		game_svr->stop();
	}
	catch (std::exception& e)
	{
		LogE <<  e.what() << LogEnd;
	}
	
	return 0;
}

