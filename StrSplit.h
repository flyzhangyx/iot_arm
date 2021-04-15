//
// Created by 18208 on 2021/4/14.
//

#ifndef UNTITLED_STRSPLIT_H
#define UNTITLED_STRSPLIT_H
#include <malloc.h>
int Stringcut(char *str, int m, int n, char *des);
char **StrSplit(char *srcStr, int srcStrLen, int *outBufferLen, char delim);
void releaseStr(char **StrVector, int size);
#endif //UNTITLED_STRSPLIT_H
