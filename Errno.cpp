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
* File : [Errno.cpp]
* Description : Implementation of error information output function.
* Remark : 
* Modification record:
* Created by leeying in Beijing University of Posts and Telecommunications, in Oct 2013.
* Revised by Cheng Pang and Kangning Xu in Beijing University of Posts and Telecommunications, in May 2015.
* Copyright (c) 2015 Huawei Technologies Co., Ltd. All rights reserved.
************************************************************************
*/

#include "stdio.h"
#include "Errno.h"


/*************************************************************************
*  Function name: dieWithUserMessager
*  Description: output information while error orrurs
*  Parameter: const char* msg	//pointer to the first char of output information
*  Return: void
*  Remark:
*  Modification record:
*  Lastly modified by Kangning Xu on 15-04-29
*************************************************************************/
void dieWithUserMessager(const char* msg){
    fputs(msg, stderr);
    fputc('\n', stderr);
}
