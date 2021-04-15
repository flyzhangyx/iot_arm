//
// Created by 18208 on 2021/4/14.
//

#include "Connect2Server.h"
void Connect2Server()
{
    struct sockaddr_in servaddr;
    if( (LocalSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        exit(0);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(3570);
    servaddr.sin_addr.s_addr = inet_addr("47.106.207.241");
    if( connect(LocalSocket, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
        exit(0);
    }
}