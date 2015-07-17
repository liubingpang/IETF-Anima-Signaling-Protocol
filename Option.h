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
* File:[Option.h]
* Description:Definition of class Option and Obejective_Option.Class Option and Class Obejective_Option is to describe GDNP options.
* Remark:
* Modification record:
* Created by Cheng Pang in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/
#ifndef OPTION_H
#define OPTION_H

#include <string>
#include <stdint.h>



enum option_type
{
	Divert,
	Accept,
	Decline,
	Waiting_time,
	Locator,
	Discovery,
	Negotiation,
	Synchronization
};


class Option
{
protected:
	option_type type;
	uint16_t len;
	uint8_t * value;
public:
	Option(option_type type, uint16_t len, uint8_t * value);
	virtual ~Option(){}
	uint16_t * to_bits();
	static Option parse_bits(uint16_t * bits);
	option_type get_type(){return type;}
	uint16_t get_len(){return len;}
	uint8_t * get_value(){if(value == NULL) return (uint8_t *)""; else return value;}
	const static int len_except_value = 4;
};

class Objective_Option:public Option
{
private:
	uint8_t loop_count;
	uint8_t flag;
	const static int flag_bits_len = 8;
public:
	Objective_Option(option_type type, uint16_t len, uint8_t * value, uint8_t loop_count, uint8_t flag);
	uint16_t * to_bits();
	static Objective_Option parse_bits(uint16_t * bits);
	uint8_t  get_loop_count(){return loop_count;}
	uint8_t  get_flag(){return flag;}
	const static int len_except_value = 6;
	void set_loop_count(uint8_t i){loop_count = i;}
	void set_flag(uint8_t i){flag = i;}
};

#endif
