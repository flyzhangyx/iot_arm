//
// Created by 18208 on 2021/4/14.
//

#include "SceCmd.h"


pthread_mutex_t MSG_LIST_MUTEX;

/*SCENE CMD*/
int AddMsg2List(IotPacketInterface recvStruct) {
    pthread_mutex_lock(&MSG_LIST_MUTEX);
    MSG_FROM_IOT_T NewRecvMsg = (MSG_FROM_IOT_T) malloc(sizeof(MSG_FROM_IOT));
    if (NewRecvMsg == NULL)
        return -1;
    memset(NewRecvMsg,0,sizeof(MSG_FROM_IOT));
    Decrypt(recvStruct.payLoad, strlen(recvStruct.payLoad), pin,recvStruct.payLoad);
    memcpy(&(NewRecvMsg->RecvIotPacket) , &recvStruct ,sizeof(MSG_FROM_IOT));
    NewRecvMsg->next = NULL;
    if(MSG_HEAD.first == NULL)
        MSG_HEAD.first = NewRecvMsg;
    else{
        NewRecvMsg->next=MSG_HEAD.first;
        MSG_HEAD.first = NewRecvMsg;
    }
    MSG_HEAD.len++;
    pthread_mutex_unlock(&MSG_LIST_MUTEX);
    return 0;
}

void *ExeMsg() {
    while (whileThreadFlag) {
        while (MSG_HEAD.first == NULL);
        if (pthread_mutex_trylock(&MSG_LIST_MUTEX) != 0) {
            sleep(1);
            continue;
        }
        MSG_FROM_IOT_T tmp = MSG_HEAD.first;
        MSG_FROM_IOT_T tmp1 = MSG_HEAD.first;
        while(tmp->next!=NULL){
            tmp1 = tmp;
            tmp = tmp->next;
        }
        if (tmp != NULL) {
            switch (atoi(tmp->RecvIotPacket.opCode)) {
                case 5: {//READ CMD FROM SQL
                    int OutStrSize = 0;
                    char **outStr = StrSplit(tmp->RecvIotPacket.payLoad, 200, &OutStrSize, '_');
                    if (OutStrSize != 5) {
                        releaseStr(outStr, OutStrSize);
                        break; ///
                    }
                    switch (atoi(outStr[0])) {
                        case 0: {
                            printf("0:%s\n", outStr[1]);
                        }
                            break;
                        case 1:
                            printf("1:%s\n", outStr[2]);
                            break;
                        default:
                            printf("%s\n",tmp->RecvIotPacket.opCode);
                            break;
                    }
                    releaseStr(outStr, OutStrSize);
                }
                    break;
                case 6:
                    printf("Err Code :%d\n",add2SceList(tmp->RecvIotPacket));
                    break;
                case 2:
                    break;
                default:
                    break;
            }
            if(tmp1 == MSG_HEAD.first)
                MSG_HEAD.first = NULL;
            else
                tmp1->next = NULL;
            free(tmp);
            MSG_HEAD.len--;
        }
        pthread_mutex_unlock(&MSG_LIST_MUTEX);
    }
}

void CreateExeMsgThread() {
    pthread_t t;
    pthread_create(&t, NULL, ExeMsg, NULL);
}