#include "stdafx.h"
#include "Http.h"

using namespace std;
//�����ļ�������Ϊ���ļ���
bool DownloadSaveFiles(char* url, char *strSaveFile) {
	bool ret = false;
	CInternetSession Sess("lpload");
	Sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 2000); //2������ӳ�ʱ
	Sess.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 2000); //2��ķ��ͳ�ʱ
	Sess.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 2000); //2��Ľ��ճ�ʱ
	Sess.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 2000); //2��ķ��ͳ�ʱ
	Sess.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 2000); //2��Ľ��ճ�ʱ
	DWORD dwFlag = INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD;

	CHttpFile* cFile = NULL;
	char      *pBuf = NULL;
	int        nBufLen = 0;
	do {
		try {
			cFile = (CHttpFile*)Sess.OpenURL(url, 1, dwFlag);
			DWORD dwStatusCode;
			cFile->QueryInfoStatusCode(dwStatusCode);
			if (dwStatusCode == HTTP_STATUS_OK) {
				//��ѯ�ļ�����
				DWORD nLen = 0;
				cFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, nLen);
				//CString strFilename = GetFileName(url,TRUE);
				nBufLen = nLen;
				if (nLen <= 0) break;//

				//����������ݻ���
				pBuf = (char*)malloc(nLen + 8);
				ZeroMemory(pBuf, nLen + 8);

				char *p = pBuf;
				while (nLen > 0) {
					//ÿ������8K
					int n = cFile->Read(p, (nLen < RECVPACK_SIZE) ? nLen : RECVPACK_SIZE);
					//��������˳�ѭ��
					if (n <= 0) break;//
					//���ջ������
					p += n;
					//ʣ�೤�ȵݼ�
					nLen -= n;
				}

				//���δ�������ж��˳�
				if (nLen != 0) break;

				//���ճɹ����浽�ļ�

				CFile file(strSaveFile, CFile::modeCreate | CFile::modeWrite);
				file.Write(pBuf, nBufLen);
				file.Close();
				ret = true;
			}
		}
		catch (...) {
			break;//
		}
	} while (0);

	//�ͷŻ���
	if (pBuf) {
		free(pBuf);
		pBuf = NULL;
		nBufLen = 0;
	}

	//�ر���������
	if (cFile) {
		cFile->Close();
		Sess.Close();
		delete cFile;
	}
	return ret;
}

string ReadWebStr(const char *url) {
	CString content;

	CInternetSession session("HttpClient");

	CHttpFile *pfile = (CHttpFile *)session.OpenURL(url);

	DWORD dwStatusCode;
	pfile->QueryInfoStatusCode(dwStatusCode);
	if (dwStatusCode == HTTP_STATUS_OK)
	{
		CString data;


		while (pfile->ReadString(data))
		{
			content += data + "\r\n";
		}
		content.TrimRight();

	}
	pfile->Close();
	delete pfile;
	session.Close();

	return string(content);
}