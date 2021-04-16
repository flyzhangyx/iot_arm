//
// Created by 18208 on 2021/4/14.
//

#ifndef UNTITLED_SCECMD_H
#define UNTITLED_SCECMD_H
#include "RecvFromServer.h"
#include <malloc.h>
#include <stdlib.h>
#include "KaisaEncrypt.h"
#include "SceCmdConfig.h"

extern int isRsaKeyGet;
extern int whileThreadFlag;
typedef
struct MSG_FROM_IOT_DEV
{
    IotPacketInterface RecvIotPacket;
    char time[10];
    struct MSG_FROM_IOT_DEV * next;
}MSG_FROM_IOT,
*MSG_FROM_IOT_T;
struct MSG_FROM_IOT_HEAD
{
    MSG_FROM_IOT_T first;
    int len;
}MSG_HEAD;
int AddCondMsg2List(IotPacketInterface CondRecvStruct);
int AddMsg2List(IotPacketInterface recvStruct);
void CreateExeMsgThread();
#endif //UNTITLED_SCECMD_H
