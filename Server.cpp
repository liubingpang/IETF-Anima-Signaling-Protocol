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
* File:[Server.cpp]
* Description: Definition of class Server's member function
* Remark:
* Modification record: rewrite CNP's API to GDNP's API :    add run() server_init()     delete recv()
* Created by leeying in Beijing University of Posts and Telecommunications, in Oct 2013.
* Revised by Cheng Pang and Kangning Xu in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/

#include "Server.h"
#include "Option.h"
#include <netdb.h>
#include <algorithm>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>

/*************************************************************************
*  Function name:server_init
*  Description:initiation of GDNP server
*  Parameter:none
*  Return:ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-5-18
*************************************************************************/
ERRNO ServerMaster::server_init()
{
	int udp_sock = socket(AF_INET6,SOCK_DGRAM,0);
	if(udp_sock < 0)
	{
		dieWithUserMessager("nsocket failed");
	}
	server_udp_init(udp_sock);


    int x;
    x=fcntl(udp_sock,F_GETFL,0);
    fcntl(udp_sock,F_SETFL,x | O_NONBLOCK);

	FD_ZERO(&read_flags);
	FD_SET(udp_sock,&read_flags);

	std::cout << "Server inti" << std::endl;
	pthread_t tid;
	pthread_create(&tid, NULL,run_help, this);
	return SUCCESS;
}


/*************************************************************************
*  Function name:listen_negotiate
*  Description:
*  Parameter:
*  Return: ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-5-18
*************************************************************************/
ERRNO ServerMaster::listen_negotiate()
{
	ERRNO rtnval;
	listen_sock = socket(AF_INET6, SOCK_STREAM, 0);
	if(listen_sock < 0)
	{
		dieWithUserMessager("listen socket failed");
		return ERROR;
	}
	pthread_mutex_lock(&fdset_lock);
	rtnval = server_tcp_init(listen_sock);
	pthread_mutex_unlock(&fdset_lock);
	FD_SET(listen_sock,&read_flags);

	std::cout << "Server listen" << std::endl;

	return rtnval;
}

/*************************************************************************
*  Function name: stop_negotiate
*  Description:
*  Parameter:
*  Return: ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-5-18
*************************************************************************/
ERRNO ServerMaster::stop_negotiate()
{
	close_tcp();
	close(listen_sock);
	return SUCCESS;
}


/*************************************************************************
*  Function name:ServerMaster
*  Description:constructor of ServerMaster
*  Parameter:manager      the Manager of this ServerMaster
              serverIP
              serverPort
              nsock	      socket id
*  Return: none
*  Remark:
*  Modification record: some initiation move to server_init()
*  Lastly modified by Cheng Pang on 15-5-19
*************************************************************************/
ServerMaster::ServerMaster():BaseNegotiator()
{
	pthread_mutex_init(&fdset_lock,NULL);
	listen_sock = -1;
	tcp_accepted = 0;
    // store global IP address of your interface
    struct ifaddrs *ifap;
    if(getifaddrs(&ifap) == 0)
    {
        for(;ifap != NULL;ifap = ifap->ifa_next)
        {
            if (ifap->ifa_addr == NULL){
                continue;
            }
            if(ifap->ifa_addr->sa_family == AF_INET6){
                char host[IP_str_len];
                int rtnval = getnameinfo(ifap->ifa_addr,sizeof(struct sockaddr_in6),host, IP_str_len, NULL, 0, 0);
                if (rtnval == 0)
                {
                    char* p = strchr(host,'%');
                    if(p != NULL){
                        *p = '\0';
                    }
                    std::string s(host);
                    local_interfaces.push_back(s);
//                    std::cout  << s << std::endl;
                }
            }
        }// end for

    }
    freeifaddrs(ifap);
}

/*************************************************************************
*  Function name:~ServerMaster
*  Description:destructor of ServerMaster
*  Parameter:none
*  Return:none
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-29
*************************************************************************/
ServerMaster::~ServerMaster(){
		//std::cout<<"running SM's destruct function"<<std::endl;
}

