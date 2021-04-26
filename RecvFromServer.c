//
// Created by 18208 on 2021/4/14.
//



#include "SceCmd.h"

extern char ID[10];
extern char PWD[10];
int ErrCnt = 0;
char HBATime[20];
char DateFromServer[8];
void CharacterReplace(char NeedRepSrc,char des,char *SrcStr)
{
    while (*(SrcStr++)!=0)
    {
        if(*(SrcStr-1)==NeedRepSrc)
        {
            *(SrcStr-1) = des;
        }
    }
}
/***
 * Exe Recv Msg Quickly
 * No Waiting
 * @return
 */
void *RecvMsg() {
    IotPacketInterface iotRecStruct;
    while (whileThreadFlag) {
        memset(&iotRecStruct, 0, sizeof(IotPacketInterface));
        if (recv(LocalSocket, &iotRecStruct, sizeof(IotPacketInterface), 0) <= 0) {
            printf("Recv Err\n");
            ErrCnt++;
            if (ErrCnt > 10) {
                close(LocalSocket);
                Connect2Server();
                GetRsaKey();
            }
            ErrCnt = 0;
            sleep(1);
            continue;
        }
        if (iotRecStruct.opCode[0] == 'H') {
            printf("%s", &iotRecStruct);
            Stringcut(&iotRecStruct, 3, 21, HBATime);
            continue;
        }
        switch (atoi(iotRecStruct.opCode)) {
            case 1: {
                int OutStrSize = 0;
                char **outStr = StrSplit(iotRecStruct.payLoad, strlen(iotRecStruct.payLoad), &OutStrSize, '_');
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
                RegSign(ID, PWD);//Next
            }
                break;
            case 3:
                isRegSign = 1;
                printf("SIGN SUCCESS\n");
                break;
            case 7:
                if (AddCondMsg2List(iotRecStruct)) {
                    printf("Add CondMsg Err\n");
                }
                break;
            case 5: {
                Decrypt(iotRecStruct.payLoad, strlen(iotRecStruct.payLoad), pin, iotRecStruct.payLoad);
                int OutStrSize = 0;
                char **outStr = StrSplit(iotRecStruct.payLoad, strlen(iotRecStruct.payLoad), &OutStrSize, '_');
                if (OutStrSize != 5) {
                    releaseStr(outStr, OutStrSize);
                    break; ///
                }
                switch (atoi(outStr[0])) {
                    case 98:
                    checkSceTriggerEvent(outStr[2]);
                        break;
                    default:
                        break;
                }
                releaseStr(outStr, OutStrSize);
            }
                break;
            case 0:
                Decrypt(iotRecStruct.payLoad,strlen(iotRecStruct.payLoad),pin,iotRecStruct.payLoad);
                int OutStrSize = 0;
                char **outStr = StrSplit(iotRecStruct.payLoad, strlen(iotRecStruct.payLoad), &OutStrSize, '_');
                if (OutStrSize != 2) {
                    releaseStr(outStr, OutStrSize);
                    break;
                }
                switch (atoi(outStr[0])) {
                    case 98:
                        //CharacterReplace('+','-',outStr[1]);
                        checkSceTriggerEvent(outStr[1]);
                        break;
                    case 0:
                        break;
                    default:
                        break;
                }
                releaseStr(outStr, OutStrSize);
                break;
            case 6:
                if (AddMsg2List(iotRecStruct) != 0) {
                    printf("Add Msg Err\n");
                }
                break;
            default:
                printf("Rec:%s\n",&iotRecStruct);
                break;
        }
    }
}

void CreateRecvThread() {
    pthread_t t;
    pthread_create(&t, 0, RecvMsg, NULL);
}