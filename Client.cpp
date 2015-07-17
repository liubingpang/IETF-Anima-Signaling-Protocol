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
* File : [Client.cpp]
* Description : This class is used in client side by Manager while the role set as client, and to send or receive message according to its current state.
* Remark : 
* Modification record:
* Created by leeying in Beijing University of Posts and Telecommunications, in Oct 2013.
* Revised by Cheng Pang and Kangning Xu in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/

#include "Client.h"
#include <stdio.h>

// initialize try times
// int Client::try_times = 0;

/*************************************************************************
*  Function name: Client::increase_try_times
*  Description: try times plus one
*  Parameter: none
*  Return: void
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-28
*************************************************************************/
void Client::increase_try_times()
{
    try_times++;
}

/*************************************************************************
*  Function name: Client::reset_try_times
*  Description: set try times as zero
*  Parameter: none
*  Return: void
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-28
*************************************************************************/
void Client::reset_try_times()
{
    try_times = 0;
}
 
/*************************************************************************
*  Function name: Client::store_last_options
*  Description: store upper data sent last time
*  Parameter: 	const void* buffer		//pointer to data in last option
					size_t buffer_size		//length of data in last option
*  Return: void
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-28
*************************************************************************/
void Client::store_last_options(const void* buffer,size_t buffer_size)
{
    memset(lastTopOptions, 0, MAXSTRINGLENGTH);
    memcpy(lastTopOptions, buffer, buffer_size);
    //std::cout<<"lastTopOptions = "<<lastTopOptions<<std::endl;
}

/*************************************************************************
*  Function name: Client::clearup
*  Description: clean up after negotiation
*  Parameter: none
*  Return: void
*  Remark:
*  Modification record:
*  Lastly modified by pc on 15-05-07
*************************************************************************/
void Client::clearup()
{
    // close socket
    // close(sock);
}
Client::Client():cur_states(OFF)
{
    // fill in sin6_addr
    inet_pton(AF_INET6, "ff02::1", &serverAddr.sin6_addr);
    inet_pton(AF_INET6, "ff02::1", &negoAddr.sin6_addr);

    cur_states = OFF;
    memset(buffer_nego_obj, 0, MAXSTRINGLENGTH);
    session_id = 0;
    loop_count = 5;
    flag = 0;
    // initialize response timer
    response_timer.tv_sec = RESONSE_TIMEOUT_SECOND;
    response_timer.tv_usec = 0;

    wait_timer.tv_sec = WAIT_TIMEOUT_SECOND;
    wait_timer.tv_usec = 0;

    // initialize try times
    try_times = 0;
}


/*************************************************************************
*  Function name: Client::~Client
*  Description: destructor of Client
*  Parameter: none
*  Return: none
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-28
*************************************************************************/
Client::~Client()
{
		//std::cout<<"running client's destruct function"<<std::endl;
}

/*************************************************************************
*  Function name: Client::send
*  Description: send request according to different message type and current state of client
*  Parameter: 	const void* buffer	//pointer to data 
					size_t buffer_size	//length of data
					enum MSG_TYPE type	//type of message
*  Return: 		ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-28
*************************************************************************/
ERRNO Client::send(const void* buffer,size_t buffer_size,enum MSG_TYPE type)
{
    ERRNO rtnval;
    switch (cur_states) {
        case OFF:
        	//std::cout<<"ready to send in OFF:"<<std::endl;
            rtnval = send_discover(buffer, buffer_size,type);
            if(rtnval == SUCCESS){
                // store the request data
                store_last_options(buffer, buffer_size);
                cur_states = WAIT_RESPONSE;
            }
            break;
        case WAIT_RESPONSE:
            // working in recv
            if(try_times < MAX_TRY_TIMES){
                // resend request message
                rtnval = send_discover(buffer, buffer_size,type);
                increase_try_times();
            }
            else{
                // exceeds the maximum of try times
                cur_states = END;
                rtnval = CLIENT_RECV_NOTHING_ERR;
            }
            break;
        case WAIT:
            //dieWithUserMessager("client doesn't allow send in status WAIT");
            rtnval = CLIENT_SEND_UNEXCEPTED_MSG_TYPE_ERR;
            break;
        case INFORMED:
            rtnval = send_in_INFORMED(buffer, buffer_size,type);
            break;
        case END:
           // dieWithUserMessager("client doesn't allow send in status END");
            rtnval = CLIENT_SEND_UNEXCEPTED_MSG_TYPE_ERR;
            break;
        default:
            rtnval = CLIENT_UNEXCEPTED_STATE_ERR;
            break;
    }

    return rtnval;
}

