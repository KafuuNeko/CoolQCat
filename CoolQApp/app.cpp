#include "stdafx.h"
#include "app.h"

using namespace std;

extern int gAuthCode;//AuthCode

//������ʽ
regex gPexelsRegExp("data-photo-modal-image-download-link=\'(.*?)\'");
regex gBingRegExp("src=\"(.*?)\"");

//����Դ�����ҳ��
int gPexelsMaxPage = 170, gBaiduMaxPage = 100, gBingMaxPage = 90;

//����Pexels�����ٶȽ��������Ի���ÿһ�η��ʺ�ȡ�õ��嵥
vector<vector<string>> gPexelsImageUrlList(gPexelsMaxPage);

//������
bool gDisposeLock = false;

void AppDispose(int8_t pType,const char *pMsg, int64_t pFromQQ, int64_t pFromGroup) {
	
	if (!strcmp(pMsg, "è")) {
		//����Ƿ����߳����ڴ���һ��ֻ����һ���������⿨��
		if (gDisposeLock) {
			CQSendMessage(pType, pFromQQ, pFromGroup, "�ϴε�èƬ����������~�����ĵȴ�");
			return;
		}

		gDisposeLock = true;//����������
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ָ��ƥ����ɣ����ڻ�ȡèͼ��");
		string fileName;

		switch (ReadConfig("Api", "s", 0, "config.ini")) {
		case 0:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ѡ��ͼƬ��Դ��Baidu");
			fileName = CatPhoto::GetPhotoBaidu();
			break;
		case 1:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ѡ��ͼƬ��Դ��Pexels");
			fileName = CatPhoto::GetPhotoPexels();
			break;
		case 2:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ѡ��ͼƬ��Դ��Bing");
			fileName = CatPhoto::GetPhotoBing();
			break;
		default:
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ѡ��ͼƬ��Դ��Baidu");
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
 * ��ȡèèͼƬ������ͼƬ�ļ���
 * API Bing
*/
string CatPhoto::GetPhotoBing() {
	int page = RandomS(gBingMaxPage, "random page");

	char bingUrl[256];
	sprintf_s(bingUrl
		, "https://cn.bing.com/images/async?q=%s&first=%d&count=5&relp=5&layout=RowBased_Landscape&mmasync=1"
		, "%E7%8C%AB%E7%8C%AB"
		, page * 10);

	string searchString(ReadWebStr(bingUrl));//ȡ��ҳԴ��

	string::const_iterator iterStart = searchString.begin();
	string::const_iterator iterEnd = searchString.end();


	smatch regexResult;
	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ȡ��èƬ�嵥");
	vector<string> imageList;
	while (regex_search(iterStart, iterEnd, regexResult, gBingRegExp)) {
		string url = regexResult[1].str();
		imageList.push_back(url.substr(0, url.find("&amp")));
		iterStart = regexResult[0].second;
	}

	int listSize = imageList.size();
	if (listSize == 0) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ͼƬ�嵥Ϊ��");
		return "";
	}

	int index = RandomS(listSize - 1, "index");

	//ͨ����ͼƬ������urlתΪmd5�����ļ�����ȷ��ͼ���Ψһ��
	MD5 *fileNameMD5 = new MD5(imageList[index]);
	string fileName = "bing_" + fileNameMD5->toString() + ".img";
	delete fileNameMD5;

	if (!ImageDownload(imageList[index], fileName)) {
		return "";
	}

	return fileName;
}

/*
 * ��ȡèèͼƬ������ͼƬ�ļ���
 * API Baidu
*/
string CatPhoto::GetPhotoBaidu() {
	int page = RandomS(gBaiduMaxPage, "random page");

	char baiduUrl[256];
	sprintf_s(baiduUrl
		, "http://m.baidu.com/sf/vsearch/image/search/wisesearchresult?tn=wisejsonala&ie=utf-8&fromsf=1&word=%s&pn=%d&rn=10&gsm=&searchtype=2&prefresh=undefined&from=link&type=2"
		, "%E7%8C%AB+%E5%8A%A8%E7%89%A9+%E5%8F%AF%E7%88%B1"
		, page * 10);
	string searchString(ReadWebStr(baiduUrl));//ȡ��ҳԴ��

	Json::Reader *reader = new Json::Reader(Json::Features::strictMode());
	Json::Value jsonValue;
	reader->parse(searchString, jsonValue);
	delete reader;

	if (jsonValue["linkData"].size() <= 0) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "linkDataΪ��");
		return "";
	}

	int index = RandomS(jsonValue["linkData"].size() - 1, "index");

	Json::Value jsonImgValue;

	//ȡ��objurl
	jsonImgValue = jsonValue["linkData"][index].get("objurl", NULL);
	if (jsonImgValue == NULL) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "objurlΪ��");
		return "";
	}

	string imgUrl = jsonImgValue.asString();
	if (imgUrl.find("tuzhan") != imgUrl.npos) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "��⵽ͼƬ��ԴΪtuzhan��ʹ��thumbnailUrl");
		jsonImgValue = jsonValue["linkData"][index].get("thumbnailUrl", NULL);
		imgUrl = jsonImgValue.asString();
	}

	//ͨ����ͼƬ������urlתΪmd5�����ļ�����ȷ��ͼ���Ψһ��
	MD5 *fileNameMd5 = new MD5(imgUrl);
	string fileName = fileNameMd5->toString() + ".img";
	delete fileNameMd5;

	if (!ImageDownload(imgUrl, fileName)) {
		//�������ʧ�ܣ����л�ͼƬ��Դ�ٴ�����
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "objurl����ʧ�ܣ�����ʹ��hoverUrl");
		jsonImgValue = jsonValue["linkData"][index].get("hoverUrl", NULL);
		if (jsonImgValue == NULL) {
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "hoverUrlΪ��");
			return "";
		}
		imgUrl = jsonImgValue.asString();

		if (!ImageDownload(imgUrl, fileName)) {
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "hoverUrl����ʧ��");
			return "";
		}
	}

	return fileName;
}

