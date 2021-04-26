#include "KaisaEncrypt.h"
#include "Connect2Server.h"
#include "RecvFromServer.h"
#include "SceCmd.h"
#include "SceCmdConfig.h"
#include "Read_ds18b20.h"

extern
struct SceCmdHead_S
{
    int TriggerNum;
    SCE_TRIGGER_T next;
}SceCmdHead;

extern
struct SceCmd_Condition_Head_S
{
    int ConditionNum;
    SCE_CONDITION_T next;
}SceCmd_Condition_Head;

extern int LocalSocket;
extern pthread_mutex_t MSG_LIST_MUTEX;
int isRegSign = 0;
int whileThreadFlag = 1;
char ID[10];
char PWD[10];
int GetIotCmd(int devClass){
    IotPacketInterface tmp;
    memset(&tmp, 0, sizeof(IotPacketInterface));
    char buf[11];
    tmp.opCode[0] = '0';
    tmp.opCode[1] = '5';
    sprintf(tmp.payLoad,"%d_",devClass);
    Encrypt(tmp.payLoad, strlen(tmp.payLoad), pin, tmp.payLoad);
    memcpy(buf, &tmp, 11);
    return send(LocalSocket,buf,11,0);
}

int UpdateTempData(float temp){
    IotPacketInterface tmp;
    memset(&tmp, 0, sizeof(IotPacketInterface));
    char buf[20];
    tmp.opCode[0] = '0';
    tmp.opCode[1] = '4';
    sprintf(tmp.payLoad,"1_%.2f_",temp);
    Encrypt(tmp.payLoad, strlen(tmp.payLoad), pin, tmp.payLoad);
    memcpy(buf, &tmp, 20);
    return send(LocalSocket,buf,20,0);
}

void GetSceCmd(){
    IotPacketInterface tmp;
    memset(&tmp, 0, sizeof(IotPacketInterface));
    char tmpbuf[11];
    tmp.opCode[0] = '0';
    tmp.opCode[1] = '6';
    memcpy(tmpbuf, &tmp, 11);
    send(LocalSocket,tmpbuf,11,0);
}

int main(int argc , char** argv)
{
    if(argc<4)
        exit(-1);
    sprintf(ID,"%s",argv[1]);
    sprintf(PWD,"%s",argv[2]);
    pthread_mutex_init(&MSG_LIST_MUTEX,NULL);
    memset(&MSG_HEAD,0,sizeof((MSG_HEAD)));
    memset(&SceCmdHead,0,sizeof(struct SceCmdHead_S));
    memset(&SceCmd_Condition_Head,0,sizeof(struct SceCmd_Condition_Head_S));
    Connect2Server();
    CreateExeMsgThread();
    CreateRecvThread();
    GetRsaKey();
    while (whileThreadFlag)
    {
        sleep(3);
        //GetIotCmd(0);
        GetIotCmd(98);
        GetSceCmd();
        UpdateTempData(ReadTemperature(argv[3]));
    };
    pthread_mutex_destroy(&MSG_LIST_MUTEX);
}