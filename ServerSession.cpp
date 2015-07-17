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
* File:[ServerSession.cpp]
* Description:Definition of class ServerSession's member function
* Remark:
* Modification record:
* Created by leeying in Beijing University of Posts and Telecommunications, in Oct 2013.
* Revised by Cheng Pang and Kangning Xu in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/

#include "ServerSession.h"
#include "Errno.h"
#include <unistd.h>
//#include "UniqueSessionId.h"
#include "Server.h"
#include "Option.h"
#include <pthread.h>
#include <string.h>

/*************************************************************************
*  Function name: queue_push
*  Description: push new message content into queue
*  Parameter: c  content
*  Return: void
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-29
*************************************************************************/
void ServerSession::queue_push(content c)
{
    // lock
    pthread_mutex_lock(&queuelock);
    this->q->push(c);
    // unlock
    pthread_mutex_unlock(&queuelock);
}

/*************************************************************************
*  Function name: queue_pop
*  Description: pop from queue
*  Parameter:
*  Return: void
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-29
*************************************************************************/
content ServerSession::queue_pop()
{
    pthread_mutex_lock(&queuelock);
    content c = q->front();
    q->pop();
    pthread_mutex_unlock(&queuelock);
    return c;
}

/*************************************************************************
*  Function name: queue_empty
*  Description: determine whether queue is empty
*  Parameter: none
*  Return: bool
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-29
*************************************************************************/
bool ServerSession::queue_empty()
{
    pthread_mutex_lock(&queuelock);
    bool result = q->empty();
    pthread_mutex_unlock(&queuelock);
    return result;
}

/*************************************************************************
*  Function name: get_cur_state
*  Description: get session state
*  Parameter: none
*  Return: server_states
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-29
*************************************************************************/
 enum server_states ServerSession::get_cur_state()
 {
        pthread_mutex_lock(&statelock);
        server_states temp =  cur_state;
        pthread_mutex_unlock(&statelock);
        return temp;
}

/*************************************************************************
*  Function name: set_cur_state
*  Description: ser session state
*  Parameter: state   session state to set
*  Return: void
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-29
*************************************************************************/
void ServerSession::set_cur_state(enum server_states state)
{
    pthread_mutex_lock(&statelock);
    this->cur_state = state;
    pthread_mutex_unlock(&statelock);
}


/*************************************************************************
*  Function name: ServerSession
*  Description: constructor of ServerSession
*  Parameter: clientAddr   session state to set
*  	          nsocket      socket id
*  	          session_id
*  	          c            content
*  Return: none
*  Remark:
*  Lastly modified by Cheng Pang on 15-5-27
*************************************************************************/
ServerSession::ServerSession(int tcp_sock,uint32_t session_id,content c)
//:BaseNegotiator(nsocket)
{
	pthread_mutex_init(&statelock,NULL);
	pthread_mutex_init(&queuelock,NULL);
	this->tcp_sock = tcp_sock;
    this->session_id = session_id;
    this->cur_state = IDLE;
    idle_check = 0;
    // push into queue
    this->q = new std::queue<content>();
    this->queue_push(c);

}


/*************************************************************************
*  Function name: ~ServerSession
*  Description: destructor of ServerSession
*  Parameter: none
*  Return: none
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-29
*************************************************************************/
ServerSession::~ServerSession(){
    pthread_mutex_lock(&queuelock);
    while (q != NULL && !q->empty())
    {
        q->pop();
    }
    if(q != NULL)
    {
        delete q;
        q = NULL;
    }
    pthread_mutex_unlock(&queuelock);
    //std::cout<<"running SS's destruct function"<<std::endl;
}

/*************************************************************************
*  Function name: run_help
*  Description: static run function
*  Parameter: arg   point to session_run_parms
*  Return: void*
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-29
*************************************************************************/
void* ServerSession::run_help(void *arg){
    session_run_parms* parm_p = (session_run_parms*)arg;
    parm_p->ss->run(parm_p->sm);
    return  (void*)0;
}

