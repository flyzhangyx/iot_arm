//
// Created by 18208 on 2021/4/15.
//

#include "SceCmdConfig.h"

struct SceCmdHead_S {
    int TriggerNum;
    SCE_TRIGGER_T next;
} SceCmdHead;

struct SceCmd_Condition_Head_S {
    int ConditionNum;
    COND_T first;
} SceCmd_Condition_Head;

COND_T add2CondList(char *CondStr) {
    int CondHash = DJBHash(CondStr, strlen(CondStr));
    COND_T tmp = SceCmd_Condition_Head.first;
    while (tmp != NULL) {
        if (tmp->Hash == CondHash) {
            return tmp;
        }
        tmp = tmp->CondListNextMoreCond;
    }
    tmp = SceCmd_Condition_Head.first;
    if (tmp == NULL) {
        SceCmd_Condition_Head.first = (COND_T) malloc(sizeof(struct cond));
        if (SceCmd_Condition_Head.first == NULL)
            return NULL;
    } else {
        while (tmp->CondListNextMoreCond != NULL) {
            tmp = tmp->CondListNextMoreCond;
        }
        tmp->CondListNextMoreCond = (COND_T) malloc(sizeof(struct cond));
        if (tmp->CondListNextMoreCond == NULL)
            return NULL;
    }
    COND_T Condition ;
    if (tmp == NULL) {
        Condition = SceCmd_Condition_Head.first;
    } else
        Condition = tmp->CondListNextMoreCond;
    memset(Condition, 0, sizeof(struct cond));
    int OutStrSize2 = 0;
    char **outStr2 = StrSplit(CondStr, strlen(CondStr), &OutStrSize2, '+');//CMD_CONTENT
    Condition->ConditionIotId = atoi(outStr2[0]);
    Condition->ConditionDevClass = atoi(outStr2[1]);
    Condition->ConditionJudgment = outStr2[2][0] == '<' ? -1 : (outStr2[2][0] == '>' ? 1 : 0);
    Stringcut(outStr2[2], 1, strlen(outStr2[2]), Condition->ConditionEvent);
    Condition->isConditionSatisfied = 0;
    Condition->Hash = CondHash;
    SceCmd_Condition_Head.ConditionNum++;
    return Condition;
}

