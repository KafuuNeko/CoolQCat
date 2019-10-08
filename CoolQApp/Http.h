#pragma once

#define random(x) (rand()%x)

#define RECVPACK_SIZE 2048

#include <afxinet.h>
#include <regex>
#include <stdio.h>
#include <string>

bool DownloadSaveFiles(char* url, char *strSaveFile);
std::string ReadWebStr(const char *url);
