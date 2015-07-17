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
* File : [common_structs.h]
* Description : This file contains several commonly used structs, such as processing queue item using in server session and parameter needed by threads running function.
* Remark : 
* Modification record:
* Created by leeying in Beijing University of Posts and Telecommunications, in Oct 2013.
* Revised by Cheng Pang and Kangning Xu in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/

#ifndef demo_common_structs_h
#define demo_common_structs_h

#include "msg.h"
#include <string.h>

// structure of processing queue item using in server session
typedef struct content{
    enum MSG_TYPE type;
    char data[MAXSTRINGLENGTH];

/*************************************************************************
*  Function name:operator ==
*  Description:overload operator ==
*  Parameter: 	const struct content &c	//to compare with the one who called this function
*  Return:	true or false
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-29
*************************************************************************/
    bool operator == (const struct content &c) const{
        if(type == c.type && strcmp(data,c.data) == 0){
            return 1;
        }
        return 0;
    }


/*************************************************************************
*  Function name:operator =
*  Description:overload operator =
*  Parameter: 	const struct content &c	//assign its value to the one who called this function
*  Return:	struct content&
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-29
*************************************************************************/
    struct content& operator=(const struct content &c){
        type = c.type;
        strcpy(data,c.data);
        return *this;
    }

}content;


class ServerMaster;
class ServerSession;

// structure of parameter needed by threads running function
typedef struct{
    // pointer instance to a ServerMaster 
    ServerMaster* sm;
    // pointer instance to a ServerSession 
    ServerSession* ss;
}session_run_parms;


#endif
