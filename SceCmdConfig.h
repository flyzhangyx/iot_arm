//
// Created by 18208 on 2021/4/15.
//

#ifndef UNTITLED_SCECMDCONFIG_H
#define UNTITLED_SCECMDCONFIG_H
#include "RecvFromServer.h"
#include "StrSplit.h"

typedef struct cond{
    int Hash;
    int isConditionSatisfied;
    int ConditionIotId;
    int ConditionDevClass;
    int ConditionJudgment;
    char ConditionEvent[10];
    struct cond* CondListNextMoreCond;
}*COND_T;

typedef struct COMMAND
{
    int CmdIotId;
    int CmdDevClass;
    char CmdContent[10];
}*SCE_COMMAND_T;

typedef struct Condition
{
    COND_T ConditionPointer;
    struct Condition* ConditionListNextMoreConditions;
    struct COMMAND* EndOfListCMD;
}*SCE_CONDITION_T;

typedef struct Trigger
{
    unsigned int Hash;
    int TriggerIotId;
    int TriggerDevClass;
    int TriggerJudgment;
    char TriggerEvent[10];
    struct Trigger* TriggerListNextTrigger;
    struct Condition* ConditionListFollowCondition;
}*SCE_TRIGGER_T;
int add2SceList(IotPacketInterface recvStruct);
#endif //UNTITLED_SCECMDCONFIG_H
