#include "stdafx.h"
#include "app.h"

#include "apphelper.h"
#include "Http.h"
#include "cqp.h"
#include "./md5/md5.h"
#include "./json/json.h"

using namespace std;

extern int gAuthCode;//AuthCode

//正则表达式
regex grPexelsRegExp("data-photo-modal-image-download-link=\'(.*?)\'");
regex grBingRegExp("src=\"(.*?)\"");

//各来源的最大页码
int gnPexelsMaxPage = 170, gnBaiduMaxPage = 100, gnBingMaxPage = 90;

//由于Pexels访问速度较慢，所以缓存每一次访问后取得的清单
vector<vector<string>> gvvsPexelsImageUrlList(gnPexelsMaxPage);

//处理锁
bool gbDisposeLock = false;

namespace CatPhoto {
	string __GetPhotoPexels();
	string __GetPhotoBaidu();
	string __GetPhotoBing();
}

void gAppDispose(int8_t pnType,const char *pcMsg, int64_t pnFromQQ, int64_t pnFromGroup) {
	
	if (!strcmp(pcMsg, "猫")) {
		//检测是否有线程正在处理，一次只允许一个处理，避免卡死
		if (gbDisposeLock) {
			gCQSendMessage(pnType, pnFromQQ, pnFromGroup, "上次的猫片还在下载呢~请耐心等待");
			return;
		}

		gbDisposeLock = true;//开启处理锁
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "指令匹配完成，正在获取猫图像");
		string sFileName;

		switch (gReadConfig("Api", "s", 0, "config.ini")) {
		case 0:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "选择图片来源：Baidu");
			sFileName = CatPhoto::__GetPhotoBaidu();
			break;
		case 1:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "选择图片来源：Pexels");
			sFileName = CatPhoto::__GetPhotoPexels();
			break;
		case 2:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "选择图片来源：Bing");
			sFileName = CatPhoto::__GetPhotoBing();
			break;
		default:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "选择图片来源：Baidu");
			sFileName = CatPhoto::__GetPhotoBaidu();
		}

		if (sFileName != "") {
			char cImageCode[128];
			sprintf_s(cImageCode, "[CQ:image,file=%s]", sFileName.c_str());
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "image", cImageCode);

			gCQSendMessage(pnType, pnFromQQ, pnFromGroup, cImageCode);
		}
		
		gbDisposeLock = false;
	}
}

/*
 * 获取猫猫图片，返回图片文件名
 * API Bing
*/
string CatPhoto::__GetPhotoBing() {
	int nPage = gRandomS(gnBingMaxPage, "page");

	char cBingUrl[256];
	sprintf_s(cBingUrl
		, "https://cn.bing.com/images/async?q=%s&first=%d&count=5&relp=5&layout=RowBased_Landscape&mmasync=1"
		, "%E7%8C%AB%E7%8C%AB"
		, nPage * 10);

	string searchString(gReadWebStr(cBingUrl));//取网页源码

	string::const_iterator iterStart = searchString.begin();
	string::const_iterator iterEnd = searchString.end();


	smatch smRegexResult;
	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "取出猫片清单");
	vector<string> vsImageList;
	while (regex_search(iterStart, iterEnd, smRegexResult, grBingRegExp)) {
		string url = smRegexResult[1].str();
		vsImageList.push_back(url.substr(0, url.find("&amp")));
		iterStart = smRegexResult[0].second;
	}

	int nListSize = vsImageList.size();
	if (nListSize == 0) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "图片清单为空");
		return "";
	}

	int nIndex = gRandomS(nListSize - 1, "index");

	//通过将图片的下载url转为md5后当作文件名，确保图像的唯一性
	MD5 *md5FileName = new MD5(vsImageList[nIndex]);
	string sFileName = "bing_" + md5FileName->toString() + ".img";
	delete md5FileName;

	if (!gImageDownload(vsImageList[nIndex], sFileName)) {
		return "";
	}

	return sFileName;
}

