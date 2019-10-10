#pragma once

#include <vector>
#include <io.h>
#include <time.h>
#include <sstream>

using std::string;

#define PRIVATE_MSG 0
#define GROUP_MSG 1
#define DISCUSS_MSG 2

void gDelayMsec(int pnMsec);

void gCQSendMessage(int8_t pnType, int64_t pnId, int64_t pnGroup, const char *pcMsg);

char *gReadConfig(const char *pcNode, const char *pcItem, const char *pcDefault, const char *pcFileName);
int32_t gReadConfig(const char *pcNode, const char *pcItem, int32_t pnDefault, const char *pcFileName);
bool gWriteConfig(const char *pcNode, const char *pcItem, int32_t pnData, const char *pcFileName);
bool gWriteConfig(const char *pcNode, const char *pcItem, const char *pcData, const char *pcFileName);

bool gImageDownload(string psDownUrl, string psFileName);
int gRandomS(int pMax, string psDescribe);