/*************************************************************************
*  Function name:run_help
*  Description:statc function for a new thread
*  Parameter:none
*  Return: void
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-5-19
*************************************************************************/
void* ServerMaster::run_help(void *arg)
{
	ServerMaster* sm =  (ServerMaster*)arg;
	sm->run();
	 return 0;
}
/*************************************************************************
*  Function name: run
*  Description:Server listens and receive asynchronously for any message
*  Parameter:none
*  Return:void
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-5-19
*************************************************************************/
void ServerMaster::run()
{
	int flag;
	struct timeval wait_t;

//	char buffer[MAXSTRINGLENGTH] = {0};
//					uint32_t session_id;
//					enum MSG_TYPE type;
//					size_t buffer_size;
//					struct sockaddr_in6 client_addr;
//	 std::cout << "recving .." << std::endl;
//	 recv_pdu((char*)buffer,buffer_size,type, client_addr, session_id);
//	 std::cout << type << std::endl;
//    return;
	while(1)
	{
		if(udp_sock == -1) continue;
		pthread_mutex_lock(&fdset_lock);
		wait_t.tv_sec=2;
		wait_t.tv_usec=0;
		FD_ZERO(&read_flags);
		FD_SET(udp_sock,&read_flags);
		if(listen_sock != -1)
			FD_SET(listen_sock,&read_flags);
		for(int i = 0; i < tcp_accepted ; i++)
		{
			FD_SET(tcp_fd_set[i],&read_flags);
		}
		flag=select(FD_SETSIZE, &read_flags,NULL,NULL,&wait_t);
		pthread_mutex_unlock(&fdset_lock);
		if(flag==-1)
		{
			std::cout << "select error" << std::endl;
		}
		else if(flag==0)
		{
			std::cout << "waiting.." << std::endl;
			continue;
		}
		else
		{
			pthread_mutex_lock(&fdset_lock);
			//udp for discovery
			if(FD_ISSET(udp_sock,&read_flags))
			{
				pthread_mutex_unlock(&fdset_lock);
				// receive request
				char buffer[MAXSTRINGLENGTH] = {0};
				uint32_t session_id;
				enum MSG_TYPE type;
				size_t buffer_size;
				struct sockaddr_in6 client_addr;
				if(SUCCESS  != recv_pdu((char*)buffer,buffer_size,type, client_addr, session_id))
				{
					dieWithUserMessager("recv_pdu failed");
					continue;
				}

//				if(check_Addr(client_addr))	// Ignoring broadcast packets from itself
				{
					if(type == DISCOVERY_MSG)
					{
						std::cout << "receive a udp packet for discovery" << std::endl<<std::endl;
						if(true)//not divert
						{
							const char *data = local_interfaces[1].c_str();
							uint16_t value_len = strlen(data)*sizeof(char) ;
							Option option(Locator, value_len , (uint8_t*)data);
							uint16_t * bits = option.to_bits();
							send_pdu(bits, option.get_len() + Option::len_except_value, RESPONSE_MSG, session_id, client_addr);

//							Option recved_opt = Option::parse_bits((uint16_t *)bits);
//							std::cout <<"type  "<< recved_opt.get_type() <<std::endl
//										 <<"len  "<< recved_opt.get_len() << std::endl
//										 <<"value  "<< (char*)recved_opt.get_value() << std::endl << std::endl;

						}
						else//divert  demo
						{
							char *data = (char* )"Other Server Locator value";
							uint16_t value_len = strlen(data)*sizeof(char) ;
							Option locator_option(Locator, value_len , (uint8_t*)data);

							Option divert_option(Divert, locator_option.get_len() + Option::len_except_value , (uint8_t*)locator_option.to_bits());
							uint16_t * bits = divert_option.to_bits();
							send_pdu(bits, divert_option.get_len() + Option::len_except_value, RESPONSE_MSG, session_id, client_addr);
						}

					}
					else
					{
						 dieWithUserMessager("receive a udp packet not for discovery");
					}
				}
			}
			//tcp for negotiation
			else if(listen_sock != -1 && FD_ISSET(listen_sock, &read_flags))
			{
				pthread_mutex_unlock(&fdset_lock);
				if(tcp_accepted < MAX_CLIENTS_NUM)
				{
					tcp_sock = accept(listen_sock, (struct sockaddr*)NULL, NULL);
					tcp_fd_set[tcp_accepted++] = tcp_sock;
					std::cout << "accept a tcp socket" << std::endl;
				}
			}
			//tcp for negotiation
			else if(tcp_sock != -1 && FD_ISSET(tcp_sock, &read_flags))
			{
				pthread_mutex_unlock(&fdset_lock);
				std::cout << "receive a tcp packet" << std::endl;
				// receive request
				char buffer[MAXSTRINGLENGTH] = {0};
				uint32_t session_id;
				enum MSG_TYPE type;
				size_t buffer_size;
				if(SUCCESS !=read_pdu((char*)buffer, buffer_size, type , session_id))
				{
					dieWithUserMessager("recv_pdu failed");
					continue;
				}
				distribute(session_id, buffer, buffer_size, type);
			}
		}

	}
}

