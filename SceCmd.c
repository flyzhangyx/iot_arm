//
// Created by 18208 on 2021/4/14.
//

#include <stdbool.h>
#include "SceCmd.h"

typedef
struct ConditionMsgRecv {
    IotPacketInterface recv;
    struct ConditionMsgRecv *next;
} *COND_RECV_T;
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
    memset(NewRecvMsg, 0, sizeof(MSG_FROM_IOT));
    Decrypt(recvStruct.payLoad, strlen(recvStruct.payLoad), pin, recvStruct.payLoad);
    memcpy(&(NewRecvMsg->RecvIotPacket), &recvStruct, sizeof(MSG_FROM_IOT));
    NewRecvMsg->next = NULL;
    if (MSG_HEAD.first == NULL)
        MSG_HEAD.first = NewRecvMsg;
    else {
        NewRecvMsg->next = MSG_HEAD.first;
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
        while (tmp->next != NULL) {
            tmp1 = tmp;
            tmp = tmp->next;
        }
        if (tmp != NULL) {
            switch (atoi(tmp->RecvIotPacket.opCode)) {
                case 5: {//READ CMD FROM SQL
                    int OutStrSize = 0;
                    char **outStr = StrSplit(tmp->RecvIotPacket.payLoad, strlen(tmp->RecvIotPacket.payLoad),
                                             &OutStrSize, '_');
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
                            printf("%s\n", tmp->RecvIotPacket.opCode);
                            break;
                    }
                    releaseStr(outStr, OutStrSize);
                }
                    break;
                case 6:
                    printf("Err Code :%d\n", add2SceList(tmp->RecvIotPacket));
                    break;
                case 2:
                    break;
                default:
                    break;
            }
            if (tmp1 == MSG_HEAD.first)
                MSG_HEAD.first = NULL;
            else
                tmp1->next = NULL;
            free(tmp);
            MSG_HEAD.len--;
        }
        pthread_mutex_unlock(&MSG_LIST_MUTEX);
    }
}


int GetIotData(int id_list[], int len) {
    if (len > 50) {
        return 0;
    }
    IotPacketInterface tmp;
    memset(&tmp, 0, sizeof(IotPacketInterface));
    char buf[205];
    tmp.opCode[0] = '0';
    tmp.opCode[1] = '7';
    char id[10];
    while (len-- > 0) {
        memset(id, 0, 10);
        sprintf(id, "%d_", id_list[len]);
        strcat(tmp.payLoad, id);
    }
    memcpy(buf, &tmp, 205);
    return send(LocalSocket, buf, 205, 0);
}


