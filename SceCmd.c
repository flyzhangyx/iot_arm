//
// Created by 18208 on 2021/4/14.
//

#include "SceCmd.h"

typedef
struct ConditionMsgRecv{
    IotPacketInterface recv;
    struct ConditionMsgRecv* next;
}*COND_RECV_T;
struct ConditionMsgRecv CondRecvHead;
extern
struct SceCmdHead_S {
    int TriggerNum;
    SCE_TRIGGER_T next;
} SceCmdHead;

extern
struct SceCmd_Condition_Head_S {
    int ConditionNum;
    COND_T first;
} SceCmd_Condition_Head;

pthread_mutex_t MSG_LIST_MUTEX;
pthread_mutex_t COND_RECV_LIST_MUTEX;


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


int GetIotData(int id_list[],int len){
    if(len>50)
    {
        return 0;
    }
    IotPacketInterface tmp;
    memset(&tmp, 0, sizeof(IotPacketInterface));
    char buf[205];
    tmp.opCode[0] = '0';
    tmp.opCode[1] = '7';
    char id[10];
    while (len-->0)
    {
        memset(id,0,10);
        sprintf(id,"%d_",id_list[len]);
        strcat(tmp.payLoad,id);
    }
    memcpy(buf, &tmp, 205);
    return send(LocalSocket,buf,205,0);
}


int AddCondMsg2List(IotPacketInterface CondRecvStruct)
{
    Decrypt(CondRecvStruct.payLoad, 200, pin,CondRecvStruct.payLoad);
    int OutStrSize = 0;
    char **outStr = StrSplit(CondRecvStruct.payLoad, strlen(CondRecvStruct.payLoad), &OutStrSize, '_');
    if (OutStrSize != 5) {
        printf("ADD COND MSG ERR \n");
        releaseStr(outStr,OutStrSize);
        return -1;
    }
    releaseStr(outStr,OutStrSize);
    pthread_mutex_lock(&COND_RECV_LIST_MUTEX);
    COND_RECV_T NewConRec = (COND_RECV_T)malloc(sizeof(struct ConditionMsgRecv));
    if(NewConRec == NULL)
        return -2;
    memset(NewConRec,0,sizeof(IotPacketInterface));
    memcpy(&(NewConRec->recv),&CondRecvStruct,sizeof(IotPacketInterface));
    COND_RECV_T tmp = CondRecvHead.next;
    if(tmp==NULL){
        CondRecvHead.next = NewConRec;
    } else {
        while (tmp->next != NULL) {
            tmp = tmp->next;
        }
        tmp->next = NewConRec;
    }
    CondRecvHead.recv.opCode[0]++;
    pthread_mutex_unlock(&COND_RECV_LIST_MUTEX);
    return 0;
}

void *UpdateConditionStatus() {
    int id_list[50];
    int len;
    memset(&CondRecvHead,0,sizeof(struct ConditionMsgRecv));
    pthread_mutex_init(&COND_RECV_LIST_MUTEX,NULL);
    while (whileThreadFlag){
        len = 0;
        memset(id_list,0,50*sizeof(int));
        while (SceCmd_Condition_Head.first==NULL)
            sleep(1);
        COND_T tmp = SceCmd_Condition_Head.first;
        while(tmp!=NULL){
            id_list[len++] = tmp->ConditionIotId;
            tmp = tmp->CondListNextMoreCond;
        }
        GetIotData(id_list,len);
        sleep(5);
        if (pthread_mutex_trylock(&COND_RECV_LIST_MUTEX)!=0){
            printf("BUSY UPDATING COND LIST\n");
            continue;
        }
        COND_RECV_T tmp_cond_recv = CondRecvHead.next;
        while (tmp_cond_recv!=NULL){
            int OutStrSize = 0;
            char **outStr = StrSplit(tmp_cond_recv->recv.payLoad, 200, &OutStrSize, '_');
            COND_T tmp_cond = SceCmd_Condition_Head.first;
            while (tmp_cond!=NULL){
                if(atoi(outStr[0])==tmp_cond->ConditionIotId&&atoi(outStr[1])==tmp_cond->ConditionDevClass){
                    tmp_cond->isConditionSatisfied = 0;
                    if(atoi(outStr[1]) != 0){
                        switch (tmp_cond->ConditionJudgment) {
                            case -1:
                                if(atoi(tmp_cond->ConditionEvent)>atoi(outStr[3]))
                                    tmp_cond->isConditionSatisfied = 1;
                                break;
                            case 0:
                                if(atoi(tmp_cond->ConditionEvent)==atoi(outStr[3]))
                                    tmp_cond->isConditionSatisfied = 1;
                                break;
                            case 1:
                                if(atoi(tmp_cond->ConditionEvent)<atoi(outStr[3]))
                                    tmp_cond->isConditionSatisfied = 1;
                                break;
                            default:
                                break;
                        }
                    } else{
                        if(atoi(tmp_cond->ConditionEvent)==atoi(outStr[2]))
                            tmp_cond->isConditionSatisfied = 1;
                    }
                }
                tmp_cond = tmp_cond->CondListNextMoreCond;
            }
            COND_RECV_T t = tmp_cond_recv->next;
            if(CondRecvHead.next == tmp_cond_recv)
            {
                CondRecvHead.next = t;
            }
            free(tmp_cond_recv);
            tmp_cond_recv = t;
            CondRecvHead.recv.opCode[0]--;
            releaseStr(outStr, OutStrSize);
        }
        pthread_mutex_unlock(&COND_RECV_LIST_MUTEX);
    }
}

void CreateExeMsgThread() {
    pthread_t t,t1;
    pthread_create(&t, NULL, ExeMsg, NULL);
    pthread_create(&t1,NULL,UpdateConditionStatus,NULL);
}