/*************************************************************************
*  Function name: run
*  Description:  handle session for each thread
*  Parameter: sm  point to ServerMaster
*  Return: void
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-5-27
*************************************************************************/
void ServerSession::run(ServerMaster* sm){
    content last_content;
    while (get_cur_state() != SESSION_END)
    {
        if(this->queue_empty()){
            sleep(1);
            continue;
        }
        idle_check = 1;
        // pop from queue, get its first element
        content c = this->queue_pop();
        //std::cout<<pthread_self()<<"receive package :data="<<c.data<<", type="<<c.type<<"and cur_state="<<cur_state<<std::endl;

		// filtering out duplicate REQUEST
        if(c == last_content)
        {
            //std::cout<<pthread_self()<<"duplicate REQUEST package"<<std::endl;
            continue;
        }
        else{
            last_content = c;
        }

        //update state
        int rtnval = set_state(c.type);
        if(rtnval == SUCCESS)
        {
			if(get_cur_state() == PROCESSING)
			{
                //std::cout<<pthread_self()<<"launch a thread to send wait msg ...cur_state="<<cur_state<<std::endl;
                pthread_t tid;
                pthread_create(&tid, NULL, wait_thread_handler, this);
            }
			Objective_Option recv_option = Objective_Option::parse_bits((uint16_t *)c.data);
			std::cout << "thread " <<pthread_self() << std::endl << "msg type "<< c.type << std::endl
						<< "option type " << recv_option.get_type() << std::endl
						<< "value " << (char*)recv_option.get_value() << std::endl
						<< "len " << recv_option.get_len() << std::endl
						<< "loop_count " << (int)recv_option.get_loop_count() << std::endl << std::endl;;
            if(get_cur_state() != SESSION_END)
            {
            	if(c.type != NEGO_MSG && c.type != REQUEST_MSG)
            	{
//            		*option = Option::parse_bits((uint16_t *)c.data, strlen(c.data));
					dieWithUserMessager("It' should be a objective option");
					std::cout << "msg type "<< c.type  << std::endl;
					continue;
				}
            	recv_option.set_loop_count(recv_option.get_loop_count() - 1);
            	//upper PROCESSING
            	char *upper_data =  (char *)sm->asa_negotiate_result(recv_option.get_value());
            	//upper PROCESSING end
            	 set_cur_state(IDLE);
                // call upper
                // upper action
                enum MSG_TYPE type;
                if(recv_option.get_type() == Synchronization)//Synchronization
				{
                	type = NEGO_END_MSG;
                	uint16_t value_len = strlen(upper_data);
                	Objective_Option send_option(recv_option.get_type(), value_len, (uint8_t*)upper_data, recv_option.get_loop_count() , recv_option.get_flag());
					std::cout << "Recived a request with Synchronization Option ! " << std::endl;
					if((rtnval = send(send_option.to_bits(), send_option.get_len()  + Objective_Option::len_except_value, type)) != SUCCESS)
					{
						std::cout<<pthread_self()<<"send  failed:"<<rtnval<<std::endl;
					}
					set_cur_state(SESSION_END);
				}
                else if ( !sm->asa_geq_fn (upper_data, recv_option.get_value()) && (recv_option.get_loop_count() != 0))//not same objective and loop_count != 0
                {
                	uint16_t value_len = strlen(upper_data);
                	Objective_Option send_option(recv_option.get_type(), value_len, (uint8_t*)upper_data, recv_option.get_loop_count() , recv_option.get_flag());
                	type = NEGO_MSG;
                    if((rtnval = send(send_option.to_bits(), send_option.get_len() + Objective_Option::len_except_value, type)) != SUCCESS)
                    {
						std::cout<<pthread_self()<<" send  failed:"<<rtnval<<std::endl;
					}
                }
                else //same objective or loop_count == 0
                {

                    type = NEGO_END_MSG;
                    Option *send_option;
                    if(recv_option.get_loop_count() == 0)
                    {
						send_option = new Option(Decline, 0, NULL);
						std::cout << "Over loop_count ,decline!" << std::endl;
                    }
                    else
                    {
                    	send_option = new Option(Accept, 0, NULL);
                    	std::cout << "Accept!!" << std::endl;
                    }
					if((rtnval = send(send_option->to_bits(), send_option->get_len()  + Option::len_except_value, type)) != SUCCESS)
					{
						std::cout<<pthread_self()<<"send  failed:"<<rtnval<<std::endl;
					}

					set_cur_state(SESSION_END);
                }
            }
            else
            {
            	Option recv_option = Option::parse_bits((uint16_t *)c.data);
            	if(recv_option.get_type() == Accept)
					std::cout << "The negotiation is accepted" << std::endl;
				else if (recv_option.get_type() == Decline)
					std::cout << "The negotiation is declined" << std::endl;
            }
        }
    }
    close_tcp();
    // clean up session id for ServerMaster
    std::cout<<"Negotiation end！"<<std::endl;
    sm->clear_when_session_end(session_id, tcp_sock);
}

