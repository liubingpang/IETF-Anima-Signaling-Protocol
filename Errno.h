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
* File : [Errno.h]
* Description : This file contains some constants, such as specific time-out and all of the error number.
* Remark : 
* Modification record:
* Created by leeying in Beijing University of Posts and Telecommunications, in Oct 2013.
* Revised by Cheng Pang and Kangning Xu in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/
#ifndef demo_Errno_h
#define demo_Errno_h

// define the length of array storaging IP addresses
#define IP_str_len  INET6_ADDRSTRLEN

// define time-out
#define RESONSE_TIMEOUT_SECOND 10
#define WAIT_TIMEOUT_SECOND 10
#define PROCESSING_TIMEOUT_SECOND 8
#define END_TIMEOUT_SECOND 20

// error code
enum ERRNO{
    // send error
    SEND_ERR = -2,
    // send unexpected bytes error
    SEND_UNEXPECTED_BYTES_ERR = -3,
    // upper option is too long
    OPTIONS_TOO_LONG_ERR = -4,
    // session_id is too long 
    SESSION_ID_TOO_LONG_ERR = -5,
    // wrong message type
    MSG_TYPE_ERR = -6,
    NULL_POINT_ERR = -7,
    // encode error happen when packing data
    ENCODE_ERR = -8,

    // receive error
    RECV_ERR = -9,

    // bind error
    BIND_ERR = -10,

    SERVER_SEND_UNEXCEPTED_MSG_TYPE_ERR = -11,

    // server comes into a unexpected state during a session
    SERVER_UNEXCEPTED_STATE_ERR = -12,

    SERVER_RECV_UNEXCEPTED_MSG_TYPE_ERR = -13,

    CLIENT_SEND_UNEXCEPTED_MSG_TYPE_ERR = -14,

    // negotiation failed
    CLIENT_RECV_NOTHING_ERR = -15,

    CLIENT_UNEXCEPTED_STATE_ERR = -16,

    CLIENT_RECV_UNEXCEPETD_MSG_TYPE_ERR = -17,

    SELECT_ERR = -18,

    UNEXCEPTE_SERVER_SOCKADDR = -19,

    ADDR_ERR =-20,

    CLIENT_RECV_NOMATCHED_SESSION_ID_ERR = -21,

	SOCK_NOT_INIT = -22,
    ERROR = -1,
    SUCCESS = 1,
	
	//time-out
    TIMEOUT = 0
};

void dieWithUserMessager(const char* msg);

#endif
