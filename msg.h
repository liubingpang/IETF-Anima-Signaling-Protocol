/*
************************************************************************
*                GDNP API v1.0
* Draft : https://tools.ietf.org/html/draft-carpenter-anima-gdn-protocol-03
* Abstract
* 		This document establishes requirements for a protocol that enables
*		intelligent devices to dynamically discover peer devices, to
*		synchronize state with them, and to negotiate parameter settings
*		mutually with them.  The document then defines a general protocol for
*		discovery, synchronization and negotiation, while the technical
*		objectives for specific scenarios are to be described in separate
*		documents.  An Appendix briefly discusses existing protocols with
*		comparable features.
*
* File:[msg.h]
* Description: some rules and format about message
* Remark:
* Modification record:
* Created by leeying in Beijing University of Posts and Telecommunications, in Oct 2013.
* Revised by Cheng Pang and Kangning Xu in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/

#ifndef MSG_H
#define MSG_H

#include <stdint.h>
#include <iostream>
#include "Errno.h"

enum MSG_TYPE{
	DISCOVERY_MSG = 0x01,
    RESPONSE_MSG = 0X02,
	REQUEST_MSG = 0x03,
	NEGO_MSG = 0x04,
    NEGO_END_MSG = 0x05,
    WAIT_MSG = 0x06
};

enum{
    // 8 bits for msg-type
    MSG_TYPE_SIZE = 8,
    // 24 bits for session_id
    SESSION_ID_SIZE = 24,
    // session_id values between 0 and 16777215
    MAX_SESSION_ID = 16777215,
    // the maximum of data length set as 1000 characters
    MAXSTRINGLENGTH = 1000
};


typedef struct msg{
    uint32_t header;
    // device id
    uint32_t device_id;
    // data
    char data[MAXSTRINGLENGTH+1];
}msg;


ERRNO encode(msg* msg_p,enum MSG_TYPE type,uint32_t session_id,const void* data,size_t data_size);
ERRNO decode(msg* msg_p,enum MSG_TYPE &type,uint32_t &session_id,char* data,size_t &data_size);
// get a random
uint32_t generate_random();

#endif
