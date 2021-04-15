//
// Created by 18208 on 2021/4/14.
//



#include "SceCmd.h"

int ErrCnt = 0;

/***
 * Exe Recv Msg Quickly
 * No Waiting
 * @return
 */
void *RecvMsg() {
    IotPacketInterface iotRecStruct;
    while (whileThreadFlag) {
        memset(&iotRecStruct, 0, sizeof(IotPacketInterface));
        if (recv(LocalSocket, &iotRecStruct, sizeof(IotPacketInterface), 0) == 0) {
            printf("Recv Err\n");
            ErrCnt++;
            if (ErrCnt > 10) {
                GetRsaKey();
            }
            ErrCnt = 0;
            sleep(1);
            continue;
        }
        if (iotRecStruct.opCode[0] == 'H') {
            printf("%s", iotRecStruct);
            continue;
        }
        switch (atoi(iotRecStruct.opCode)) {
            case 1: {
                int OutStrSize = 0;
                char **outStr = StrSplit(iotRecStruct.payLoad, 200, &OutStrSize, '_');
                if (OutStrSize != 3) {
                    releaseStr(outStr, OutStrSize);
                    break;
                }
                rsaKey.publicKey = atoi(outStr[0]);
                rsaKey.commonKey = atoi(outStr[1]);
                rsaKey.encryptBlockBytes = atoi(outStr[2]);
                releaseStr(outStr, OutStrSize);
                isRsaKeyGet = 1;
                GetPin();//Next

            }
                break;
            case 2: {
                int OutStrSize = 0;
                char **outStr = StrSplit(iotRecStruct.payLoad, 200, &OutStrSize, '_');
                if (OutStrSize != 1) {
                    releaseStr(outStr, OutStrSize);
                    break;
                }
                //MD5check???LOST//
                int encodedCrypto[100] = {0};
                memcpy(encodedCrypto, outStr[0], sizeof(int) * 6 * rsaKey.encryptBlockBytes);
                decodeMessage(6, rsaKey.encryptBlockBytes, encodedCrypto, pin, rsaKey.publicKey, rsaKey.commonKey);
                pin[6] = 0;
                releaseStr(outStr, OutStrSize);
                isPinGet = 1;
                RegSign("1234567", "1234567");//Next
            }
                break;
            case 3:
                isRegSign = 1;
                printf("SIGN SUCCESS\n");
                break;
            default:
                if (AddMsg2List(iotRecStruct) != 0) {
                    printf("Add Msg Err\n");
                }
                break;
        }
    }
}

void CreateRecvThread() {
    pthread_t t;
    pthread_create(&t, 0, RecvMsg, NULL);
}