/*
 * ��ȡèèͼƬ������ͼƬ�ļ���
 * API Pexels
*/
string CatPhoto::GetPhotoPexels()
{
	int page = RandomS(gPexelsMaxPage, "random page");

	if (gPexelsImageUrlList[page].size() == 0) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "���ڻ���ͼ���б�");

		char pexelsUrl[64];
		srand((unsigned)time(NULL));

		sprintf_s(pexelsUrl, "https://www.pexels.com/cats/?format=html&page=%d", page + 1);
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "��ȡ��ҳԴ��");
		string searchString(ReadWebStr(pexelsUrl));//ȡ��ҳԴ��

		smatch regexResult;//����ƥ����

		string::const_iterator iterStart = searchString.begin();
		string::const_iterator iterEnd = searchString.end();

		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ȡ��èƬ�嵥");
		while (regex_search(iterStart, iterEnd, regexResult, gPexelsRegExp)) {
			string url = regexResult[1].str();
			gPexelsImageUrlList[page].push_back(url.substr(0, url.find('?')));
			iterStart = regexResult[0].second;
		}

		if (gPexelsImageUrlList[page].size() == 0) {
			//û�ҵ��κ�ͼƬ
			CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ȡèƬ�嵥ʧ��");
			return "";
		}

	}
	else {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ͼ���嵥�ѻ���");
	}

	int listSize = gPexelsImageUrlList[page].size();

	int index = RandomS(listSize - 1, "index");
	string imageUrl = gPexelsImageUrlList[page][index];//�����ȡһ��ͼƬurl

	string fileName = imageUrl.substr(imageUrl.find_last_of('/') + 1, imageUrl.length() - 1);

	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ȡͼ������ֵ");
	//ȡ��wֵ wֵ���û��Զ��壬������ͼ��������ȣ�wֵԽ��ͼ��Խ����
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
