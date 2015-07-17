/*
************************************************************************
*                GDNP API v1.0
* Draft : https://tools.ietf.org/html/draft-carpenter-anima-gdn-protocol-03
* Abstract : 	
* 		This document establishes requirements for a protocol that enables
*		intelligent devices to dynamically discover peer devices, to
*		synchronize state with them, and to negotiate parameter settings
*		mutually with them.  The document then defines a general protocol for
*		discovery, synchronization and negotiation, while the technical
*		objectives for specific scenarios are to be described in separate
*		documents.  An Appendix briefly discusses existing protocols with
*		comparable features.
*
* File:[Client_TCP.cpp]
* Description:The implementation of TCP communication function in class Client.
* Remark:
* Modification record:
* Created by Kangning in Beijing University of Posts and Telecommunications, in May  2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/
#include "Client.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>

/*************************************************************************
*  Function name: Client::negotiate
*  Description: Function for negotiation in class Client using TCP
*  Parameter: 	const void * buffer_obj	//pointer to negotiation objective
*  Return: 		ERRNO
*  Remark:
*  Lastly modified by Kangning Xu on 15-5-25
*************************************************************************/
ERRNO Client::negotiate(const void * buffer_obj)
{
	std::cout << "Negotiation start!!!" << std::endl;
	 struct sockaddr_in6 BroadcastAddr;

	 //check the negoAddr: default broadcast address is not acceptable
	 memset(&BroadcastAddr,0,sizeof(BroadcastAddr));
	 BroadcastAddr.sin6_family = AF_INET6;
	 inet_pton(AF_INET6, "ff02::1", &BroadcastAddr.sin6_addr);
	 BroadcastAddr.sin6_port = htons(4444);

	 if(sockAddrEqual(negoAddr,BroadcastAddr))
	 {
		 std::cout << "wrong negotiation address!" << std::endl;
		 return ERROR;
	 }


	 session_id= generate_random()%(MAX_SESSION_ID+1);

	 loop_count = 5;
	 flag = 0;

	 uint16_t value_len = strlen((char*)buffer_obj) ;
	 Objective_Option obj_opt(Negotiation, value_len, (uint8_t*)buffer_obj,loop_count,flag);
	 uint16_t * bits = obj_opt.to_bits();
	 memcpy(buffer_nego_obj, bits, value_len + Objective_Option::len_except_value);

	 //create asynchronous thread
	 pthread_t id;
	 int ret = pthread_create(&id, NULL, nego_thread, (void *)this);
//	 sleep(5);
	 if(ret) {
	    std::cout << "Create pthread error!" << std::endl;
	    return ERROR;
	 }
	 else
		 std::cout << "Create negotiation pthread !" << std::endl;

	 return SUCCESS;
}

/*************************************************************************
*  Function name: Client::nego_thread
*  Description: Thread function for asynchronous negotiation in class Client
*  Parameter: 	void* _client	//pointer to current Client class
*  Return: 		void
*  Remark:
*  Lastly modified by Kangning Xu on 15-5-25
*************************************************************************/
void *Client::nego_thread(void* _client)
{
	 Client *c = (Client *)_client;

//	 Objective_Option recved_opt = Objective_Option::parse_bits((uint16_t *)c->buffer_nego_obj);
//		 			std::cout <<"after strcpy   type  "<< recved_opt.get_type() <<std::endl
//		 						 <<"len  "<< recved_opt.get_len() << std::endl
//								 <<"loop count" << (int)recved_opt.get_loop_count() << std::endl
//		 						 <<"value  "<< (char*)recved_opt.get_value() << std::endl << std::endl;
	 //nego, an iterative process
	 c->send_tcp(c->buffer_nego_obj,MAXSTRINGLENGTH,REQUEST_MSG);


	 //strcpy((char *)c->buffer_nego_obj,buffer);
	 c->cur_states = OFF;

	 return NULL;
}

