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
		string fileName;


		switch (ReadConfig("Api", "s", 0, "config.ini")) {
		case 0:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "Api：Baidu");
			fileName = GetPhoto_Baidu();
			break;
		case 1:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "Api：Pexels");
			fileName = GetPhoto_Pexels();
			break;
		default:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "Api：Baidu");
			fileName = GetPhoto_Baidu();
		}

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

int pPexelsMaxPage = 170,pBaiduMaxPage = 100;
vector<vector<string>> pImageUrlList(pPexelsMaxPage);//图片url清单

/*
 * 获取猫猫图片，返回图片文件名
 * API Baidu
*/
string GetPhoto_Baidu() {
	ostringstream tempOstr;
	int page = 0;

cmd_rand_page:
	srand((unsigned)time(NULL));
	page = random(pBaiduMaxPage - 1);

	tempOstr << "random page:" << page;
	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", tempOstr.str().c_str());
	tempOstr.clear();
	tempOstr.str("");

	if (page < 0 || page > pBaiduMaxPage) {
		delay_msec(100);
		goto cmd_rand_page;
	}

	char url[256];
	sprintf_s(url
		,"http://m.baidu.com/sf/vsearch/image/search/wisesearchresult?tn=wisejsonala&ie=utf-8&fromsf=1&word=%s&pn=%d&rn=10&gsm=&searchtype=2&prefresh=undefined&from=link&type=2"
		,"%E7%8C%AB+%E5%8A%A8%E7%89%A9+%E5%8F%AF%E7%88%B1" 
		,page*10);
	string searchString(ReadWebStr(url));//取网页源码

	Json::Reader *reader = new Json::Reader(Json::Features::strictMode());
	Json::Value root;
	reader->parse(searchString, root);
	
	if (root["linkData"].size() <= 0) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "linkData为空");
		delete reader;
		return "";
	}

	int index = 0;
cmd_rand_index:
	srand((unsigned)time(NULL));
	index = random(root["linkData"].size() - 1);

	tempOstr << "index:" << index;
	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", tempOstr.str().c_str());
	tempOstr.clear();
	tempOstr.str("");

	if (index < 0) {
		delay_msec(100);
		goto cmd_rand_index;
	}

	//取出objurl
	Json::Value jsonImgValue = root["linkData"][index].get("objurl", NULL);

	if (jsonImgValue == NULL) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "objurl为空");
		delete reader;
		return "";
	}

	string imgUrl = jsonImgValue.asString();
	if (imgUrl.find("tuzhan") != imgUrl.npos) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "检测到图片来源为tuzhan，使用thumbnailUrl");
		jsonImgValue = root["linkData"][index].get("thumbnailUrl", NULL);
		imgUrl = jsonImgValue.asString();
	}
	MD5 *fileNameMd5 = new MD5(imgUrl);
	
	string fileName = fileNameMd5->toString()+".img";

	char savePath[256];
	sprintf_s(savePath, ".\\data\\image\\%s", fileName);

	//检查图片是否已经缓存，若已缓存，则无需下载
	if ((_access(savePath, 0)) == -1) {
		if (!DownloadSaveFiles((char *)imgUrl.c_str(), savePath)) {
			//如果下载失败，则切换图片来源再次下载
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "objurl下载失败，尝试使用hoverUrl");
			jsonImgValue = root["linkData"][index].get("hoverUrl", NULL);
			if (jsonImgValue == NULL) {
				CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "hoverUrl为空");
				delete reader;
				return "";
			}
			imgUrl = jsonImgValue.asString();
			if (!DownloadSaveFiles((char *)imgUrl.c_str(), savePath)) {
				CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "hoverUrl下载失败");
				delete reader;
				return "";
			}
		}
	}
	else {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "检测到猫片已缓存");
	}

	delete reader;
	return fileName;
}

/*
 * 获取猫猫图片，返回图片文件名
 * API Pexels
*/
string GetPhoto_Pexels()
{
	ostringstream tempOstr;

	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "取页码");

	int page = 0;
cmd_rand_page:
	srand((unsigned)time(NULL));
	page = random(pPexelsMaxPage-1);
	
	tempOstr << "random page:" << page;
	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", tempOstr.str().c_str());
	tempOstr.clear();
	tempOstr.str("");


	if (page < 0 || page > pPexelsMaxPage) {
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

	int index = 0;
cmd_index_rand:
	srand((unsigned)time(NULL));
	index = random(listSize-1);

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