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
* Description:Definition of class Option's and class Obejective_Option's member function
* Remark:
* Modification record:
* Created by Cheng Pang in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/

#include "Option.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>

/*************************************************************************
*  Function name: Option::Option
*  Description: constructor of Option
*  Parameter: type   option_type
*  	             len     uint16_t  in octets
*  	             value  uint8_t
*  Return: none
*  Remark:
*  Lastly modified by Cheng Pang on 15-6-3
*************************************************************************/
Option::Option(option_type type, uint16_t len, uint8_t * value)
{
	this->type = type;
	this->len = len;
	this->value = value;
}

/*************************************************************************
*  Function name: Obejective_Option::Obejective_Option
*  Description: constructor of Obejective_Option
*  Parameter: type   option_type
*  	             len     uint16_t  in octets
*  	             value  uint8_t
*  	             loop_count  uint8_t
*  	             flag  uint8_t
*  Return: none
*  Remark:
*  Lastly modified by Cheng Pang on 15-6-3
*************************************************************************/
Objective_Option::Objective_Option(option_type type, uint16_t len, uint8_t * value, uint8_t loop_count, uint8_t flag)
:Option(type, len, value)
{
	this->flag = flag;
	this->loop_count = loop_count;
}

/*************************************************************************
*  Function name: Option::to_bits
*  Description: make Option into bits of GDNP option format
*  Parameter: none
*  Return: uint16_t*
*  Remark:
*  Lastly modified by Cheng Pang on 15-6-3
*************************************************************************/
uint16_t* Option::to_bits()
{
	uint16_t * bits = (uint16_t*)malloc(sizeof(uint8_t) * len + len_except_value);
    memset(bits,0,sizeof(uint8_t) * len + len_except_value);
    uint16_t type_num = type;

    *bits |= type_num;
    *(bits+1) |= len;
    if(value != NULL)
    	memcpy(bits+2,value,len);

    return bits;
}

/*************************************************************************
*  Function name: Obejective_Option::to_bits
*  Description: make Obejective_Option into bits of GDNP obejective_option format
*  Parameter: none
*  Return: uint16_t*
*  Remark:
*  Lastly modified by Cheng Pang on 15-6-3
*************************************************************************/
uint16_t* Objective_Option::to_bits()
{
	uint16_t * bits = (uint16_t*)malloc(sizeof(uint8_t) * len + len_except_value);
    memset(bits,0,sizeof(uint8_t) * len + len_except_value);
    uint16_t type_num = type;

    *bits |= type_num;
    *(bits+1) |= len;
    *(bits+2) |= ((uint16_t)loop_count <<  flag_bits_len) | flag;
    if(value != NULL)
    	memcpy(bits+3,value,len);

    return bits;
}

/*************************************************************************
*  Function name: Option::parse_bits
*  Description: parse bits of GDNP option format
*  Parameter: bits uint16_t
*  				 bits_size  size_t   in octets
*  Return: Option
*  Remark:
*  Lastly modified by Cheng Pang on 15-6-3
*************************************************************************/
Option Option::parse_bits(uint16_t * bits)
{
	uint16_t type_num = *bits;
	option_type type;
	switch(type_num)
	{
		case 0:
			type = Divert;
			break;
		case 1:
			type = Accept;
			break;
		case 2:
			type = Decline;
			break;
		case 3:
			type =Waiting_time;
			break;
		case 4:
			type = Locator;
			break;
		case 5:
			type = Discovery;
			break;
		case 6:
			type = Negotiation;
			break;
		case 7:
			type = Synchronization;
			break;
	}
	uint16_t len;
	len = *(bits+1) ;

	char * data;
	if(len != 0)
	{
		data = (char *)malloc(sizeof(len));
		strncpy(data, (char*)(bits+2),len);
	}
	else
		data =NULL;

	return Option (type, len, (uint8_t*)data);;
}

/*************************************************************************
*  Function name: Obejective_Option::parse_bits
*  Description: parse bits of GDNP obejective_option format
*  Parameter: bits uint16_t
*  				 bits_size  size_t   in octets
*  Return: Obejective_Option
*  Remark:
*  Lastly modified by Cheng Pang on 15-6-3
*************************************************************************/
Objective_Option Objective_Option::parse_bits(uint16_t * bits)
{
	uint16_t type_num = *bits;
	option_type type;
	switch(type_num)
	{
		case 0:
			type = Divert;
			break;
		case 1:
			type = Accept;
			break;
		case 2:
			type = Decline;
			break;
		case 3:
			type =Waiting_time;
			break;
		case 4:
			type = Locator;
			break;
		case 5:
			type = Discovery;
			break;
		case 6:
			type = Negotiation;
			break;
		case 7:
			type = Synchronization;
			break;
	}
	uint16_t len = *(bits+1) ;
	uint8_t loop_count = *(bits+2) >> flag_bits_len;
	uint8_t flag = *(bits+2);

	char * data;
	if(len != 0)
	{
		data = (char *)malloc(sizeof(len));
		strncpy(data, (char*)(bits+3),len);
	}
	else
		data =NULL;

	return Objective_Option (type, len, (uint8_t*)data, loop_count, flag);
}