/*************************************************************************
*  Function name: Client::send_tcp
*  Description: Sending function in TCP negotiation in class Client
*  Parameter: 	const void* buffer	//pointer to option
*  				size_t buffer_size	//size of option
*  				enum MSG_TYPE type	//type of message
*  Return: 		ERRNO
*  Remark:
*  Lastly modified by Kangning Xu on 15-5-25
*************************************************************************/
ERRNO Client::send_tcp(const void* buffer,size_t buffer_size,enum MSG_TYPE type)
{
	 ERRNO rtnval;
	 switch (cur_states) {
	 	 case OFF:
//	 		 std::cout<<"ready to send in OFF:"<<std::endl;

	       rtnval = send_negotiate(buffer, buffer_size,type);
	       if(rtnval == SUCCESS)
	       	 {
	    	   // store the request data
	         store_last_options(buffer, buffer_size);
	         }
	       break;
	     default:
	    	 std::cout<<"CLIENT_UNEXCEPTED_STATE_ERR  " << cur_states <<std::endl;
	    	 rtnval = CLIENT_UNEXCEPTED_STATE_ERR;
	    	 break;
	     }
	 return rtnval;
}

/*************************************************************************
*  Function name: Client::send_negotiate
*  Description: Initialize tcp socket, send a REQUEST_MSG and start the negotiation
*  Parameter: 	const void* buffer	//pointer to option
*  				size_t buffer_size	//size of option
*  				enum MSG_TYPE type	//type of message
*  Return: 		ERRNO
*  Remark:
*  Lastly modified by Kangning Xu on 15-5-25
*************************************************************************/
ERRNO Client::send_negotiate(const void* buffer,size_t buffer_size,enum MSG_TYPE type)
{
	ERRNO rtnval;
	if(type != REQUEST_MSG)
	{
		dieWithUserMessager("wrong function call!");
	   return CLIENT_SEND_UNEXCEPTED_MSG_TYPE_ERR;
	}

	//init tcp
	int tcp_sock = socket(AF_INET6, SOCK_STREAM, 0);
	char serverIP[40];
	inet_ntop(AF_INET6,(void *)&negoAddr.sin6_addr,serverIP,40);

	rtnval = client_tcp_init(tcp_sock,serverIP);
	if(rtnval != SUCCESS)
	{
		while(try_times < MAX_TRY_TIMES)
		{
			rtnval = client_tcp_init(tcp_sock,serverIP);
			if(rtnval != SUCCESS)
				increase_try_times();
			else
			{
				reset_try_times();
				break;
			}
		}
		if(try_times != 0)
			{
				std::cout<<"server no answer"<<std::endl;
				return ERROR;
			}
	}

	struct timeval Timeout = {10, 0};
	//send timeout
	setsockopt(tcp_sock,SOL_SOCKET,SO_SNDTIMEO,(const void *)&Timeout,(socklen_t)sizeof(struct timeval));
	//recv timeout
	setsockopt(tcp_sock,SOL_SOCKET,SO_RCVTIMEO,(const void *)&Timeout,(socklen_t)sizeof(struct timeval));

	if(loop_count == 0) return ERROR;
	//send pdu
	rtnval = write_pdu(buffer,buffer_size,REQUEST_MSG,session_id);
	loop_count--;

	if(rtnval != SUCCESS)
		return rtnval;

	//recv pdu
	uint32_t actual_session_id;
	rtnval = read_pdu((char*)buffer,buffer_size,type,actual_session_id);
	while(actual_session_id != session_id)
	{
		//dieWithUserMessager("session_id not match");
		//return CLIENT_RECV_NOMATCHED_SESSION_ID_ERR;
		rtnval = read_pdu((char*)buffer,buffer_size,type,actual_session_id);
	}

	//distribute
	rtnval = do_negotiate(buffer,buffer_size,type);

	close(tcp_sock);
	return rtnval;
}

