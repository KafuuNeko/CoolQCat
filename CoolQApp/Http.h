#pragma once

#include <afxinet.h>
#include <regex>
#include <stdio.h>
#include <string>

#define RECVPACK_SIZE 4096

using std::string;

bool gDownloadSaveFiles(char* pcUrl, char *pcSaveFile);
string gReadWebStr(const char *pcUrl);
