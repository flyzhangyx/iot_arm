//
// Created by 18208 on 2021/4/22.
//

#ifndef UNTITLED_READ_DS18B20_H
#define UNTITLED_READ_DS18B20_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#define  BUFSIZE  128
float ReadTemperature(char*);
#endif //UNTITLED_READ_DS18B20_H
