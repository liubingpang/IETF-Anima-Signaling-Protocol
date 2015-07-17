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
* File : [BaseNegotiator.cpp]
* Description : This class is used for basic communication, such as send pdu and receive pdu.
* Remark : 
* Modification record:
* Created by leeying in Beijing University of Posts and Telecommunications, in Oct 2013.
* Revised by Cheng Pang and Kangning Xu in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/

#include "BaseNegotiator.h"
#include "Option.h"
#include <errno.h>

/*************************************************************************
*  Function name:BaseNegotiator::BaseNegotiator
*  Description:constructor of BaseNegotiator
*  Parameter:	none
*  Return:none
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-05-18
*************************************************************************/
BaseNegotiator::BaseNegotiator(){
	udp_sock = -1;
	tcp_sock = -1;
	// Construct server side address structure
	//	memset(&serverAddr,0,sizeof(serverAddr));
	//	serverAddr.sin6_family = AF_INET6;
	//	serverAddr.sin6_port = htons(port);
}

/*************************************************************************
*  Function name:BaseNegotiator::server_udp_init
*  Description:client udp socket init
*  Parameter:	none
*  Return:ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-05-20
*************************************************************************/
ERRNO BaseNegotiator::server_udp_init(const int udp_sock)
{
	this->udp_sock = udp_sock;

	// fill in sin6
	struct sockaddr_in6 inet_addr;
	memset(&inet_addr,0,sizeof(inet_addr));
	inet_addr.sin6_family = AF_INET6;
	inet_addr.sin6_port = htons(port);
	inet_addr.sin6_addr = in6addr_any;
	// bind
	if(0 != (bind(udp_sock,(struct sockaddr*)&inet_addr,sizeof(inet_addr))))
	{
	   return BIND_ERR;
	}
	return SUCCESS;
}

/*************************************************************************
*  Function name:BaseNegotiator::client_udp_init
*  Description:server udp socket init
*  Parameter:	none
*  Return:ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-05-20
*************************************************************************/
ERRNO BaseNegotiator::client_udp_init(const int udp_sock,const char serverIP[], struct sockaddr_in6 &server_addr)
{
	this->udp_sock = udp_sock;

	// fill in sin6_addr
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin6_family = AF_INET6;
	server_addr.sin6_port = htons(port);
	int rtnVal = inet_pton(AF_INET6, serverIP, &server_addr.sin6_addr);
	if(rtnVal == 0){
		dieWithUserMessager("construct targetAddr failed!:invalid address string");
		return ERROR;
	}
	else if(rtnVal < 0 ){
		dieWithUserMessager("construct targetAddr failed!");
		return ERROR;
	}

	return SUCCESS;
}

/*************************************************************************
*  Function name:BaseNegotiator::server_tcp_init
*  Description:server tcp socket init
*  Parameter:	none
*  Return:ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-05-20
*************************************************************************/
ERRNO BaseNegotiator::server_tcp_init(const int listen_sock)
{

	struct sockaddr_in6 inet_addr;
	memset(&inet_addr,0,sizeof(inet_addr));
	inet_addr.sin6_family = AF_INET6;
	inet_addr.sin6_port = htons(port);
	inet_addr.sin6_addr = in6addr_any;

	// bind
	if(0 != (bind(listen_sock,(struct sockaddr*)&inet_addr,sizeof(inet_addr))))
	{
	   return BIND_ERR;
	}

	if( -1 == listen(listen_sock, 10))
	{
	    //listen
		dieWithUserMessager("listen socket error: %s(errno: %d)\n");
		return ERROR;
	}

	return SUCCESS;
}