/*************************************************************************
*  Function name: Client::do_negotiate
*  Description: Processing received negotiation message: keep negotiating or wait or end
*  Parameter: 	const void* buffer	//pointer to option
*  				size_t buffer_size	//size of option
*  				enum MSG_TYPE type	//type of message
*  Return: 		ERRNO
*  Remark:
*  Lastly modified by Kangning Xu on 15-5-25
*************************************************************************/
ERRNO Client::do_negotiate(const void* buffer,size_t buffer_size,enum MSG_TYPE type)
{
	void *asa_answer = NULL;
	ERRNO rtnval;
	struct timeval Timeout = {10, 0};

	uint32_t actual_session_id;

	//parse recived option as format Option
	Option recved_opt = Option::parse_bits((uint16_t *)buffer);
	option_type opt_type = recved_opt.get_type();
//	uint16_t opt_len = recved_opt.get_len();
	uint8_t * opt_vlaue = recved_opt.get_value();

	//parse recived option as format Objective_Option
	Objective_Option recved_obj_opt = Objective_Option::parse_bits((uint16_t *)buffer);
//	option_type obj_opt_type = recved_obj_opt.get_type();
//	uint16_t obj_opt_len = recved_obj_opt.get_len();
	uint8_t * obj_opt_vlaue = recved_obj_opt.get_value();
	loop_count = (int)recved_obj_opt.get_loop_count();
	flag = (int)recved_obj_opt.get_flag();

	loop_count--;

	uint16_t value_len = 0;

	Option accept_opt(Accept, 0, (uint8_t*)asa_answer);

	Option decline_opt(Decline, 0, (uint8_t*)asa_answer);

	uint16_t * nego_bits;
	char send_buffer[MAXSTRINGLENGTH];

	//distinguish received msg type
	switch(type)
	{
		case NEGO_MSG:
			cur_states = NEGOING;
				//give msg to upper to gain an answer, judge the answer, take it(send NEGO_END_MSG) or more write and read and do_negotiate

				asa_answer = asa_negotiate_result((void *)obj_opt_vlaue);
				if(asa_geq_fn(asa_answer,(void *)obj_opt_vlaue))
				{
					//upper take the answer, send NEGO_END_MSG with Accept
					rtnval = write_pdu((const void *)accept_opt.to_bits(), accept_opt.get_len() + Option::len_except_value ,NEGO_END_MSG, session_id);
					std::cout << "The negotiation is accepted" << std::endl;
					do_configuration(obj_opt_vlaue);
					return rtnval;
				}
				//need more negotiation
				if(loop_count == 0)
				{
					//send NEGO_END_MSG with Accept
					rtnval = write_pdu((const void *)decline_opt.to_bits(), decline_opt.get_len()+ Option::len_except_value, NEGO_END_MSG, session_id);
					std::cout << "The negotiation is over loop_count, so decline it" << std::endl;
					return rtnval;
				}
				//send new NEGO_MSG
				value_len = strlen((char*)asa_answer);

				nego_bits = nego_obj_opt2bits(asa_answer,value_len);
				memcpy(send_buffer, nego_bits , value_len + Objective_Option::len_except_value);

				rtnval = write_pdu((const void *)send_buffer,MAXSTRINGLENGTH, NEGO_MSG, session_id);
				store_last_options(send_buffer, MAXSTRINGLENGTH);

				if(rtnval != SUCCESS) return rtnval;
				//recv pdu
				rtnval = read_pdu((char*)buffer,buffer_size,type,actual_session_id);
				while(actual_session_id != session_id)
				{
					//dieWithUserMessager("session_id not match");
					//return CLIENT_RECV_NOMATCHED_SESSION_ID_ERR;
					rtnval = read_pdu((char*)buffer,buffer_size,type,actual_session_id);
				}
				//distribute
				rtnval = do_negotiate(buffer,buffer_size,type);

			break;

		case WAIT_MSG:
			//if(loop_count == 0) return ERROR;
			//sleep for a little while, then read and do_negotiate
			cur_states = WAIT;

			if(opt_type == Waiting_time)
			{
				Timeout.tv_sec = (int) * opt_vlaue;
			}
			else
			{
				rtnval = ERROR;
				break;
			}

			setsockopt(tcp_sock,SOL_SOCKET,SO_RCVTIMEO,(const void *)&Timeout,(socklen_t)sizeof(struct timeval));

			rtnval = read_pdu((char*)buffer,buffer_size,type,actual_session_id);

			if(rtnval != SUCCESS) break;

			while(actual_session_id != session_id)
			{
				//dieWithUserMessager("session_id not match");
				//return CLIENT_RECV_NOMATCHED_SESSION_ID_ERR;
				rtnval = read_pdu((char*)buffer,buffer_size,type,actual_session_id);
			}

			//reset Timeout and distribute new received msg
			Timeout.tv_sec = 10;
			setsockopt(tcp_sock,SOL_SOCKET,SO_RCVTIMEO,(const void *)&Timeout,(socklen_t)sizeof(struct timeval));

			rtnval = do_negotiate(buffer,buffer_size,type);
			break;

		case NEGO_END_MSG:
			//end_tcp
			if(opt_type == Accept)
			{
				std::cout << "The negotiation is accepted" << std::endl;
				Objective_Option obj_opt = Objective_Option::parse_bits((uint16_t *)lastTopOptions);
				do_configuration(obj_opt.get_value());
			}
			else if (opt_type == Decline)
				std::cout << "The negotiation is declined" << std::endl;
			rtnval = SUCCESS;
			break;

		default:
			//wrong type, read and do_negotiate
			//return CLIENT_RECV_UNEXCEPETD_MSG_TYPE_ERR;
			rtnval = ERROR;
			break;
	}
	return rtnval;
}

