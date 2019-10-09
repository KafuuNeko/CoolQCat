#include "stdafx.h"
#include "apphelper.h"

using namespace std;

extern int gAuthCode;//AuthCode

void delay_msec(int msec)
{
	clock_t now = clock();
	while (clock() - now < msec);
}

void CQSendMessage(int8_t pType, int64_t id, int64_t group, const char *msg) {
	if (pType == PRIVATE_MSG) {
		CQ_sendPrivateMsg(gAuthCode, id, msg);
	}
	else if (pType == GROUP_MSG) {
		CQ_sendGroupMsg(gAuthCode, group, msg);
	}
	else if (pType == DISCUSS_MSG) {
		CQ_sendDiscussMsg(gAuthCode, group, msg);
	}
}

char *ReadConfig(const char *pNode, const char *pItem, const char *pDefault, const char *pFileName) {
	ostringstream configFilePath;
	configFilePath << CQ_getAppDirectory(gAuthCode) << pFileName;
	char *res = new char[128];
	GetPrivateProfileString(pNode, pItem, pDefault, res, 128, configFilePath.str().c_str());
	return res;
}

int32_t ReadConfig(const char *pNode, const char *pItem, int32_t pDefault, const char *pFileName) {
	ostringstream configFilePath;
	configFilePath << CQ_getAppDirectory(gAuthCode) << pFileName;
	return GetPrivateProfileInt(pNode, pItem, pDefault, configFilePath.str().c_str());
}

bool WriteConfig(const char *pNode, const char *pItem, int32_t pData, const char *pFileName) {
	ostringstream filePath, writeData;

	filePath << CQ_getAppDirectory(gAuthCode) << pFileName;
	writeData << pData;

	return WritePrivateProfileString(pNode, pItem, writeData.str().c_str(), filePath.str().c_str());
}

bool WriteConfig(const char *pNode, const char *pItem, const char *pData, const char *pFileName) {
	ostringstream filePath;

	filePath << CQ_getAppDirectory(gAuthCode) << pFileName;

	return WritePrivateProfileString(pNode, pItem, pData, filePath.str().c_str());
}

/*
* 下载图片，成功返回true
*/
bool ImageDownload(string pDownUrl, string pFileName) {
	char savePath[256];
	sprintf_s(savePath, ".\\data\\image\\%s", pFileName.c_str());

	//检查图片是否已经缓存，若已缓存，则无需下载
	if ((_access(savePath, 0)) == -1) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "download", pDownUrl.c_str());
		if (!DownloadSaveFiles((char *)pDownUrl.c_str(), savePath)) {
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "图片下载失败");
			return false;
		}
	}
	else {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "猫片已存在");
	}

	return true;
}

/*
 *获取随机数
*/
int RandomS(int pMax, string pDescribe) {
	ostringstream tempOstr;

	int page = 0;
cmd_rand_page:
	srand((unsigned)time(NULL));
	page = random(pMax - 1);

	tempOstr << pDescribe << ":" << page;
	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", tempOstr.str().c_str());
	tempOstr.clear();
	tempOstr.str("");

	if (page < 0 || page > pMax) {
		delay_msec(100);
		goto cmd_rand_page;
	}
	return page;
}
