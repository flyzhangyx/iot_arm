//
// Created by 18208 on 2021/4/14.
//

#ifndef UNTITLED_RECVFROMSERVER_H
#define UNTITLED_RECVFROMSERVER_H
#include <pthread.h>
#include <sys/socket.h>
#include "RecvFromServer.h"
#include "Connect2Server.h"
#include "StrSplit.h"
#include "GetRsa.h"
#include "RegSign.h"
extern int isRegSign;
extern int isPinGet;
typedef struct
{
    char opCode[3];
    char SeqNum[2];
    char payLoad[200];//Max
} IotPacketInterface;
void CreateRecvThread();
#endif //UNTITLED_RECVFROMSERVER_H
