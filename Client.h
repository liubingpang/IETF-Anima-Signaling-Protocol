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
* File : [Client.h]
* Description : This class is used in client side by Manager while the role set as client, and to send or receive message according to its current state.
* Remark : 
* Modification record:
* Created by leeying in Beijing University of Posts and Telecommunications, in Oct 2013.
* Revised by Cheng Pang and Kangning Xu in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/

#ifndef __demo__Client__
#define __demo__Client__

#include "BaseNegotiator.h"
#include "Option.h"
#include <unistd.h>
#include <string.h>

// set maximum of try times
#define MAX_TRY_TIMES 5

// state of client
enum client_states{
    OFF = 1,
    WAIT_RESPONSE = 2,
    WAIT = 3,
    INFORMED = 4,
    END = 5,
	 NEGOING = 6
};

class Client:public BaseNegotiator{
private:
    // try times
    int try_times;
    // upper data send last time
    char lastTopOptions[MAXSTRINGLENGTH];
    // timers
    struct timeval response_timer;
    struct timeval wait_timer;

    uint32_t session_id;
    client_states cur_states;
    struct sockaddr_in6 serverAddr;
    struct sockaddr_in6 negoAddr;

    char buffer_nego_obj[MAXSTRINGLENGTH];
    int loop_count;
    int flag;

    // increase try times
    void increase_try_times();
    // reset try times
    void reset_try_times();
	// store upper data sent last time
    void store_last_options(const void* buffer,size_t buffer_size);

    //State functions
    ERRNO send_discover(const void* buffer,size_t buffer_size,enum MSG_TYPE type);
    ERRNO send_in_INFORMED(const void* buffer,size_t buffer_size, enum MSG_TYPE type);
    ERRNO recv_in_time(const void* buffer,enum MSG_TYPE &type,struct timeval tv);
    ERRNO recv_in_WAIT_RESPONSE(const void* buffer,enum MSG_TYPE &type);
    ERRNO recv_in_WAIT(const void* buffer,enum MSG_TYPE &type);

    ERRNO send(const void* buffer,size_t buffer_size,enum MSG_TYPE type);
    ERRNO recv(const void* buffer,enum MSG_TYPE &type);

    static void *nego_thread(void* _client);
    ERRNO send_tcp(const void* buffer,size_t buffer_size,enum MSG_TYPE type);


    ERRNO send_negotiate(const void* buffer,size_t buffer_size,enum MSG_TYPE type);
    ERRNO do_negotiate(const void* buffer,size_t buffer_size,enum MSG_TYPE type);
    uint16_t * nego_obj_opt2bits(const void* opt_data, uint16_t data_size);

	 // clear up after discovery
    void clearup();

    /*************************************************************************
    *  Function name: Client::get_curr_state
    *  Description: get current state of client
    *  Parameter: none
    *  Return: client_states	//current state of client
    *  Remark:
    *  Modification record:
    *  Lastly modified by Kangning Xu on 15-04-28
    *************************************************************************/
        client_states get_curr_state(){
            return this->cur_states;
        }


public:
    // constructor
    Client();

    // destructor
    ~Client();

    ERRNO discover();

    ERRNO negotiate(const void* buffer_obj);

    ERRNO synchronize(const void* buffer_obj);

/*************************************************************************
*  Function name : Client::asa_geq_fn
*  Description:provided by the ASA for comparing whether the value  is equal, should be overwritten
*  Parameter:		void * value_a
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
*  Function name : Client::asa_negotiate_result
*  Description:provided by the ASA for GDNP to pass the negotiated value to ASA and return the value for negotiation,, should be overwritten
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

/*************************************************************************
*  Function name : Client::do_configuration
*  Description:provided by the ASA for GDNP to do configuration by using negotiation result, should be overwritten
*  Parameter:	void * nego_result
*  Return:void
*  Remark:virtual function
*  Modification record:
*  Lastly modified by Cheng Pang on 15-05-21
*************************************************************************/
	virtual void do_configuration(const void * nego_result)
	{
		std::cout << "do configuration...." << std::endl;
	}


};

#endif /* defined(__demo__Client__) */