/*************************************************************************
*  Function name: wait_thread_handler
*  Description:  didn't get response from upper in PROCESSING_TIMEOUT_SECOND,send WAIT message
*  Parameter: arg    point to ServerSession
*  Return: void*
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-29
*************************************************************************/
void* ServerSession::wait_thread_handler(void* arg)
{
	ServerSession* ss = (ServerSession*)arg;
	int total_sleep = 0;
	while(ss->get_cur_state() == PROCESSING)
	{
		sleep(1);
		total_sleep++;
		if(ss->get_cur_state() != PROCESSING)
		{
			//std::cout<<"wait_thread "<<pthread_self()<<":over! cur_state="<<ss->get_cur_state()<<std::endl;
			break;
		}
		else if(ss->get_cur_state() == PROCESSING && total_sleep >=  PROCESSING_TIMEOUT_SECOND){
			//std::cout<<"wait_thread "<<pthread_self()<<":send WAIT MSG"<<std::endl;
			uint32_t time = WAIT_TIMEOUT_SECOND * 1000;
			Option wait_option(Waiting_time, 4, (uint8_t*)&time);
            ss->send(wait_option.to_bits(), wait_option.get_len() + Option::len_except_value, WAIT_MSG);
			total_sleep = 0;
   	    }
    }
    return (void*)0;
}

/*************************************************************************
*  Function name: end_thread_handler
*  Description:  didn't receive new request in END_TIMEOUT_SECOND, update state into SESSION_END
*  Parameter: arg    point to ServerSession
*  Return: ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-29
*************************************************************************/
void* ServerSession::end_thread_handler(void* arg){
    ServerSession* ss = (ServerSession*)arg;
    int total_sleep = 0;
    while (total_sleep < END_TIMEOUT_SECOND)
    {
        sleep(1);
        total_sleep++;
        if(ss->idle_check == 1)
        {
        	ss->idle_check = 0;
        	return 0;
        }
    }

    // have no new request
//    if(ss->idle_check == 0)
    {
		//std::cout<<"end_thread "<<pthread_self()<<":no new package, time-out! "<<std::endl;
        ss->set_cur_state(SESSION_END);
    }
    return (void*)0;
}

/*************************************************************************
*  Function name: send
*  Description: send in different state
*  Parameter: buffer
*  			 	 buffer_size
*  			 	 type
*  Return: ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-4-29
*************************************************************************/
ERRNO ServerSession::send(const void* buffer, size_t buffer_size,enum MSG_TYPE type){
    ERRNO rtnval = SUCCESS;
//    std::cout << cur_state <<" " << type << std::endl;
    switch (cur_state)
    {
        case IDLE:
        	switch (type)
			{
				case NEGO_MSG:
					// send NEGO_MSG MSG
					rtnval = write_pdu(buffer,buffer_size, type,session_id);
					if(rtnval == SUCCESS)
					{
						set_cur_state(IDLE);
						// launch end thread
						pthread_t tid;
						idle_check = 0;
						pthread_create(&tid, NULL, end_thread_handler, this);

					}
					break;
				case NEGO_END_MSG:
					// send ENDING MSG
					rtnval = write_pdu(buffer,buffer_size,type,session_id);
					if(rtnval == SUCCESS)
					{
						set_cur_state(SESSION_END);
					}
					break;
				default:
					rtnval = MSG_TYPE_ERR;
					break;
			}
            break;
        case PROCESSING:
            switch (type)
            {
                case WAIT_MSG:
                    // send WAIT MSG
                    rtnval = write_pdu((char*)buffer,buffer_size,type,session_id);
                    break;
                default:
                    rtnval = MSG_TYPE_ERR;
                    break;
            }
            break;
        case SESSION_END:
            dieWithUserMessager("Server doesn't allowed to send in state SESSION_END");
            rtnval = SERVER_SEND_UNEXCEPTED_MSG_TYPE_ERR;
            break;
        default:
            rtnval = SERVER_UNEXCEPTED_STATE_ERR;
            break;
    }
    return rtnval;
}

/*************************************************************************
*  Function name: set_state
*  Description: set session state by the message type
*  Parameter: type    message type
*  Return: ERRNO
*  Remark:
*  Modification record:
*  Lastly modified by Cheng Pang on 15-5-20
*************************************************************************/
ERRNO ServerSession::set_state(enum MSG_TYPE type){
    ERRNO rtnval;
    switch (cur_state)
    {
        case IDLE:
            switch (type)
            {
                case REQUEST_MSG:
                    // update state into PROCESSING
                    set_cur_state(PROCESSING);
                    rtnval = SUCCESS;
                    break;
                case NEGO_MSG:
                	set_cur_state(PROCESSING);
                	rtnval = SUCCESS;
                	break;
                case NEGO_END_MSG:
					set_cur_state(SESSION_END);
                    rtnval = SUCCESS;
                    break;
                default:
                    rtnval = SERVER_RECV_UNEXCEPTED_MSG_TYPE_ERR;
                    break;
            }
            break;
        case PROCESSING:
            dieWithUserMessager("Server doesn't allowed recv ANY MSG in state PROCESSING");
            rtnval = ERROR;
            break;
        case SESSION_END:
            dieWithUserMessager("Server doesn't allowed recv ANY MSG in state SESSION_END");
            rtnval = ERROR;
            break;
        default:
            rtnval = SERVER_UNEXCEPTED_STATE_ERR;
            break;
    }
    return rtnval;
}