int add2SceList(IotPacketInterface recvStruct) {
    if (atoi(recvStruct.opCode) == 6) {
        int hash = DJBHash(recvStruct.payLoad, strlen(recvStruct.payLoad));
        SCE_TRIGGER_T tmp_sce = SceCmdHead.next;
        while (tmp_sce != NULL) {
            if (tmp_sce->Hash == hash)
                return 0;
            tmp_sce = tmp_sce->TriggerListNextTrigger;
        }
        int OutStrSize = 0;
        char **outStr = StrSplit(recvStruct.payLoad, 200, &OutStrSize, '_');
        if (OutStrSize != 7) {
            releaseStr(outStr, OutStrSize);
            return -1;
        }
        switch (atoi(outStr[0])) {
            case 99: {//Offload to this Arm Dev
                int OutStrSize1 = 0;
                char **outStr1 = StrSplit(outStr[1], strlen(outStr[1]), &OutStrSize1, '-');//CMD_CONTENT
                if (OutStrSize1 != 7) {
                    releaseStr(outStr1, OutStrSize1);
                    releaseStr(outStr, OutStrSize);
                    return -2;
                }
                int OutStrSize2 = 0;
                char **outStr2 = StrSplit(outStr1[3], strlen(outStr1[3]), &OutStrSize2, '+');//CMD_CONTENT
                if (OutStrSize2 % 3 != 0) {
                    releaseStr(outStr2, OutStrSize2);
                    releaseStr(outStr1, OutStrSize1);
                    releaseStr(outStr, OutStrSize);
                    return -3;
                }
                SCE_TRIGGER_T tmp = SceCmdHead.next;
                if (tmp != NULL)
                    while (tmp->TriggerListNextTrigger != NULL) {
                        tmp = tmp->TriggerListNextTrigger;
                    }
                SCE_TRIGGER_T NewSceCmd = (SCE_TRIGGER_T) malloc(sizeof(struct Trigger));
                if (NewSceCmd == NULL) {
                    releaseStr(outStr2, OutStrSize2);
                    releaseStr(outStr1, OutStrSize1);
                    releaseStr(outStr, OutStrSize);
                    return -4;
                }
                if (tmp == NULL)
                    SceCmdHead.next = NewSceCmd;
                else
                    tmp->TriggerListNextTrigger = NewSceCmd;
                memset(NewSceCmd, 0, sizeof(struct Trigger));
                NewSceCmd->ConditionListFollowCondition = NULL;
                NewSceCmd->TriggerListNextTrigger = NULL;
                NewSceCmd->TriggerDevClass = atoi(outStr1[1]);
                NewSceCmd->TriggerIotId = atoi(outStr1[0]);
                NewSceCmd->TriggerJudgment = outStr1[2][0] == '<' ? -1 : (outStr1[2][0] == '>' ? 1 : 0);
                Stringcut(outStr1[2], 1, strlen(outStr1[2]), NewSceCmd->TriggerEvent);
                NewSceCmd->Hash = hash;
                int flag = 0;
                int i = 0;
                SCE_CONDITION_T Condition = NULL;
                while (i < OutStrSize2 / 3) {
                    if (Condition == NULL) {
                        Condition = (SCE_CONDITION_T) malloc(sizeof(struct Condition));
                        NewSceCmd->ConditionListFollowCondition = Condition;
                    } else {
                        Condition->ConditionListNextMoreConditions = (SCE_CONDITION_T) malloc(
                                sizeof(struct Condition));
                        Condition = Condition->ConditionListNextMoreConditions;
                    }
                    if (Condition == NULL) {
                        if (tmp == NULL)
                            SceCmdHead.next = NULL;
                        else
                            tmp->TriggerListNextTrigger = NULL;
                        SCE_CONDITION_T tmp1 = NewSceCmd->ConditionListFollowCondition;
                        while (tmp1 != NULL) {
                            NewSceCmd->ConditionListFollowCondition = tmp1->ConditionListNextMoreConditions;
                            free(tmp1);
                            tmp1 = NewSceCmd->ConditionListFollowCondition;
                        }
                        free(NewSceCmd);
                        releaseStr(outStr2, OutStrSize2);
                        releaseStr(outStr1, OutStrSize1);
                        releaseStr(outStr, OutStrSize);
                        return -5;
                    }
                    memset(Condition, 0, sizeof(struct Condition));
                    Condition->ConditionListNextMoreConditions = NULL;
                    Condition->EndOfListCMD = NULL;
                    char CondStr[100] = "";
                    int j = 0;
                    int startIndex = flag;
                    while(j<3){
                        if(*(flag+outStr1[3]) == '+')
                            j++;
                        flag++;
                    }
                    Stringcut(outStr1[3],startIndex,flag-1,CondStr);
                    Condition->ConditionPointer = add2CondList(CondStr);
                    i++;
                }
                SCE_COMMAND_T CMD = (SCE_COMMAND_T) malloc(sizeof(struct COMMAND));
                if (CMD == NULL) {
                    SCE_CONDITION_T tmp1 = NewSceCmd->ConditionListFollowCondition;
                    while (tmp1 != NULL) {
                        NewSceCmd->ConditionListFollowCondition = tmp1->ConditionListNextMoreConditions;
                        free(tmp1);
                        tmp1 = NewSceCmd->ConditionListFollowCondition;
                    }
                    free(NewSceCmd);
                    releaseStr(outStr2, OutStrSize2);
                    releaseStr(outStr1, OutStrSize1);
                    releaseStr(outStr, OutStrSize);
                    return -6;
                }
                Condition->EndOfListCMD = CMD;
                memset(CMD, 0, sizeof(struct COMMAND));
                CMD->CmdIotId = atoi(outStr1[4]);
                CMD->CmdDevClass = atoi(outStr1[5]);
                memcpy(CMD->CmdContent, outStr1[6], strlen(outStr1[6]) + 1);
                releaseStr(outStr2, OutStrSize2);
                releaseStr(outStr1, OutStrSize1);
            }
                break;
            default:
                printf("%s\n", recvStruct.opCode);
                break;
        }
        releaseStr(outStr, OutStrSize);
        SceCmdHead.TriggerNum++;
    } else {
        printf("Err Type Sce Msg\n");
    }
    return 0;
}