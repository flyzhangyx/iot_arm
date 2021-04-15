//
// Created by 18208 on 2021/4/14.
//

#ifndef UNTITLED_GETRSA_H
#define UNTITLED_GETRSA_H
#include "RSAUtils.h"
#include <sys/socket.h>
#include "RecvFromServer.h"
RSAKey rsaKey;
char pin[7];
int GetRsaKey();
int GetPin();
#endif //UNTITLED_GETRSA_H
