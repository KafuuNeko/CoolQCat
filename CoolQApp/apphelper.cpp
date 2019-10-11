#include "stdafx.h"
#include "apphelper.h"
#include "Http.h"
#include "cqp.h"

using namespace std;

extern int gAuthCode;//AuthCode

void gDelayMsec(int pnMsec)
{
	clock_t now = clock();
	while (clock() - now < pnMsec);
}

void gCQSendMessage(int8_t pnType, int64_t pnId, int64_t pnGroup, const char *pcMsg) {
	if (pnType == PRIVATE_MSG) {
		CQ_sendPrivateMsg(gAuthCode, pnId, pcMsg);
	}
	else if (pnType == GROUP_MSG) {
		CQ_sendGroupMsg(gAuthCode, pnGroup, pcMsg);
	}
	else if (pnType == DISCUSS_MSG) {
		CQ_sendDiscussMsg(gAuthCode, pnGroup, pcMsg);
	}
}

char *gReadConfig(const char *pcNode, const char *pcItem, const char *pcDefault, const char *pcFileName) {
	ostringstream configFilePath;
	configFilePath << CQ_getAppDirectory(gAuthCode) << pcFileName;
	char *res = new char[128];
	GetPrivateProfileString(pcNode, pcItem, pcDefault, res, 128, configFilePath.str().c_str());
	return res;
}

int32_t gReadConfig(const char *pcNode, const char *pcItem, int32_t pnDefault, const char *pcFileName) {
	ostringstream configFilePath;
	configFilePath << CQ_getAppDirectory(gAuthCode) << pcFileName;
	return GetPrivateProfileInt(pcNode, pcItem, pnDefault, configFilePath.str().c_str());
}

bool gWriteConfig(const char *pcNode, const char *pcItem, int32_t pnData, const char *pcFileName) {
	ostringstream filePath, writeData;

	filePath << CQ_getAppDirectory(gAuthCode) << pcFileName;
	writeData << pnData;

	return WritePrivateProfileString(pcNode, pcItem, writeData.str().c_str(), filePath.str().c_str());
}

bool gWriteConfig(const char *pcNode, const char *pcItem, const char *pcData, const char *pcFileName) {
	ostringstream filePath;

	filePath << CQ_getAppDirectory(gAuthCode) << pcFileName;

	return WritePrivateProfileString(pcNode, pcItem, pcData, filePath.str().c_str());
}

/*
* 下载图片，成功返回true
*/
bool gImageDownload(string psDownUrl, string psFileName) {
	char savePath[256];
	sprintf_s(savePath, ".\\data\\image\\%s", psFileName.c_str());

	//检查图片是否已经缓存，若已缓存，则无需下载
	if ((_access(savePath, 0)) == -1) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "download", psDownUrl.c_str());
		if (!gDownloadSaveFiles((char *)psDownUrl.c_str(), savePath)) {
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
int gRandomS(int pnMax, string psDescribe) {
	ostringstream tempOstr;

	mt19937 rng;
	rng.seed(random_device()());
	uniform_int_distribution<int32_t> dis(0, pnMax);
	int32_t res = dis(rng);
	
	tempOstr << psDescribe << ":Max[" << pnMax << "];Result[" << res << "]";

	CQ_addLog(gAuthCode, CQLOG_DEBUG, "Random Device", tempOstr.str().c_str());

	return res;
}