/*
 * 获取猫猫图片，返回图片文件名
 * API Baidu
*/
string CatPhoto::__GetPhotoBaidu() {
	int nPage = gRandomS(gnBaiduMaxPage, "page");

	char cBaiduUrl[256];
	sprintf_s(cBaiduUrl
		, "http://m.baidu.com/sf/vsearch/image/search/wisesearchresult?tn=wisejsonala&ie=utf-8&fromsf=1&word=%s&pn=%d&rn=10&gsm=&searchtype=2&prefresh=undefined&from=link&type=2"
		, "%E7%8C%AB+%E5%8A%A8%E7%89%A9+%E5%8F%AF%E7%88%B1"
		, nPage * 10);
	string sSearchString(gReadWebStr(cBaiduUrl));//取网页源码

	Json::Reader *jrReader = new Json::Reader(Json::Features::strictMode());
	Json::Value jvJsonValue;
	jrReader->parse(sSearchString, jvJsonValue);
	delete jrReader;

	if (jvJsonValue["linkData"].size() <= 0) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "linkData为空");
		return "";
	}

	int nIndex = gRandomS(jvJsonValue["linkData"].size() - 1, "index");

	Json::Value jvImgValue;

	//取出objurl
	jvImgValue = jvJsonValue["linkData"][nIndex].get("objurl", NULL);
	if (jvImgValue == NULL) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "objurl为空");
		return "";
	}

	string sImgUrl = jvImgValue.asString();
	if (sImgUrl.find("tuzhan") != sImgUrl.npos) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "检测到图片来源为tuzhan，使用thumbnailUrl");
		jvImgValue = jvJsonValue["linkData"][nIndex].get("thumbnailUrl", NULL);
		sImgUrl = jvImgValue.asString();
	}

	//通过将图片的下载url转为md5后当作文件名，确保图像的唯一性
	MD5 *md5FileName = new MD5(sImgUrl);
	string sFileName = md5FileName->toString() + ".img";
	delete md5FileName;

	if (!gImageDownload(sImgUrl, sFileName)) {
		//如果下载失败，则切换图片来源再次下载
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "objurl下载失败，尝试使用hoverUrl");
		jvImgValue = jvJsonValue["linkData"][nIndex].get("hoverUrl", NULL);
		if (jvImgValue == NULL) {
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "hoverUrl为空");
			return "";
		}
		sImgUrl = jvImgValue.asString();

		if (!gImageDownload(sImgUrl, sFileName)) {
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "hoverUrl下载失败");
			return "";
		}
	}

	return sFileName;
}

/*
 * 获取猫猫图片，返回图片文件名
 * API Pexels
*/
string CatPhoto::__GetPhotoPexels()
{
	int nPage = gRandomS(gnPexelsMaxPage - 1, "page");

	if (gvvsPexelsImageUrlList[nPage].size() == 0) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "正在缓存图像列表");

		char cPexelsUrl[64];
		srand((unsigned)time(NULL));

		sprintf_s(cPexelsUrl, "https://www.pexels.com/cats/?format=html&page=%d", nPage + 1);
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "获取网页源码");
		string sSearchString(gReadWebStr(cPexelsUrl));//取网页源码

		smatch smRegexResult;//正则匹配结果

		string::const_iterator iterStart = sSearchString.begin();
		string::const_iterator iterEnd = sSearchString.end();

		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "取出猫片清单");
		while (regex_search(iterStart, iterEnd, smRegexResult, grPexelsRegExp)) {
			string url = smRegexResult[1].str();
			gvvsPexelsImageUrlList[nPage].push_back(url.substr(0, url.find('?')));
			iterStart = smRegexResult[0].second;
		}

		if (gvvsPexelsImageUrlList[nPage].size() == 0) {
			//没找到任何图片
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "取猫片清单失败");
			return "";
		}

	}
	else {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "图像清单已缓存");
	}

	int nListSize = gvvsPexelsImageUrlList[nPage].size();

	int nIndex = gRandomS(nListSize - 1, "index");
	string sImageUrl = gvvsPexelsImageUrlList[nPage][nIndex];//随机获取一个图片url

	string sFileName = sImageUrl.substr(sImageUrl.find_last_of('/') + 1, sImageUrl.length() - 1);

	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "取图像清晰值");
	//取得w值 w值由用户自定义，代表了图像的清晰度，w值越高图像越清晰
	int nImageW = gReadConfig("image", "w", 0, "config.ini");
	if (nImageW == 0) {
		nImageW = 600;
		gWriteConfig("image", "w", nImageW, "config.ini");
	}

	sFileName = "w" + to_string(nImageW) + "-" + sFileName;
	string sDownloadPath = sImageUrl + "?" + "w=" + to_string(nImageW);

	if (!gImageDownload(sDownloadPath, sFileName)) {
		return "";
	}

	return sFileName;
}
