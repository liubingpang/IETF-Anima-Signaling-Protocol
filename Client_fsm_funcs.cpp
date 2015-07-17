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
* File : [Client_fsm_funcs.cpp]
* Description : Implementation of State functions in class Client: to send or receive message according to different message type and current state of client.
* Remark : 
* Modification record:
* Created by leeying in Beijing University of Posts and Telecommunications, in Oct 2013.
* Revised by Cheng Pang and Kangning Xu in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/

#include "Client.h"
#include <stdio.h>
#include <stdlib.h>


/*************************************************************************
*  Function name: Client::send_discover
*  Description: check message type and send DISCOVERY_MSG out while client is OFF or WAIT_NEGOATION_RESPONSE
*  Parameter: 	const void* buffer	//pointer to data 
					size_t buffer_size	//length of data
					enum MSG_TYPE type	//message type. Sending works with DISCOVERY_MSG only.
*  Return: 		ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-29
*************************************************************************/
ERRNO Client::send_discover(const void* buffer,size_t buffer_size,enum MSG_TYPE type){
    ERRNO rtnval;
    if(type != DISCOVERY_MSG){
        dieWithUserMessager("wrong function call!");
        rtnval = CLIENT_SEND_UNEXCEPTED_MSG_TYPE_ERR;
    }
    else{
        // send pdu
        rtnval = send_pdu(buffer,buffer_size,DISCOVERY_MSG,session_id,serverAddr);
        while(rtnval == ADDR_ERR)
        {
            // pick a port randomly
            int localPort = generate_random()%10000;
            if(localPort <= 2000){
                localPort += 2000;
            }
            std::cout<<"pick a port: localPort = "<<localPort<<std::endl;
            // bind port
            struct sockaddr_in6 localAddr;
            memset(&localAddr,0,sizeof(localAddr));
            localAddr.sin6_family = AF_INET6;
            localAddr.sin6_port = htons(localPort);
            inet_pton(AF_INET6,"::1",&localAddr.sin6_addr);
            int rtnVal = bind(udp_sock,(struct sockaddr*)&localAddr,sizeof(localAddr));
            if(rtnVal < 0)
            {
                perror("bind failed");
            }
            rtnval = send_pdu(buffer,buffer_size,DISCOVERY_MSG,session_id,serverAddr);
        }
    }
    return rtnval;
}


/*************************************************************************
*  Function name: send_in_INFORMED
*  Description: send message while state of client is INFORMED
*  Parameter: 	const void* buffer	//pointer to data 
					size_t buffer_size	//length of data
					enum MSG_TYPE type	//message type
*  Return: 		ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-29
*************************************************************************/
ERRNO Client::send_in_INFORMED(const void* buffer,size_t buffer_size,enum MSG_TYPE type){
    ERRNO rtnval;
    switch (type) {
        case REQUEST_MSG:
            rtnval = send_pdu(buffer,buffer_size,type,session_id,serverAddr);
            if(rtnval == SUCCESS){
                store_last_options(buffer, buffer_size);
                //get into next state
                cur_states = WAIT_RESPONSE;
            }
            break;
        case RESPONSE_MSG:
            dieWithUserMessager("client doesn't allowed to send response msg");
            rtnval = CLIENT_SEND_UNEXCEPTED_MSG_TYPE_ERR;
            break;
        case WAIT_MSG:
            dieWithUserMessager("client doesn't allowed to send wait msg");
            rtnval = CLIENT_SEND_UNEXCEPTED_MSG_TYPE_ERR;
            break;
        case NEGO_END_MSG:
            rtnval = send_pdu(buffer,buffer_size,type,session_id,serverAddr);
            if(rtnval == SUCCESS){
                //get into next state
                cur_states = END;
                clearup();
            }
            break;
        default:
            break;
    }

    return rtnval;
}


