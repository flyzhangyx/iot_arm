//
// Created by 18208 on 2021/4/14.
//
#include "GetRsa.h"
extern int LocalSocket;
int isRsaKeyGet = 0;
int isPinGet = 0;
int GetRsaKey(){
    IotPacketInterface tmp;
    memset(&tmp, 0, sizeof(IotPacketInterface));
    char buf[11];
    tmp.opCode[0] = '0';
    tmp.opCode[1] = '1';
    memcpy(buf, &tmp, 11);
    memset(&rsaKey,0,sizeof(RSAKey));
    isRsaKeyGet = 0;
    return send(LocalSocket,buf,11,0);
}

int GetPin(){
    if(!isRsaKeyGet)
        return -1;
    IotPacketInterface tmp;
    memset(&tmp, 0, sizeof(IotPacketInterface));
    char buf[11];
    tmp.opCode[0] = '0';
    tmp.opCode[1] = '2';
    memcpy(buf, &tmp, 11);
    isPinGet = 0;
    return send(LocalSocket,buf,11,0);
}