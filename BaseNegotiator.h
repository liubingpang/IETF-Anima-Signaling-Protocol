/*
************************************************************************
*                GDNP API v1.0
* Draft : https://tools.ietf.org/html/draft-carpenter-anima-gdn-protocol-03
* Abstract : 	
*		This document establishes requirements for a protocol that enables
*		intelligent devices to dynamically discover peer devices, to
*		synchronize state with them, and to negotiate parameter settings
*		mutually with them.  The document then defines a general protocol for
*		discovery, synchronization and negotiation, while the technical
*		objectives for specific scenarios are to be described in separate
*		documents.  An Appendix briefly discusses existing protocols with
*		comparable features.
*
* File : [BaseNegotiator.h]
* Description : This class is used for basic communication, such as send pdu and receive pdu.
* Remark : 
* Modification record:
* Created by leeying in Beijing University of Posts and Telecommunications, in Oct 2013.
* Revised by Cheng Pang and Kangning Xu in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/

#ifndef __demo__BaseNegotiator__
#define __demo__BaseNegotiator__

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "msg.h"
#include "Errno.h"



class BaseNegotiator{

public:
    // constructor
    BaseNegotiator();


/*************************************************************************
*  Function name:BaseNegotiator::BaseNegotiator
*  Description:destructor of BaseNegotiator
*  Parameter:	int nsocket
*  Return:none
*  Remark:virtual destructor
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-28
*************************************************************************/
    virtual ~BaseNegotiator(){
		//std::cout<<"running BN's destruct function"<<std::endl;
	};



protected:
	const static int port = 4444;
	int udp_sock;
	int tcp_sock;

	void set_udp_sock(int udp_sock){this->udp_sock = udp_sock;}
	void set_tcp_sock(int tcp_sock){this->tcp_sock = tcp_sock;}

    ERRNO write_pdu(const void* data,size_t buffer_size,enum MSG_TYPE type,uint32_t session_id);
    ERRNO read_pdu(char data[],size_t &buffer_size,enum MSG_TYPE &type,uint32_t &session_id);
    ERRNO send_pdu(const void* data,size_t buffer_size,enum MSG_TYPE type,uint32_t session_id,struct sockaddr_in6 targetAddr);
    ERRNO recv_pdu(char data[],size_t &buffer_size,enum MSG_TYPE &type,struct sockaddr_in6 &fromAddr,uint32_t &session_id);
    //determine whether two socket addresses are equal
    bool sockAddrEqual(struct sockaddr_in6 actual ,struct sockaddr_in6 expected);

    ERRNO server_udp_init(const int udp_sockfd);
    ERRNO server_tcp_init(const int tcp_sockfd);
    ERRNO client_udp_init(const int udp_sock,const char serverIP[], struct sockaddr_in6 &server_addr);
	ERRNO client_tcp_init(const int tcp_sockfd,const char serverIP[]);

	void close_udp(){close(udp_sock);}
	void close_tcp(){close(tcp_sock);}

};

#endif /* defined(__demo__BaseNegotiator__) */