/*************************************************************************
*  Function name: check_Addr
*  Description: Ignoring broadcast packets from itself
*  Parameter: client_addr   a addr to checking not itself
*  Return: bool  true for not itself
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-5-20
*************************************************************************/
bool ServerMaster::check_Addr(struct sockaddr_in6 client_addr)
{
	char ip[IP_str_len];
	inet_ntop(AF_INET6, &client_addr.sin6_addr,ip,sizeof(ip));
	std::string tmp(ip);
	std::vector<std::string>::iterator iter = find(local_interfaces.begin( ), local_interfaces.end( ), tmp);

	if(iter == local_interfaces.end())
		return true;
	else
	{
		std::cout<<"Server received a packet from itself. ignored."<<std::endl;
		return false;
	}
}


/*************************************************************************
*  Function name: distribute
*  Description: distribute received message to specific thread
*  Parameter: clientAddr
*  	          session_id
*  	          buffer
*  	          buffer_size
*  	          type        message type
*  Return: void
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-29
*************************************************************************/
void ServerMaster::distribute(uint32_t session_id,const void* buffer,size_t buffer_size,enum MSG_TYPE type)
{
    // value
    content c;
    c.type = type;
    memcpy(c.data, buffer, buffer_size);

    std::map<uint32_t,ServerSession*>::iterator iter = ss_map.begin();

	while (iter != ss_map.end())
	{
		if(iter->first == session_id)
		{
			if(type == REQUEST_MSG)
			{
					dieWithUserMessager("old session should not send REQUEST_MSG");
					return;
			}
			// push into queue
            iter->second->queue_push(c);
            break;
        }
        iter++;
    }

    if(iter == ss_map.end())
    {
    	if(type != REQUEST_MSG)
		{
				dieWithUserMessager("New session must begin with REQUEST_MSG");
				return;
		}
		std::cout<<"start negotiation process"<<std::endl;

        // new SeverSession for processing
		ServerSession *ss = new ServerSession(this->tcp_sock,session_id,c);
		// store in ss_map
		ss_map.insert(std::map<uint32_t, ServerSession*>::value_type(session_id,ss));

        // parameters of threads running function
        session_run_parms parm;
        parm.sm = this;
        parm.ss = ss;

        // launch the thread
       pthread_t tid;
       pthread_create(&tid, NULL, ServerSession::run_help, &parm);

    }
}


/*************************************************************************
*  Function name: clear_when_session_end
*  Description: clean up after session finished
*  Parameter: usid  UniqueSession id
*  Return: void
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-29
*************************************************************************/
void ServerMaster::clear_when_session_end(uint32_t sessionId , int tcp_sock)
{
    //std::cout<<"clean up after session finished"<<std::endl;
    std::map<uint32_t,ServerSession*>::iterator ss_iter;
    /*std::cout<<"Before cleaning:"<<std::endl;
    ss_iter = ss_map.begin();
    while(ss_iter != ss_map.end()){
        std::cout<<ss_iter->first;
        ss_iter++;
    }*/

    // clear ServerSession instance
	ss_iter = ss_map.begin();
    while(ss_iter != ss_map.end()){
        if(ss_iter->first == sessionId){
            //std::cout<<"cleaning ServerSession....."<<std::endl;
            ServerSession* ss = ss_iter->second;
            // call destructor of ServerSession
            delete ss;
            ss = NULL;

            ss_map.erase(ss_iter);
        }
        ss_iter++;
    }

    for(int i = 0 ;i < tcp_accepted ;i++)
    {
    	if(tcp_fd_set[i] == tcp_sock)
    	{
    		int j =i;
    		while( j < tcp_accepted-1)
    		{
    			tcp_fd_set[j] = tcp_fd_set[j+1];
    			j++;
    		}
    	}
    	tcp_accepted--;
    }
     /*std::cout<<"After cleaning:"<<std::endl;
    ss_iter = ss_map.begin();
    while(ss_iter != ss_map.end()){
        std::cout<<ss_iter->first;
        ss_iter++;
    }*/
}