int AddCondMsg2List(IotPacketInterface CondRecvStruct) {
    Decrypt(CondRecvStruct.payLoad, 200, pin, CondRecvStruct.payLoad);
    int OutStrSize = 0;
    char **outStr = StrSplit(CondRecvStruct.payLoad, strlen(CondRecvStruct.payLoad), &OutStrSize, '_');
    if (OutStrSize != 5) {
        printf("ADD COND MSG ERR \n");
        releaseStr(outStr, OutStrSize);
        return -1;
    }
    releaseStr(outStr, OutStrSize);
    pthread_mutex_lock(&COND_RECV_LIST_MUTEX);
    COND_RECV_T NewConRec = (COND_RECV_T) malloc(sizeof(struct ConditionMsgRecv));
    if (NewConRec == NULL)
        return -2;
    printf("1==========%x\n",NewConRec);
    memset(NewConRec, 0, sizeof(IotPacketInterface));
    NewConRec->next = NULL;
    memcpy(&(NewConRec->recv), &CondRecvStruct, sizeof(IotPacketInterface));
    COND_RECV_T tmp = CondRecvHead.next;
    if (tmp == NULL) {
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
    memset(&CondRecvHead, 0, sizeof(struct ConditionMsgRecv));
    CondRecvHead.next = NULL;
    pthread_mutex_init(&COND_RECV_LIST_MUTEX, NULL);
    while (whileThreadFlag) {
        len = 0;
        memset(id_list, 0, 50 * sizeof(int));
        while (SceCmd_Condition_Head.first == NULL)
            sleep(1);
        COND_T tmp = SceCmd_Condition_Head.first;
        while (tmp != NULL) {
            id_list[len++] = tmp->ConditionIotId;
            tmp = tmp->CondListNextMoreCond;
        }
        GetIotData(id_list, len);
        sleep(5);
        if (pthread_mutex_trylock(&COND_RECV_LIST_MUTEX) != 0) {
            printf("BUSY UPDATING COND LIST\n");
            continue;
        }
        COND_RECV_T tmp_cond_recv = CondRecvHead.next;
        while (tmp_cond_recv != NULL) {
            int OutStrSize = 0;
            printf("*****%x_%x\n", &(tmp_cond_recv->recv), tmp_cond_recv);
            char **outStr = StrSplit(tmp_cond_recv->recv.payLoad, strlen(tmp_cond_recv->recv.payLoad), &OutStrSize,
                                     '_');
            if (OutStrSize == 5) {
                COND_T tmp_cond = SceCmd_Condition_Head.first;
                while (tmp_cond != NULL) {
                    if (atoi(outStr[0]) == tmp_cond->ConditionIotId && atoi(outStr[1]) == tmp_cond->ConditionDevClass) {
                        tmp_cond->isConditionSatisfied = 0;
                        if (atoi(outStr[1]) != 0) {
                            switch (tmp_cond->ConditionJudgment) {
                                case -1:
                                    if (atoi(tmp_cond->ConditionEvent) > atoi(outStr[3]))
                                        tmp_cond->isConditionSatisfied = 1;
                                    break;
                                case 0:
                                    if (atoi(tmp_cond->ConditionEvent) == atoi(outStr[3]))
                                        tmp_cond->isConditionSatisfied = 1;
                                    break;
                                case 1:
                                    if (atoi(tmp_cond->ConditionEvent) < atoi(outStr[3]))
                                        tmp_cond->isConditionSatisfied = 1;
                                    break;
                                default:
                                    break;
                            }
                        } else {
                            //printf("%s__________%s\n", tmp_cond->ConditionEvent, outStr[2]);
                            if (atoi(tmp_cond->ConditionEvent) == atoi(outStr[2]))
                                tmp_cond->isConditionSatisfied = 1;
                            else
                                tmp_cond->isConditionSatisfied = 0;
                        }
                    }
                    tmp_cond = tmp_cond->CondListNextMoreCond;
                }
            }
            COND_RECV_T t = tmp_cond_recv->next;
            printf("2==========%x\n",tmp_cond_recv);
            if (CondRecvHead.next == tmp_cond_recv) {
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


int checkSceTriggerEvent(char *TriggerStr) {
    int OutStrSize = 0;
    char **outStr = StrSplit(TriggerStr, strlen(TriggerStr), &OutStrSize, '+');
    if (OutStrSize != 3) {
        releaseStr(outStr, OutStrSize);
        return -1;
    }
    SCE_TRIGGER_T tmp = SceCmdHead.next;
    while (tmp != NULL) {
        char tmpStr[10];
        memset(tmpStr, 0, 10);
        Stringcut(outStr[2], 1, strlen(outStr[2]), tmpStr);
        if (tmp->TriggerIotId == atoi(outStr[0]) && tmp->TriggerDevClass == atoi(outStr[1]) &&
            tmp->TriggerJudgment == (outStr[2][0] == '<' ? -1 : (outStr[2][0] == '>' ? 1 : 0)) &&
            !strcmp(tmp->TriggerEvent, tmpStr)) {
            SCE_CONDITION_T tmp1 = tmp->ConditionListFollowCondition;
            while (tmp1->ConditionListNextMoreConditions != NULL && tmp1->ConditionPointer->isConditionSatisfied == 1) {
                tmp1 = tmp1->ConditionListNextMoreConditions;
            }
            if (tmp1->ConditionListNextMoreConditions == NULL) {
                if (tmp1->EndOfListCMD == NULL)
                    return -2;
                SCE_COMMAND_T tmp2 = tmp1->EndOfListCMD;
                IotPacketInterface tmp;
                memset(&tmp, 0, sizeof(IotPacketInterface));
                char buf[30];
                tmp.opCode[0] = '0';
                tmp.opCode[1] = '8';
                sprintf(tmp.payLoad, "%d_%d_%s_", tmp2->CmdIotId, tmp2->CmdDevClass, tmp2->CmdContent);
                Encrypt(tmp.payLoad, strlen(tmp.payLoad), pin, tmp.payLoad);
                memcpy(buf, &tmp, 30);
                send(LocalSocket, buf, 30, 0);
                printf("Send Ok\n");
            }
        }
        tmp = tmp->TriggerListNextTrigger;
    }
    return 0;
}

void *checkTimeSceCmd() {
    char weekday[8];
    int y, m, d;
    while (whileThreadFlag) {
        sleep(5);
        y = HBATime[0] * 1000 + HBATime[1] * 100 + HBATime[2] * 10 + HBATime[3];
        m = HBATime[5] * 10 + HBATime[6];
        d = HBATime[8] * 10 + HBATime[9];
        memcpy(weekday, getWeekdayByYearday(y, m, d), 7);
        SCE_TRIGGER_T tmp = SceCmdHead.next;
        while (tmp != NULL) {
            if (tmp->TriggerIotId == 0) {
                bool flag = false;
                for (int i = 0; i < 7; i++) {
                    if (weekday[i] + tmp->TriggerDate[i] == 2 * '1') {
                        flag = true;
                        break;
                    } else {
                        flag = false;
                    }
                }
                if (flag == true) {
                    int h = (HBATime[11] - tmp->TriggerTime[0]) * 10 + HBATime[12] - tmp->TriggerTime[1];
                    int m = (HBATime[14] - tmp->TriggerTime[3]) * 10 + HBATime[15] - tmp->TriggerTime[4];
                    int s = (HBATime[17] - tmp->TriggerTime[6]) * 10 + HBATime[18] - tmp->TriggerTime[7];
                    if (h == 0 && m == 0 && abs(s) < 5) {
                        SCE_CONDITION_T tmp1 = tmp->ConditionListFollowCondition;
                        while (tmp1->ConditionListNextMoreConditions != NULL &&
                               tmp1->ConditionPointer->isConditionSatisfied == 1) {
                            tmp1 = tmp1->ConditionListNextMoreConditions;
                        }
                        if (tmp1->ConditionListNextMoreConditions == NULL) {
                            if (tmp1->EndOfListCMD == NULL) {
                                tmp = tmp->TriggerListNextTrigger;
                                continue;
                            }
                            SCE_COMMAND_T tmp2 = tmp1->EndOfListCMD;
                            IotPacketInterface tmp;
                            memset(&tmp, 0, sizeof(IotPacketInterface));
                            char buf[30];
                            tmp.opCode[0] = '0';
                            tmp.opCode[1] = '8';
                            sprintf(tmp.payLoad, "%d_%d_%s_", tmp2->CmdIotId, tmp2->CmdDevClass, tmp2->CmdContent);
                            Encrypt(tmp.payLoad, strlen(tmp.payLoad), pin, tmp.payLoad);
                            memcpy(buf, &tmp, 30);
                            send(LocalSocket, buf, 30, 0);
                            printf("Send Ok\n");
                        }
                    }
                }
            }
            tmp = tmp->TriggerListNextTrigger;
        }
    }
}

void CreateExeMsgThread() {
    pthread_t t, t1, t2;
    pthread_create(&t, NULL, ExeMsg, NULL);
    pthread_create(&t1, NULL, UpdateConditionStatus, NULL);
    pthread_create(&t2, NULL, checkTimeSceCmd, NULL);
}