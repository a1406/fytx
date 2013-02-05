#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include "gate_server.h"
#include <core.h>
#include <Glog.h>
using namespace boost::detail::thread;
//void tmain()
//{
//	try
//	{
//		gate_svr.init();
//		gate_svr.run();
//	}
//	catch (std::exception& e)
//	{
//		std::cerr << e.what() << LogEnd;
//	}
//}

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

	
	logger.readConfig("./instance/gate_cfg.json");
	logger.printConfig();
	net_core;
	gate_svr.init();
	gate_svr.run();
	while(1)
	{
		sleep(100000);
/*		
		std::string input_str;
		getline(std::cin,input_str);
		if(input_str=="q")
			break;
		else if (input_str=="login")
		{
			gate_svr.disconnect_account_svr();
		}
		else if (input_str=="fee")
		{
			gate_svr.disconnect_fee_svr();
		}
		else
		{
			LogW << color_red(input_str) << " is not a correct commands." << LogEnd;
		}
*/		
	}
	gate_svr.stop();

	net_core.stop();
	//LogT<<  "exit gate server" << LogEnd;
	return 0;
}
