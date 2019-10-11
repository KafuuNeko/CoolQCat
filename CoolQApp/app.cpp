#include "stdafx.h"
#include "app.h"

#include "apphelper.h"
#include "Http.h"
#include "cqp.h"
#include "./md5/md5.h"
#include "./json/json.h"

using namespace std;

extern int gAuthCode;//AuthCode

//������ʽ
regex grPexelsRegExp("data-photo-modal-image-download-link=\'(.*?)\'");
regex grBingRegExp("src=\"(.*?)\"");

//����Դ�����ҳ��
int gnPexelsMaxPage = 170, gnBaiduMaxPage = 100, gnBingMaxPage = 90;

//����Pexels�����ٶȽ��������Ի���ÿһ�η��ʺ�ȡ�õ��嵥
vector<vector<string>> gvvsPexelsImageUrlList(gnPexelsMaxPage);

//������
bool gbDisposeLock = false;

namespace CatPhoto {
	string __GetPhotoPexels();
	string __GetPhotoBaidu();
	string __GetPhotoBing();
}

void gAppDispose(int8_t pnType,const char *pcMsg, int64_t pnFromQQ, int64_t pnFromGroup) {
	
	if (!strcmp(pcMsg, "è")) {
		//����Ƿ����߳����ڴ���һ��ֻ����һ���������⿨��
		if (gbDisposeLock) {
			gCQSendMessage(pnType, pnFromQQ, pnFromGroup, "�ϴε�èƬ����������~�����ĵȴ�");
			return;
		}

		gbDisposeLock = true;//����������
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ָ��ƥ����ɣ����ڻ�ȡèͼ��");
		string sFileName;

		switch (gReadConfig("Api", "s", 0, "config.ini")) {
		case 0:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ѡ��ͼƬ��Դ��Baidu");
			sFileName = CatPhoto::__GetPhotoBaidu();
			break;
		case 1:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ѡ��ͼƬ��Դ��Pexels");
			sFileName = CatPhoto::__GetPhotoPexels();
			break;
		case 2:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ѡ��ͼƬ��Դ��Bing");
			sFileName = CatPhoto::__GetPhotoBing();
			break;
		default:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ѡ��ͼƬ��Դ��Baidu");
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
 * ��ȡèèͼƬ������ͼƬ�ļ���
 * API Bing
*/
string CatPhoto::__GetPhotoBing() {
	int nPage = gRandomS(gnBingMaxPage, "page");

	char cBingUrl[256];
	sprintf_s(cBingUrl
		, "https://cn.bing.com/images/async?q=%s&first=%d&count=5&relp=5&layout=RowBased_Landscape&mmasync=1"
		, "%E7%8C%AB%E7%8C%AB"
		, nPage * 10);

	string searchString(gReadWebStr(cBingUrl));//ȡ��ҳԴ��

	string::const_iterator iterStart = searchString.begin();
	string::const_iterator iterEnd = searchString.end();


	smatch smRegexResult;
	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ȡ��èƬ�嵥");
	vector<string> vsImageList;
	while (regex_search(iterStart, iterEnd, smRegexResult, grBingRegExp)) {
		string url = smRegexResult[1].str();
		vsImageList.push_back(url.substr(0, url.find("&amp")));
		iterStart = smRegexResult[0].second;
	}

	int nListSize = vsImageList.size();
	if (nListSize == 0) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ͼƬ�嵥Ϊ��");
		return "";
	}

	int nIndex = gRandomS(nListSize - 1, "index");

	//ͨ����ͼƬ������urlתΪmd5�����ļ�����ȷ��ͼ���Ψһ��
	MD5 *md5FileName = new MD5(vsImageList[nIndex]);
	string sFileName = "bing_" + md5FileName->toString() + ".img";
	delete md5FileName;

	if (!gImageDownload(vsImageList[nIndex], sFileName)) {
		return "";
	}

	return sFileName;
}

/*
 * ��ȡèèͼƬ������ͼƬ�ļ���
 * API Baidu
*/
string CatPhoto::__GetPhotoBaidu() {
	int nPage = gRandomS(gnBaiduMaxPage, "page");

	char cBaiduUrl[256];
	sprintf_s(cBaiduUrl
		, "http://m.baidu.com/sf/vsearch/image/search/wisesearchresult?tn=wisejsonala&ie=utf-8&fromsf=1&word=%s&pn=%d&rn=10&gsm=&searchtype=2&prefresh=undefined&from=link&type=2"
		, "%E7%8C%AB+%E5%8A%A8%E7%89%A9+%E5%8F%AF%E7%88%B1"
		, nPage * 10);
	string sSearchString(gReadWebStr(cBaiduUrl));//ȡ��ҳԴ��

	Json::Reader *jrReader = new Json::Reader(Json::Features::strictMode());
	Json::Value jvJsonValue;
	jrReader->parse(sSearchString, jvJsonValue);
	delete jrReader;

	if (jvJsonValue["linkData"].size() <= 0) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "linkDataΪ��");
		return "";
	}

	int nIndex = gRandomS(jvJsonValue["linkData"].size() - 1, "index");

	Json::Value jvImgValue;

	//ȡ��objurl
	jvImgValue = jvJsonValue["linkData"][nIndex].get("objurl", NULL);
	if (jvImgValue == NULL) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "objurlΪ��");
		return "";
	}

	string sImgUrl = jvImgValue.asString();
	if (sImgUrl.find("tuzhan") != sImgUrl.npos) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "��⵽ͼƬ��ԴΪtuzhan��ʹ��thumbnailUrl");
		jvImgValue = jvJsonValue["linkData"][nIndex].get("thumbnailUrl", NULL);
		sImgUrl = jvImgValue.asString();
	}

	//ͨ����ͼƬ������urlתΪmd5�����ļ�����ȷ��ͼ���Ψһ��
	MD5 *md5FileName = new MD5(sImgUrl);
	string sFileName = md5FileName->toString() + ".img";
	delete md5FileName;

	if (!gImageDownload(sImgUrl, sFileName)) {
		//�������ʧ�ܣ����л�ͼƬ��Դ�ٴ�����
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "objurl����ʧ�ܣ�����ʹ��hoverUrl");
		jvImgValue = jvJsonValue["linkData"][nIndex].get("hoverUrl", NULL);
		if (jvImgValue == NULL) {
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "hoverUrlΪ��");
			return "";
		}
		sImgUrl = jvImgValue.asString();

		if (!gImageDownload(sImgUrl, sFileName)) {
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "hoverUrl����ʧ��");
			return "";
		}
	}

	return sFileName;
}

