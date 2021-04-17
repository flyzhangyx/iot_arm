//
// Created by 18208 on 2021/4/14.
//

#ifndef UNTITLED_RSAUTILS_H
#define UNTITLED_RSAUTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct
{
    int publicKey;
    int privateKey;
    int commonKey;
    int encryptBlockBytes;
} RSAKey;
const char * getWeekdayByYearday(int iY, int iM, int iD);
void InitRSA(RSAKey*);
void encodeMessage(int len, int bytes, char* message,int* outCrypto, int exponent, int modulus);
void decodeMessage(int len, int bytes, int* cryptogram,char *outSource, int exponent, int modulus);
unsigned int DJBHash(char* str, unsigned int len);
#endif //UNTITLED_RSAUTILS_H
