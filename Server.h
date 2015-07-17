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
* File:[Server.h]
* Description:Definition of class ServerMaster.Class ServerMaster is the controller of server-side
* Remark:
* Modification record:
* Created by leeying in Beijing University of Posts and Telecommunications, in Oct 2013.
* Revised by Cheng Pang and Kangning Xu in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/

#ifndef ServerMaster_H
#define ServerMaster_H

//#include "BaseNegotiator.h"
//#include "UniqueSessionId.h"
#include "ServerSession.h"
//#include "common_structs.h"
#include <map>
#include <queue>
#include <vector>
#include <string>
#include <pthread.h>
#include <ifaddrs.h>

#define MAX_CLIENTS_NUM 5

class Manager;

class ServerMaster:public BaseNegotiator{
public:
    // constructor
    ServerMaster();
    // destructor
    virtual ~ ServerMaster();

    ERRNO server_init();
    ERRNO listen_negotiate();
    ERRNO stop_negotiate();

/*************************************************************************
*  Function name : ServerMaster::asa_geq_fn
*  Description : provided by the ASA for comparing whether the value is equal, should be overwritten
*  Parameter:	void * value_a
*  						void * value_b
*  Return:bool
*  Remark:virtual function
*  Modification record:
*  Lastly modified by Cheng Pang on 15-05-21
*************************************************************************/
	virtual bool asa_geq_fn(const void * value_a, const void * value_b)
	{
		return true;
	}

/*************************************************************************
*  Function name : ServerMaster::asa_negotiate_result
*  Description : provided by the ASA for GDNP to pass the negotiated value to ASA and return the value for negotiation,, should be overwritten
*  Parameter:	void * value
*  Return:void *    the value ASA want
*  Remark:virtual function
*  Modification record:
*  Lastly modified by Cheng Pang on 15-05-21
*************************************************************************/
	virtual void * asa_negotiate_result( void * value)
	{
		return value;
	}

    //  clean up after session finished
    void clear_when_session_end(uint32_t sessionId, int tcp_sock);
private:
    pthread_mutex_t fdset_lock;
    int listen_sock;
	fd_set read_flags;
    std::map<uint32_t, ServerSession*> ss_map;
    // value of the local network adapter
    std::vector<std::string> local_interfaces;
    int tcp_fd_set[MAX_CLIENTS_NUM];
    int tcp_accepted;

    bool check_Addr(struct sockaddr_in6 client_Addr);
    // distribute data to specific thread
    void distribute(uint32_t session_id,const void* buffer,size_t buffer_size,enum MSG_TYPE type);
     void run();
     static void* run_help(void *arg);

};

#endif /* defined(ServerMaster__) */