/*************************************************************************
*  Function name: Client::recv
*  Description: receive a message in different state of client
*  Parameter: 	const void* buffer	//store data of received message 
					enum MSG_TYPE &type	//store type of received message
*  Return: 		ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-28
*************************************************************************/
ERRNO Client::recv(const void* buffer,enum MSG_TYPE &type)
{
    ERRNO rtnval;
    switch (cur_states)
    {
        case OFF:
            rtnval = CLIENT_RECV_UNEXCEPETD_MSG_TYPE_ERR;
            break;
        case WAIT_RESPONSE:
            while(cur_states == WAIT_RESPONSE){
                if((rtnval = recv_in_WAIT_RESPONSE(buffer, type)) == TIMEOUT && cur_states == WAIT_RESPONSE ){
                    // resend request message
                    rtnval = send(lastTopOptions,strlen(lastTopOptions), DISCOVERY_MSG);
                }
            }

            reset_try_times();
            break;
        case WAIT:
            rtnval = recv_in_WAIT(buffer, type);
            if(rtnval == TIMEOUT){// timeout
                cur_states = END;
                rtnval = CLIENT_RECV_NOTHING_ERR;
            }
            break;
        case INFORMED:
            rtnval = CLIENT_RECV_UNEXCEPETD_MSG_TYPE_ERR;
            break;
        case END:
            rtnval = CLIENT_RECV_UNEXCEPETD_MSG_TYPE_ERR;
            break;
        default:
            rtnval = CLIENT_UNEXCEPTED_STATE_ERR;
            break;
    }
    return rtnval;
}


/*************************************************************************
*  Function name: Client::discover
*  Description: Function for discover in class Client using UDP
*  Parameter: 	const void * buffer_obj	//pointer to discover objective
*  Return: 		ERRNO
*  Remark:
*  Lastly modified by Kangning Xu on 15-5-25
*************************************************************************/
ERRNO Client::discover()
{
    // Client
	std::cout << "discovery start!!" << std::endl;
	 ERRNO rtnval;
    int sock = socket(AF_INET6,SOCK_DGRAM,0);
    if(sock < 0)
    {
        dieWithUserMessager("socket failed");
    }
    int broadcastPerm = 1;
    if(setsockopt(sock,SOL_SOCKET,SO_BROADCAST,&broadcastPerm,sizeof(broadcastPerm)) < 0)
    {
        dieWithUserMessager("set sockopt failed");
    }
    session_id= generate_random()%(MAX_SESSION_ID+1);


    // send a discover request
    enum MSG_TYPE type;
    char buffer[MAXSTRINGLENGTH];

    Objective_Option obj_opt(Discovery, 0, (uint8_t*)"",0,0);
    uint16_t * bits = obj_opt.to_bits();
    memcpy(buffer, bits, Objective_Option::len_except_value);

    client_udp_init(sock, "ff02::1", serverAddr);

    //std::cout<<"ready to send:"<<std::endl;
    rtnval = send(buffer, strlen(buffer), DISCOVERY_MSG);

    if(rtnval != SUCCESS)
    {
    	dieWithUserMessager("sendto failed");
		//std::cout<<rtnval<<std::endl;
      return rtnval;
    }

    // waiting for response
    //rtnval = m.nrecvfrom(nsock,session_id,buffer,type);

    rtnval = recv(buffer,type);
    if(rtnval == SUCCESS)
    {
    	cur_states = OFF;
    	close(sock);
    	close_udp();
    }

    if(rtnval != SUCCESS)
    {
    	dieWithUserMessager("recv failed");
    }
    else
    {
    	if(type == RESPONSE_MSG)
		{
			Option recved_opt = Option::parse_bits((uint16_t *)buffer);
			option_type type = recved_opt.get_type();
			char * recved_addr;

			if(type == Locator)
			{
				recved_addr = (char*)recved_opt.get_value();
				std::cout<<"Server ip6: "<< recved_addr << std::endl;
				inet_pton(AF_INET6, recved_addr, &negoAddr.sin6_addr);
//        		struct in6_addr ip = negoAddr.sin6_addr;
//        		printf("0x%x%x%x%x\n", htonl(ip.s6_addr32[0]),htonl(ip.s6_addr32[1]),htonl(ip.s6_addr32[2]),htonl(ip.s6_addr32[3]));
//        		char s[64]={"\0"};
//        		inet_ntop(AF_INET6,(void*)&ip,s,64);
//				std::cout <<"sin6_addr "<< s << std::endl;
			}
			else return ERROR;
		}
//    	Option recved_opt = Option::parse_bits((uint16_t *)buffer);
//		std::cout <<"type  "<< recved_opt.get_type() <<std::endl
//					 <<"len  "<< recved_opt.get_len() << std::endl
//					 <<"value  "<< (char*)recved_opt.get_value() << std::endl;
    }
//	strcpy((char *)buffer_obj,(char*)recved_opt.get_value());
    std::cout << "dicovery end!!" <<std::endl;
   //m.nclose("",0);
    return SUCCESS;
}
