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
* File:[ServerSession.h]
* Description:Definition of class ServerSession. Basic control unit for each session
* Remark:
* Modification record:
* Created by leeying in Beijing University of Posts and Telecommunications, in Oct 2013.
* Revised by Cheng Pang and Kangning Xu in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/

#ifndef ServerSession_H
#define ServerSession_H

#include <queue>
#include "BaseNegotiator.h"
#include "common_structs.h"
#include "Errno.h"
#include <pthread.h>

// server state
enum server_states{
    IDLE = 0,
    PROCESSING = 1,
    SESSION_END = 2
};

class ServerMaster;

// a negotiation session of Server
class ServerSession:public BaseNegotiator{
private:
    // pointer to the queue
    std::queue<content> *q;
    // session id
    uint32_t session_id;
    // current state
    enum server_states cur_state;
    // statelock
    pthread_mutex_t statelock;
    // queuelock
    pthread_mutex_t queuelock;



public:
    // constructor
    ServerSession(int nsocket,uint32_t session_id,content c);

    // destructor
	~ServerSession();

    // push into queue
    void queue_push(content c);
    // pop from queue
    content queue_pop();
    // determine if queue is empty
    bool queue_empty();

    // set/get current state
    enum server_states get_cur_state();
    void set_cur_state(enum server_states state);

    // thread running function 
    static void* run_help(void* arg);
    void run(ServerMaster* sm);

    // thread to send WAIT_MSG
    static void* wait_thread_handler(void* arg);

    // thread to cheack if the session ends
    static void* end_thread_handler(void* arg);

    // update state
    ERRNO  set_state(enum MSG_TYPE type);
    // send
    ERRNO send(const void* buffer, size_t buffer_size,enum MSG_TYPE type);
    int idle_check;
};

#endif /* defined(__demo__ServerSession__) */
