//
// Created by 18208 on 2021/4/22.
//

#include "Read_ds18b20.h"
float ReadTemperature(char* str)
{
    float temp;
    int i, j;
    int fd;
    int ret;

    char buf[BUFSIZE];
    char tempBuf[5];
    char path[100];
    sprintf(path,"%s%s%s","/sys/bus/w1/devices/",str,"/w1_slave");
    fd = open(path, O_RDONLY); //以只读方式打开ds18b20设备文件

    if(-1 == fd){              //打开设备文件出错
        perror("open device file error");  //打印出错信息（原因）
        return 1;
    }

    while(1){
        ret = read(fd, buf, BUFSIZE);  //将设备文件中的内容读入buf中
        if(0 == ret){                  //返回0表示读文件结束
            break;	                   //跳出循环体
        }
        if(-1 == ret){                 //读文件出错
            if(errno == EINTR){        //出错是由中断引起的
                continue;	           //继续读文件
            }
            perror("read()");          //打印出错信息（原因）
            close(fd);                 //关闭文件
            return 1;
        }
    }

    for(i=0;i<sizeof(buf);i++){
        if(buf[i] == 't'){               //如果读到‘t’，说明后面马上出现温度值
            for(j=0;j<sizeof(tempBuf);j++){
                tempBuf[j] = buf[i+2+j]; //将温度值以字符串的形式写入tempBuf中
            }
        }
    }

    temp = (float)atoi(tempBuf) / 1000;  //将字符串转换为浮点型温度数据

    printf("%.3f C\n",temp);             //打印出温度值

    close(fd);                           //关闭文件

    return temp;
}