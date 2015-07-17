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
* File:[msg.cpp]
* Description: Definition of encode and decode
* Remark:
* Modification record:
* Created by leeying in Beijing University of Posts and Telecommunications, in Oct 2013.
* Revised by Cheng Pang and Kangning Xu in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/

#include "msg.h"
#include <stdlib.h>
#include <time.h>
#include <cstring>

/*************************************************************************
*  Function name: encode
*  Description: encode message by rules
*  Parameter:  msg_p   message to send
              	  	  type    message type
              	  	  session_id
              	  	  data    data to encode
              	  	  data_size
*  Return:ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-28
*************************************************************************/
ERRNO encode(msg* msg_p,enum MSG_TYPE type,uint32_t session_id,const void* data,size_t data_size){
    if(data_size > MAXSTRINGLENGTH){
        return OPTIONS_TOO_LONG_ERR;
    }
    if (session_id > MAX_SESSION_ID){
        return SESSION_ID_TOO_LONG_ERR;
    }
    // initialize msg_p
    memset(msg_p, 0, sizeof(msg));
    // set message type
    msg_p->header = type<<SESSION_ID_SIZE;
    // set session id
    msg_p->header |= session_id;

    // set a random device id
    msg_p->device_id = generate_random();

    // set data
    memcpy(msg_p->data, data, data_size);
    return SUCCESS;
}

/*************************************************************************
*  Function name: decode
*  Description: decode message by rules
*  Parameter: msg_p   received message
              	  	  type    message type
              	  	  session_id
              	  	  data    buffer for decoded data
              	  	  data_size
*  Return: ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-28
*************************************************************************/
ERRNO decode(msg* msg_p,enum MSG_TYPE &type,uint32_t &session_id,char data[],size_t &data_size){
    if(msg_p == NULL){
        return NULL_POINT_ERR;
    }
    // msg type
    int result = (msg_p->header) >> SESSION_ID_SIZE;
    switch (result) {
        case 1:
            type = DISCOVERY_MSG;
            break;
        case 2:
            type = RESPONSE_MSG;
            break;
        case 3:
            type = REQUEST_MSG;
            break;
        case 4:
            type = NEGO_MSG;
            break;
        case 5:
        	type = NEGO_END_MSG;
        	break;
        case 6:
        	type = WAIT_MSG;
        	break;
        default:
            return MSG_TYPE_ERR;
            break;
    }

    // session id
    session_id = 0;
    session_id = msg_p->header-(type<<SESSION_ID_SIZE);

    // device id
//    uint32_t device_id = msg_p->device_id;

    // data
    memcpy(data, msg_p->data,MAXSTRINGLENGTH);

    data_size = sizeof(msg_p->data);

    return SUCCESS;
}

/*************************************************************************
*  Function name: generate_random
*  Description: generate a random uint32_t
*  Parameter: none
*  Return: uint32_t
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-28
*************************************************************************/
uint32_t generate_random(){
    // using current time as seed
    srand((unsigned)time( NULL ));
    return (uint32_t)rand();
}