/*************************************************************************
*  Function name: Client::nego_obj_opt2bits
*  Description: Conversion function from Objective_Option to bits
*  Parameter: 	const void* opt_data	//pointer to Objective_Option
*  				uint16_t data_size	//size of Objective_Option
*  Return: 		uint16_t *				//pointer to bits
*  Remark:
*  Lastly modified by Kangning Xu on 15-5-25
*************************************************************************/
uint16_t * Client::nego_obj_opt2bits(const void* opt_data, uint16_t data_size)
{
	Objective_Option nego_obj_opt(Negotiation, data_size, (uint8_t*)opt_data, loop_count, flag);
	return nego_obj_opt.to_bits();
}

/*************************************************************************
*  Function name: Client::negotiate
*  Description: Function for synchronize in class Client, a specific kind of negotiation in which loop count equals 1
*  Parameter: 	const void * buffer_obj	//pointer to synchronize objective
*  Return: 		ERRNO
*  Remark:
*  Lastly modified by Kangning Xu on 15-5-25
*************************************************************************/
ERRNO Client::synchronize(const void* buffer_obj)
{
	std::cout << "Synchronizing start!" << std::endl;
	ERRNO rtnval;
	struct sockaddr_in6 BroadcastAddr;


	//check the negoAddr: default broadcast address is not acceptable
	memset(&BroadcastAddr,0,sizeof(BroadcastAddr));
	BroadcastAddr.sin6_family = AF_INET6;
	inet_pton(AF_INET6, "ff02::1", &BroadcastAddr.sin6_addr);
	BroadcastAddr.sin6_port = htons(4444);

	if(sockAddrEqual(negoAddr,BroadcastAddr))
	{
		std::cout << "wrong synchronization address!" << std::endl;
		return ERROR;
	}

	session_id= generate_random()%(MAX_SESSION_ID+1);
	loop_count = 1;
	flag = 0;

	uint16_t value_len = strlen((char*)buffer_obj);
	Objective_Option obj_opt(Synchronization, value_len, (uint8_t*)buffer_obj,loop_count,flag);
	uint16_t * bits = obj_opt.to_bits();


	char buffer[MAXSTRINGLENGTH];

	memcpy(buffer,(char *)bits,value_len + Objective_Option::len_except_value);

	//to avoid bed influence from running nego_thread
	cur_states = OFF;

	//nego: only once
	rtnval = send_tcp(buffer,MAXSTRINGLENGTH,REQUEST_MSG);

	//give result to upper through recved_obj_opt
	Objective_Option recved_obj_opt = Objective_Option::parse_bits((uint16_t *)buffer);
	//option_type obj_opt_type = recved_obj_opt.get_type();
	//uint16_t obj_opt_len = recved_obj_opt.get_len();
	//uint8_t * obj_opt_vlaue = recved_obj_opt.get_value();

	//reset current state
	cur_states = OFF;

	std::cout << "Synchronizing end!" << std::endl;
	do_configuration(recved_obj_opt.get_value());
	return rtnval;
}
