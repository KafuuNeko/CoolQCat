#pragma once

#include "Http.h"
#include "cqp.h"
#include <vector>
#include  <io.h>
#include <time.h>
#include <sstream>

#define PRIVATE_MSG 0
#define GROUP_MSG 1
#define DISCUSS_MSG 2

std::string GetPhoto_Cat();
void SendMessage(int8_t pType, int64_t id, int64_t group, const char *msg);
void AppDispose(int8_t pType, const char *pMsg, int64_t pFromQQ, int64_t pFromGroup);
char *ReadConfig(const char *pNode, const char *pItem, const char *pDefault, const char *pFileName);
int32_t ReadConfig(const char *pNode, const char *pItem, int32_t pDefault, const char *pFileName);
bool WriteConfig(const char *pNode, const char *pItem, int32_t pData, const char *pFileName);
bool WriteConfig(const char *pNode, const char *pItem, const char *pData, const char *pFileName);
void delay_msec(int msec);