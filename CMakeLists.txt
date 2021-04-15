cmake_minimum_required(VERSION 3.13)
project(untitled C)

set(CMAKE_C_STANDARD 99)

add_executable(untitled main.c RSAUtils.c RSAUtils.h KaisaEncrypt.c KaisaEncrypt.h Connect2Server.c Connect2Server.h RecvFromServer.c RecvFromServer.h SceCmd.c SceCmd.h GetRsa.c GetRsa.h StrSplit.c StrSplit.h RegSign.c RegSign.h SceCmdConfig.c SceCmdConfig.h)

find_package(Threads)

target_link_libraries(untitled ${CMAKE_THREAD_LIBS_INIT})