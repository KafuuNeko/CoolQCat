#include "stdafx.h"
#include "app.h"

using namespace std;
extern int gAuthCode;//AuthCode
regex gPexelsRegExp("data-photo-modal-image-download-link=\'(.*?)\'");
bool gDisposeLock = false;

void AppDispose(int8_t pType,const char *pMsg, int64_t pFromQQ, int64_t pFromGroup) {
	if (!strcmp(pMsg, "è")) {
		//����Ƿ����߳����ڴ���һ��ֻ����һ���������⿨��
		if (gDisposeLock) {
			SendMessage(pType, pFromQQ, pFromGroup, "�ϴε�èƬ����������~�����ĵȴ�");
			return;
		}

		gDisposeLock = true;//����������
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ָ��ƥ����ɣ����ڻ�ȡèͼ��");
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
vector<vector<string>> pImageUrlList(pMaxPage);//ͼƬurl�嵥

string GetPhoto_Cat()
{
	ostringstream tempOstr;

	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ȡҳ��");

cmd_rand_page:
	int page = random(pMaxPage-1);
	
	tempOstr << "random page:" << page;
	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", tempOstr.str().c_str());
	tempOstr.clear();
	tempOstr.str("");


	if (page < 0 || page > 99) {
		delay_msec(100);//�ȴ�100ms����
		goto cmd_rand_page;
	}

	if (pImageUrlList[page].size() == 0) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "���ڻ���ͼ���б�");

		char pexelsUrl[64];
		srand((unsigned)time(NULL));
		
		sprintf_s(pexelsUrl, "https://www.pexels.com/cats/?format=html&page=%d", page+1);
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "��ȡ��ҳԴ��");
		string searchString(ReadWebStr(pexelsUrl));//ȡ��ҳԴ��

		smatch regexResult;//����ƥ����

		string::const_iterator iterStart = searchString.begin();
		string::const_iterator iterEnd = searchString.end();

		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ȡ��èƬ�嵥");
		while (regex_search(iterStart, iterEnd, regexResult, gPexelsRegExp)) {
			string url = regexResult[1].str();
			pImageUrlList[page].push_back(url.substr(0, url.find('?')));
			iterStart = regexResult[0].second;
		}

		if (pImageUrlList[page].size() == 0) {
			//û�ҵ��κ�ͼƬ
			return "";
		}

	}
	else {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ͼ���嵥�ѻ���");
	}
	
	int listSize = pImageUrlList[page].size();

	tempOstr << "ͼ���嵥��Ŀ:" << listSize;
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
		delay_msec(100);//�ȴ�100ms����
		goto cmd_index_rand;
	}

	string imageUrl = pImageUrlList[page][index];//�����ȡһ��ͼƬurl
	string fileName = imageUrl.substr(imageUrl.find_last_of('/') + 1, imageUrl.length() - 1);

	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "ȡͼ������ֵ");
	//ȡ��wֵ wֵ���û��Զ��壬������ͼ��������ȣ�wֵԽ��ͼ��Խ����
	int _w = ReadConfig("image", "w", 0, "config.ini");
	if (_w == 0) {
		_w = 600;
		WriteConfig("image", "w", _w, "config.ini");
	}

	tempOstr << "w" << _w << "-" << fileName;
	fileName = tempOstr.str();
	tempOstr.clear();
	tempOstr.str("");

	CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "��ȡ����·��");
	//ȡ�ñ���·��
	char savePath[256];
	sprintf_s(savePath, ".\\data\\image\\%s", fileName.c_str());

	if ((_access(savePath, 0)) == -1) {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "��ʼ����èƬ");
		//�ļ������ڣ�������èƬ
		//ȡ��ͼƬurl
		char downloadPath[256];
		sprintf_s(downloadPath, "%s?w=%d", imageUrl.c_str(), _w);

		//����ͼƬ
		if (!DownloadSaveFiles(downloadPath, savePath)) {
			return "";
		}
	}
	else {
		CQ_addLog(gAuthCode, CQLOG_DEBUG, "dispose", "��⵽èƬ�ѻ���");
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