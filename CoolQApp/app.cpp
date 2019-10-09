#include "stdafx.h"
#include "app.h"

using namespace std;

extern int gAuthCode;//AuthCode

//正则表达式
regex gPexelsRegExp("data-photo-modal-image-download-link=\'(.*?)\'");
regex gBingRegExp("src=\"(.*?)\"");

//各来源的最大页码
int gPexelsMaxPage = 170, gBaiduMaxPage = 100, gBingMaxPage = 90;

//由于Pexels访问速度较慢，所以缓存每一次访问后取得的清单
vector<vector<string>> gPexelsImageUrlList(gPexelsMaxPage);

//处理锁
bool gDisposeLock = false;

void AppDispose(int8_t pType,const char *pMsg, int64_t pFromQQ, int64_t pFromGroup) {
	
	if (!strcmp(pMsg, "猫")) {
		//检测是否有线程正在处理，一次只允许一个处理，避免卡死
		if (gDisposeLock) {
			CQSendMessage(pType, pFromQQ, pFromGroup, "上次的猫片还在下载呢~请耐心等待");
			return;
		}

		gDisposeLock = true;//开启处理锁
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "指令匹配完成，正在获取猫图像");
		string fileName;

		switch (ReadConfig("Api", "s", 0, "config.ini")) {
		case 0:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "选择图片来源：Baidu");
			fileName = CatPhoto::GetPhotoBaidu();
			break;
		case 1:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "选择图片来源：Pexels");
			fileName = CatPhoto::GetPhotoPexels();
			break;
		case 2:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "选择图片来源：Bing");
			fileName = CatPhoto::GetPhotoBing();
			break;
		default:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "选择图片来源：Baidu");
			fileName = CatPhoto::GetPhotoBaidu();
		}

		if (fileName != "") {
			char imageCode[128];
			sprintf_s(imageCode, "[CQ:image,file=%s]", fileName.c_str());
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "image", imageCode);

			CQSendMessage(pType, pFromQQ, pFromGroup, imageCode);
		}
		
		gDisposeLock = false;
	}
}

/*
 * 获取猫猫图片，返回图片文件名
 * API Bing
*/
string CatPhoto::GetPhotoBing() {
	int page = RandomS(gBingMaxPage, "random page");

	char bingUrl[256];
	sprintf_s(bingUrl
		, "https://cn.bing.com/images/async?q=%s&first=%d&count=5&relp=5&layout=RowBased_Landscape&mmasync=1"
		, "%E7%8C%AB%E7%8C%AB"
		, page * 10);

	string searchString(ReadWebStr(bingUrl));//取网页源码

	string::const_iterator iterStart = searchString.begin();
	string::const_iterator iterEnd = searchString.end();


	smatch regexResult;
	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "取出猫片清单");
	vector<string> imageList;
	while (regex_search(iterStart, iterEnd, regexResult, gBingRegExp)) {
		string url = regexResult[1].str();
		imageList.push_back(url.substr(0, url.find("&amp")));
		iterStart = regexResult[0].second;
	}

	int listSize = imageList.size();
	if (listSize == 0) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "图片清单为空");
		return "";
	}

	int index = RandomS(listSize - 1, "index");

	//通过将图片的下载url转为md5后当作文件名，确保图像的唯一性
	MD5 *fileNameMD5 = new MD5(imageList[index]);
	string fileName = "bing_" + fileNameMD5->toString() + ".img";
	delete fileNameMD5;

	if (!ImageDownload(imageList[index], fileName)) {
		return "";
	}

	return fileName;
}

