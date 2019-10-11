#pragma once

#include <afxinet.h>
#include <regex>
#include <stdio.h>
#include <string>

#define RECVPACK_SIZE 2048

using std::string;

bool gDownloadSaveFiles(char* pcUrl, char *pcSaveFile);
string gReadWebStr(const char *pcUrl);