/*************************************************************************
*  Function name: recv_in_time
*  Description: receive a message in time
*  Parameter: 	const void* buffer	//store data of received message
					enum MSG_TYPE &type	//store type of received message
					struct timeval tv	//time limit
*  Return: ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-29
*************************************************************************/
ERRNO Client::recv_in_time(const void* buffer,enum MSG_TYPE &type,struct timeval tv){
    fd_set readfd;
    FD_ZERO(&readfd);
    FD_SET(udp_sock, &readfd);
    int rtnval = select(udp_sock+1, &readfd, NULL, NULL, &tv);


    if(rtnval == 0){// timeout
        return TIMEOUT;
    }
    else if(rtnval < 0){// error
        dieWithUserMessager("select failed");
        return SELECT_ERR;
    }
    else{
        uint32_t actual_session_id;
        struct sockaddr_in6 fromAddr;
        size_t buffer_size;
        //receive pdu
        ERRNO rtnval = recv_pdu((char*)buffer,buffer_size,type,fromAddr,actual_session_id);
        if(rtnval != SUCCESS)
        {
            return rtnval;
        }

        if(actual_session_id != session_id){
            dieWithUserMessager("session_id not match");
            return CLIENT_RECV_NOMATCHED_SESSION_ID_ERR;
        }


        // determine if the pdu is sent by itself or not
        //if(!bn->sockAddrEqual(fromAddr,serverAddr)){
        //   dieWithUserMessager("recv fromAddr != targetAddr");
        //   return UNEXCEPTE_SERVER_SOCKADDR;
        //}
        return SUCCESS;

    }

}


/*************************************************************************
*  Function name: recv_in_WAIT_NEGOATION_RESPONSE
*  Description: receive a message while state of client is WAIT_NEGOATION_RESPONSE
*  Parameter: 	const void* buffer	//store data of received message
					enum MSG_TYPE &type	//store type of received message
*  Return: 		ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-29
*************************************************************************/
ERRNO Client::recv_in_WAIT_RESPONSE(const void* buffer,enum MSG_TYPE &type){
    ERRNO rtnval = recv_in_time(buffer, type,response_timer);

    if(rtnval == SUCCESS){
        switch (type) {
            case DISCOVERY_MSG:
                dieWithUserMessager("client cann't recv DISCOVERY_MSG");
                return CLIENT_RECV_UNEXCEPETD_MSG_TYPE_ERR;
                break;
                // received a WAIT_MSG
            case WAIT_MSG:
                //std::cout<<"recv WAIT MSG in WAIT_NEGOATION_RESPONSE"<<std::endl;
                cur_states = WAIT;
                // wait
                return recv(buffer, type);
                break;
                //  received a RESPONSE_MSG
            case RESPONSE_MSG:
                //std::cout<<"recv RESPONSE MSG in WAIT_NEGOATION_RESPONSE"<<std::endl;
                cur_states = INFORMED;
                return SUCCESS;
                break;
            case NEGO_END_MSG:
                //std::cout<<"recv RESPONSE MSG in WAIT_NEGOATION_RESPONSE"<<std::endl;
                cur_states = END;
                break;
            default:
                //std::cout<<"client can't process type = "<<type<<std::endl;
                return MSG_TYPE_ERR;
                break;
        }
    }
    else if(rtnval == TIMEOUT){
        std::cout<<"recv_in_WAIT_RESPONSE timeout"<<std::endl;
    }
    return rtnval;
}

/*************************************************************************
*  Function name: recv_in_WAIT
*  Description: receive a message while state of client is WAIT
*  Parameter: 	const void* buffer	//store data of received message
					enum MSG_TYPE &type	//store type of received message
*  Return: 		ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-29
*************************************************************************/
ERRNO Client::recv_in_WAIT(const void*buffer, enum MSG_TYPE &type){
    ERRNO rtnval = recv_in_time(buffer, type,wait_timer);
    // receive a pdu
    if(rtnval == SUCCESS){
        switch (type) {
            case REQUEST_MSG:
                //dieWithUserMessager("client cann't recv REQUEST_MSG");
                return CLIENT_RECV_UNEXCEPETD_MSG_TYPE_ERR;
                break;
                // received a WAIT_MSG
            case WAIT_MSG:
                //std::cout<<"recv WAIT MSG in WAIT"<<std::endl;
                rtnval = recv_in_WAIT(buffer, type);
                break;
                // received a RESPONSE_MSG
            case RESPONSE_MSG:
                //std::cout<<"recv RESPONSE MSG in WAIT"<<std::endl;
                cur_states = INFORMED;
                break;
            case NEGO_END_MSG:
                //std::cout<<"recv END MSG in WAIT"<<std::endl;
                cur_states = END;
                break;
            default:
                //std::cout<<"client can't process type = "<<type<<std::endl;
                return MSG_TYPE_ERR;
                break;
        }
    }
    else if(rtnval == TIMEOUT){
        std::cout<<"recv_in_WAIT timeout"<<std::endl;
    }
    return rtnval;
}