/*
 * 获取猫猫图片，返回图片文件名
 * API Baidu
*/
string CatPhoto::GetPhotoBaidu() {
	int page = RandomS(gBaiduMaxPage, "random page");

	char baiduUrl[256];
	sprintf_s(baiduUrl
		, "http://m.baidu.com/sf/vsearch/image/search/wisesearchresult?tn=wisejsonala&ie=utf-8&fromsf=1&word=%s&pn=%d&rn=10&gsm=&searchtype=2&prefresh=undefined&from=link&type=2"
		, "%E7%8C%AB+%E5%8A%A8%E7%89%A9+%E5%8F%AF%E7%88%B1"
		, page * 10);
	string searchString(ReadWebStr(baiduUrl));//取网页源码

	Json::Reader *reader = new Json::Reader(Json::Features::strictMode());
	Json::Value jsonValue;
	reader->parse(searchString, jsonValue);
	delete reader;

	if (jsonValue["linkData"].size() <= 0) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "linkData为空");
		return "";
	}

	int index = RandomS(jsonValue["linkData"].size() - 1, "index");

	Json::Value jsonImgValue;

	//取出objurl
	jsonImgValue = jsonValue["linkData"][index].get("objurl", NULL);
	if (jsonImgValue == NULL) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "objurl为空");
		return "";
	}

	string imgUrl = jsonImgValue.asString();
	if (imgUrl.find("tuzhan") != imgUrl.npos) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "检测到图片来源为tuzhan，使用thumbnailUrl");
		jsonImgValue = jsonValue["linkData"][index].get("thumbnailUrl", NULL);
		imgUrl = jsonImgValue.asString();
	}

	//通过将图片的下载url转为md5后当作文件名，确保图像的唯一性
	MD5 *fileNameMd5 = new MD5(imgUrl);
	string fileName = fileNameMd5->toString() + ".img";
	delete fileNameMd5;

	if (!ImageDownload(imgUrl, fileName)) {
		//如果下载失败，则切换图片来源再次下载
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "objurl下载失败，尝试使用hoverUrl");
		jsonImgValue = jsonValue["linkData"][index].get("hoverUrl", NULL);
		if (jsonImgValue == NULL) {
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "hoverUrl为空");
			return "";
		}
		imgUrl = jsonImgValue.asString();

		if (!ImageDownload(imgUrl, fileName)) {
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "hoverUrl下载失败");
			return "";
		}
	}

	return fileName;
}

/*
 * 获取猫猫图片，返回图片文件名
 * API Pexels
*/
string CatPhoto::GetPhotoPexels()
{
	int page = RandomS(gPexelsMaxPage, "random page");

	if (gPexelsImageUrlList[page].size() == 0) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "正在缓存图像列表");

		char pexelsUrl[64];
		srand((unsigned)time(NULL));

		sprintf_s(pexelsUrl, "https://www.pexels.com/cats/?format=html&page=%d", page + 1);
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "获取网页源码");
		string searchString(ReadWebStr(pexelsUrl));//取网页源码

		smatch regexResult;//正则匹配结果

		string::const_iterator iterStart = searchString.begin();
		string::const_iterator iterEnd = searchString.end();

		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "取出猫片清单");
		while (regex_search(iterStart, iterEnd, regexResult, gPexelsRegExp)) {
			string url = regexResult[1].str();
			gPexelsImageUrlList[page].push_back(url.substr(0, url.find('?')));
			iterStart = regexResult[0].second;
		}

		if (gPexelsImageUrlList[page].size() == 0) {
			//没找到任何图片
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "取猫片清单失败");
			return "";
		}

	}
	else {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "图像清单已缓存");
	}

	int listSize = gPexelsImageUrlList[page].size();

	int index = RandomS(listSize - 1, "index");
	string imageUrl = gPexelsImageUrlList[page][index];//随机获取一个图片url

	string fileName = imageUrl.substr(imageUrl.find_last_of('/') + 1, imageUrl.length() - 1);

	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "取图像清晰值");
	//取得w值 w值由用户自定义，代表了图像的清晰度，w值越高图像越清晰
	int _w = ReadConfig("image", "w", 0, "config.ini");
	if (_w == 0) {
		_w = 600;
		WriteConfig("image", "w", _w, "config.ini");
	}

	fileName = "w" + to_string(_w) + "-" + fileName;
	string downloadPath = imageUrl + "?" + "w=" + to_string(_w);

	if (!ImageDownload(downloadPath, fileName)) {
		return "";
	}

	return fileName;
}
