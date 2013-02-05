#ifndef __XM_GL_PROTOCOL_H__
#define __XM_GL_PROTOCOL_H__
#include "protocol.h"
namespace sg
{
	namespace protocol
	{
		namespace c2l
		{
			enum 
			{
				c2l_begin		= sg::protocol::Gate2LoginBegin,
					register_req,	// json: { "msg":["account","passwd" ]}
					login_req,		// json: { "msg":[user_type,openid,timestamp,sign_str,qd_key,ver,imei]}	
					logout_req,
					changePassword_req,//[oldPW,newPW]

					system_keep_alive, // json []
					charge_gold_req,	// ["account",order_id,gold,status]
					reg_gm_svr_req,    // 
				c2l_end
			};
		}

		namespace l2c
		{
			enum
			{
				l2c_begin		= sg::protocol::Login2GateBegin,
					register_resp,	// json: { "msg":["account",0] }	(0:failed,1:success)  
					login_resp,		// json: { "msg":[0,player_id,"提示字符"] }	(0:failed,1:success,2:already login,3:ban user)  
					logout_resp,
					changePassword_resp,// [(-1:非法操作,0:成功,1:旧密码错误)]
					charge_gold_resp,	// [(-1:非法操作,0:成功,1:失败),OrderId,Gold]
				l2c_end
			};
		}
	}
}
#endif
