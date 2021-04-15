//
// Created by 18208 on 2021/4/14.
//

#include "RegSign.h"
int RegSign(char* Id ,char* pwd){
    if(isRegSign)
        return 0;
    IotPacketInterface tmp;
    memset(&tmp, 0, sizeof(IotPacketInterface));
    char buf[sizeof(IotPacketInterface)];
    tmp.opCode[0] = '0';
    tmp.opCode[1] = '3';
    int encodedCrypto[50] = {0};
    char encodedCryptoByte[160];
    memset(encodedCryptoByte, 0, 160);
    encodeMessage(strlen(Id) * rsaKey.encryptBlockBytes, rsaKey.encryptBlockBytes, Id, encodedCrypto, rsaKey.publicKey, rsaKey.commonKey);
    memcpy(encodedCryptoByte, encodedCrypto, strlen(Id) * rsaKey.encryptBlockBytes * 4);
    sprintf(tmp.payLoad, "%s_", pwd);
    Stringcut(encodedCryptoByte, 0, strlen(Id) * rsaKey.encryptBlockBytes * 4 - 1, tmp.payLoad + 8);
    tmp.payLoad[strlen(Id) * rsaKey.encryptBlockBytes * 4 + 8] = '_';
    memcpy(buf, &tmp, sizeof(IotPacketInterface));
    return send(LocalSocket,buf,205,0);
}