/*
 * ��ȡèèͼƬ������ͼƬ�ļ���
 * API Pexels
*/
string CatPhoto::__GetPhotoPexels()
{
	int nPage = gRandomS(gnPexelsMaxPage - 1, "page");

	if (gvvsPexelsImageUrlList[nPage].size() == 0) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "���ڻ���ͼ���б�");

		char cPexelsUrl[64];
		srand((unsigned)time(NULL));

		sprintf_s(cPexelsUrl, "https://www.pexels.com/cats/?format=html&page=%d", nPage + 1);
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "��ȡ��ҳԴ��");
		string sSearchString(gReadWebStr(cPexelsUrl));//ȡ��ҳԴ��

		smatch smRegexResult;//����ƥ����

		string::const_iterator iterStart = sSearchString.begin();
		string::const_iterator iterEnd = sSearchString.end();

		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ȡ��èƬ�嵥");
		while (regex_search(iterStart, iterEnd, smRegexResult, grPexelsRegExp)) {
			string url = smRegexResult[1].str();
			gvvsPexelsImageUrlList[nPage].push_back(url.substr(0, url.find('?')));
			iterStart = smRegexResult[0].second;
		}

		if (gvvsPexelsImageUrlList[nPage].size() == 0) {
			//û�ҵ��κ�ͼƬ
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ȡèƬ�嵥ʧ��");
			return "";
		}

	}
	else {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ͼ���嵥�ѻ���");
	}

	int nListSize = gvvsPexelsImageUrlList[nPage].size();

	int nIndex = gRandomS(nListSize - 1, "index");
	string sImageUrl = gvvsPexelsImageUrlList[nPage][nIndex];//�����ȡһ��ͼƬurl

	string sFileName = sImageUrl.substr(sImageUrl.find_last_of('/') + 1, sImageUrl.length() - 1);

	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ȡͼ������ֵ");
	//ȡ��wֵ wֵ���û��Զ��壬������ͼ��������ȣ�wֵԽ��ͼ��Խ����
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
