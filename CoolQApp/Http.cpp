#include "stdafx.h"
#include "Http.h"


using namespace std;
//下载文件并保存为新文件名
bool gDownloadSaveFiles(char* pcUrl, char *pcSaveFile) {
	bool bResult = false;
	CInternetSession Sess("lpload");
	Sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 2000); //2秒的连接超时
	Sess.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 2000); //2秒的发送超时
	Sess.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 2000); //2秒的接收超时
	Sess.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 2000); //2秒的发送超时
	Sess.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 2000); //2秒的接收超时
	DWORD dwFlag = INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD;

	CHttpFile* cFile = NULL;
	char      *pBuf = NULL;
	int        nBufLen = 0;
	do {
		try {
			cFile = (CHttpFile*)Sess.OpenURL(pcUrl, 1, dwFlag);
			DWORD dwStatusCode;
			cFile->QueryInfoStatusCode(dwStatusCode);
			if (dwStatusCode == HTTP_STATUS_OK) {
				//查询文件长度
				DWORD nLen = 0;
				cFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, nLen);
				//CString strFilename = GetFileName(url,TRUE);
				nBufLen = nLen;
				if (nLen <= 0) break;//

				//分配接收数据缓存
				pBuf = (char*)malloc(nLen + 8);
				ZeroMemory(pBuf, nLen + 8);

				char *p = pBuf;
				while (nLen > 0) {
					//每次下载8K
					int n = cFile->Read(p, (nLen < RECVPACK_SIZE) ? nLen : RECVPACK_SIZE);
					//接收完成退出循环
					if (n <= 0) break;//
					//接收缓存后移
					p += n;
					//剩余长度递减
					nLen -= n;
				}

				//如果未接收完中断退出
				if (nLen != 0) break;

				//接收成功保存到文件

				CFile file(pcSaveFile, CFile::modeCreate | CFile::modeWrite);
				file.Write(pBuf, nBufLen);
				file.Close();
				bResult = true;
			}
		}
		catch (...) {
			break;//
		}
	} while (0);

	//释放缓存
	if (pBuf) {
		free(pBuf);
		pBuf = NULL;
		nBufLen = 0;
	}

	//关闭下载连接
	if (cFile) {
		cFile->Close();
		Sess.Close();
		delete cFile;
	}
	return bResult;
}

string gReadWebStr(const char *pcUrl) {
	CString csContent;

	CInternetSession ciSession("HttpClient");

	CHttpFile *pfile = (CHttpFile *)ciSession.OpenURL(pcUrl);

	DWORD dwStatusCode;
	pfile->QueryInfoStatusCode(dwStatusCode);
	if (dwStatusCode == HTTP_STATUS_OK)
	{
		CString data;


		while (pfile->ReadString(data))
		{
			csContent += data + "\r\n";
		}
		csContent.TrimRight();

	}
	pfile->Close();
	delete pfile;
	ciSession.Close();

	return string(csContent);
}