/*************************************************************************
*  Function name:BaseNegotiator::client_tcp_init
*  Description:client tcp socket init
*  Parameter:	none
*  Return:ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-05-20
*************************************************************************/
ERRNO BaseNegotiator::client_tcp_init(const int tcp_sock, const char serverIP[])
{
	this->tcp_sock = tcp_sock;

	struct sockaddr_in6 server_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin6_family = AF_INET6;
	server_addr.sin6_port = htons(port);
	int rtnVal = inet_pton(AF_INET6, serverIP, &server_addr.sin6_addr);
	if(rtnVal == 0){
		dieWithUserMessager("construct targetAddr failed!:invalid address string");
		return ERROR;
	}
	else if(rtnVal < 0 ){
		dieWithUserMessager("construct targetAddr failed!");
		return ERROR;
	}

	if( connect(tcp_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		dieWithUserMessager("connect error");
	}

	return SUCCESS;
}


/*************************************************************************
*  Function name:BaseNegotiator::send_pdu
*  Description:To send a pdu(Protocol Data Unit) by udp
*  Parameter:	const void* data
				size_t buffer_size
				enum MSG_TYPE type
				uint32_t session_id
				struct sockaddr_in6 targetAddr
*  Return:ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-28
*************************************************************************/
ERRNO BaseNegotiator::send_pdu(const void* data,size_t buffer_size,enum MSG_TYPE type,uint32_t session_id,struct sockaddr_in6 targetAddr){

	if(udp_sock < 0)
	{
		dieWithUserMessager("udp_sock not init");
		return SOCK_NOT_INIT;
	}
    // encapsulate PDU
    msg pdu;
    int rtnval = encode(&pdu, type, session_id, data,buffer_size);
//    std::cout << "send type"<< type << std::endl;
    if(rtnval != SUCCESS){
        return ENCODE_ERR;
    }

    // send PDU
    ssize_t numBytes = sendto(udp_sock,&pdu, sizeof(pdu), 0, (struct sockaddr*)&targetAddr,sizeof(targetAddr));
    if(numBytes < 0){
        if(errno == EADDRNOTAVAIL){
            return ADDR_ERR;
        }
        return SEND_ERR;
    }
    else if((size_t)numBytes < sizeof(pdu)){
        return SEND_UNEXPECTED_BYTES_ERR;
    }

    return SUCCESS;
}

/*************************************************************************
*  Function name:BaseNegotiator::recv_pdu
*  Description:receive a pdu(Protocol Data Unit) by udp
*  Parameter:	char data[]
				size_t &buffer_size
				enum MSG_TYPE &type
				struct sockaddr_in6 &fromAddr
				uint32_t &session_id
*  Return:ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-28
*************************************************************************/
ERRNO BaseNegotiator::recv_pdu(char data[],size_t &buffer_size,enum MSG_TYPE &type,struct sockaddr_in6 &fromAddr,uint32_t &session_id){
	if(udp_sock < 0)
	{
		dieWithUserMessager("udp_sock not init");
		return SOCK_NOT_INIT;
	}

	socklen_t fromAddrLen = sizeof(fromAddr);

    msg pdu_buf;
    ssize_t numBytes = recvfrom(udp_sock, &pdu_buf, sizeof(pdu_buf), 0, (struct sockaddr*)& fromAddr, &fromAddrLen);
    if(numBytes < 0){
        return RECV_ERR;
    }

    memset(data, 0, MAXSTRINGLENGTH);
    ERRNO rtnval = decode(&pdu_buf,type, session_id, data,buffer_size);

//    std::cout << "recv type"<<  type << std::endl;

    return rtnval;
}

/*************************************************************************
*  Function name:BaseNegotiator::write_pdu
*  Description:To send a pdu(Protocol Data Unit) by tcp
*  Parameter:	const void* data
						size_t buffer_size
						enum MSG_TYPE type
						uint32_t session_id
*  Return:ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-05-20
*************************************************************************/
ERRNO BaseNegotiator::write_pdu(const void* data,size_t buffer_size,enum MSG_TYPE type,uint32_t session_id)
{
	if(tcp_sock < 0)
	{
		dieWithUserMessager("tcp_sock not init");
		return SOCK_NOT_INIT;
	}
	// encapsulate PDU
	msg pdu;
	int rtnval = encode(&pdu, type, session_id, data,buffer_size);
	if(rtnval != SUCCESS){
		return ENCODE_ERR;
	}

//	Objective_Option recved_opt = Objective_Option::parse_bits((uint16_t *)data);
//			std::cout <<"before write   type  "<< recved_opt.get_type() <<std::endl
//						 <<"len  "<< recved_opt.get_len() << std::endl
//						 <<"value  "<< (char*)recved_opt.get_value() << std::endl <<buffer_size<< std::endl;
	// send PDU
	ssize_t numBytes = write(tcp_sock,&pdu, sizeof(pdu));
	if(numBytes < 0){
		if(errno == EADDRNOTAVAIL){
			return ADDR_ERR;
		}
		return SEND_ERR;
	}
	else if((size_t)numBytes < sizeof(pdu)){
		return SEND_UNEXPECTED_BYTES_ERR;
	}

	return SUCCESS;
}

/*************************************************************************
*  Function name:BaseNegotiator::read_pdu
*  Description:receive a pdu(Protocol Data Unit) by tcp
*  Parameter:	char data[]
						size_t &buffer_size
						enum MSG_TYPE &type
						uint32_t &session_id
*  Return:ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-05-20
*************************************************************************/
ERRNO BaseNegotiator::read_pdu(char data[],size_t &buffer_size,enum MSG_TYPE &type,uint32_t &session_id)
{
	if(tcp_sock < 0)
	{
		dieWithUserMessager("tcp_sock not init");
		return SOCK_NOT_INIT;
	}

	msg pdu_buf;
	ssize_t numBytes = read(tcp_sock, &pdu_buf, sizeof(pdu_buf));
	if(numBytes < 0){
		return RECV_ERR;
	}

	memset(data, 0, MAXSTRINGLENGTH);
	ERRNO rtnval = decode(&pdu_buf,type, session_id, data,buffer_size);
//	Objective_Option recved_opt = Objective_Option::parse_bits((uint16_t *)data);
//		std::cout <<"after read    type  "<< recved_opt.get_type() <<std::endl
//					 <<"len  "<< recved_opt.get_len() << std::endl
//					 <<"value  "<< (char*)recved_opt.get_value() << std::endl <<buffer_size<< std::endl;

	return rtnval;
}

/*************************************************************************
*  Function name:BaseNegotiator::sockAddrEqual
*  Description:to judge whether the actual address equals the expected address.
*  Parameter:	struct sockaddr_in6 actual
				struct sockaddr_in6 expected
*  Return: true or false
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-28
*************************************************************************/
bool BaseNegotiator::sockAddrEqual(struct sockaddr_in6 actual ,struct sockaddr_in6 expected){
    // if expected address equals the broadcast address, then return true
    char expectedIP[IP_str_len];
    inet_ntop(AF_INET6, &expected.sin6_addr,expectedIP,sizeof(expectedIP));
//    std::cout << strstr(expectedIP,"ff02::") << std::endl
//    		      << expectedIP <<std::endl;
//    if(strstr(expectedIP,"ff02::") == expectedIP){
//        return 1;
//    }
    // compare IP address
    char actualIP[IP_str_len];
    inet_ntop(AF_INET6, &actual.sin6_addr,actualIP,sizeof(actualIP));
    if(strcmp(actualIP, expectedIP) != 0){
        return 0;
    }
    // compare port
    int actual_port = ntohs(actual.sin6_port);
    int expected_port = ntohs(expected.sin6_port);
    if(actual_port != expected_port){
        return 0;
    }

    return 1;
}
