#pragma once

#include <vector>
#include <io.h>
#include <time.h>
#include <sstream>

#include "Http.h"
#include "cqp.h"

using std::string;

#define PRIVATE_MSG 0
#define GROUP_MSG 1
#define DISCUSS_MSG 2

void delay_msec(int msec);

void CQSendMessage(int8_t pType, int64_t id, int64_t group, const char *msg);

char *ReadConfig(const char *pNode, const char *pItem, const char *pDefault, const char *pFileName);
int32_t ReadConfig(const char *pNode, const char *pItem, int32_t pDefault, const char *pFileName);
bool WriteConfig(const char *pNode, const char *pItem, int32_t pData, const char *pFileName);
bool WriteConfig(const char *pNode, const char *pItem, const char *pData, const char *pFileName);

bool ImageDownload(string pDownUrl, string pFileName);
int RandomS(int pMax, string pDescribe);