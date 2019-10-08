#include "stdafx.h"
#include "app.h"

using namespace std;
extern int gAuthCode;//AuthCode
regex gPexelsRegExp("data-photo-modal-image-download-link=\'(.*?)\'");
bool gDisposeLock = false;

void AppDispose(int8_t pType,const char *pMsg, int64_t pFromQQ, int64_t pFromGroup) {
	if (!strcmp(pMsg, "猫")) {
		//检测是否有线程正在处理，一次只允许一个处理，避免卡死
		if (gDisposeLock) {
			SendMessage(pType, pFromQQ, pFromGroup, "上次的猫片还在下载呢~请耐心等待");
			return;
		}

		gDisposeLock = true;//开启处理锁
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "指令匹配完成，正在获取猫图像");
		string fileName = GetPhoto_Cat();
		if (fileName != "") {
			char imageCode[128];
			sprintf_s(imageCode, "[CQ:image,file=%s]", fileName.c_str());
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "image", imageCode);

			SendMessage(pType, pFromQQ, pFromGroup, imageCode);
		}
		
		gDisposeLock = false;
	}
}

void SendMessage(int8_t pType, int64_t id, int64_t group, const char *msg) {
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

int pMaxPage = 170;
vector<vector<string>> pImageUrlList(pMaxPage);//图片url清单

string GetPhoto_Cat()
{
	ostringstream tempOstr;

	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "取页码");

cmd_rand_page:
	int page = random(pMaxPage-1);
	
	tempOstr << "random page:" << page;
	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", tempOstr.str().c_str());
	tempOstr.clear();
	tempOstr.str("");


	if (page < 0 || page > 99) {
		delay_msec(100);//等待100ms重试
		goto cmd_rand_page;
	}

	if (pImageUrlList[page].size() == 0) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "正在缓存图像列表");

		char pexelsUrl[64];
		srand((unsigned)time(NULL));
		
		sprintf_s(pexelsUrl, "https://www.pexels.com/cats/?format=html&page=%d", page+1);
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "获取网页源码");
		string searchString(ReadWebStr(pexelsUrl));//取网页源码

		smatch regexResult;//正则匹配结果

		string::const_iterator iterStart = searchString.begin();
		string::const_iterator iterEnd = searchString.end();

		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "取出猫片清单");
		while (regex_search(iterStart, iterEnd, regexResult, gPexelsRegExp)) {
			string url = regexResult[1].str();
			pImageUrlList[page].push_back(url.substr(0, url.find('?')));
			iterStart = regexResult[0].second;
		}

		if (pImageUrlList[page].size() == 0) {
			//没找到任何图片
			return "";
		}

	}
	else {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "图像清单已缓存");
	}
	
	int listSize = pImageUrlList[page].size();

	tempOstr << "图像清单条目:" << listSize;
	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", tempOstr.str().c_str());
	tempOstr.clear();
	tempOstr.str("");

cmd_index_rand:
	srand((unsigned)time(NULL));
	int index = random(listSize-1);

	tempOstr << "index:" << index;
	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", tempOstr.str().c_str());
	tempOstr.clear();
	tempOstr.str("");

	if (index < 0 || index >= listSize) {
		delay_msec(100);//等待100ms重试
		goto cmd_index_rand;
	}

	string imageUrl = pImageUrlList[page][index];//随机获取一个图片url
	string fileName = imageUrl.substr(imageUrl.find_last_of('/') + 1, imageUrl.length() - 1);

	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "取图像清晰值");
	//取得w值 w值由用户自定义，代表了图像的清晰度，w值越高图像越清晰
	int _w = ReadConfig("image", "w", 0, "config.ini");
	if (_w == 0) {
		_w = 600;
		WriteConfig("image", "w", _w, "config.ini");
	}

	tempOstr << "w" << _w << "-" << fileName;
	fileName = tempOstr.str();
	tempOstr.clear();
	tempOstr.str("");

	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "制取保存路径");
	//取得保存路径
	char savePath[256];
	sprintf_s(savePath, ".\\data\\image\\%s", fileName.c_str());

	if ((_access(savePath, 0)) == -1) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "开始下载猫片");
		//文件不存在，则下载猫片
		//取得图片url
		char downloadPath[256];
		sprintf_s(downloadPath, "%s?w=%d", imageUrl.c_str(), _w);

		//下载图片
		if (!DownloadSaveFiles(downloadPath, savePath)) {
			return "";
		}
	}
	else {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "检测到猫片已缓存");
	}

	return fileName;	
}

char *ReadConfig(const char *pNode, const char *pItem, const char *pDefault, const char *pFileName) {
	ostringstream configFilePath;
	configFilePath << CQ_getAppDirectory(gAuthCode) << pFileName;
	char *res = new char[128];
	GetPrivateProfileString(pNode, pItem, pDefault, res, 128, configFilePath.str().c_str());
	return res;
}

int32_t ReadConfig(const char *pNode,const char *pItem,int32_t pDefault,const char *pFileName) {
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

void delay_msec(int msec)
{
	clock_t now = clock();
	while (clock() - now < msec);
}