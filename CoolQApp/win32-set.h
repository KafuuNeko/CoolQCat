#pragma once
#include <Windows.h>
#include "app.h"

#define IDB_BUTTON_SAVE 1
#define IDB_EDIT_DISTINCT 2

LRESULT CALLBACK WinSunProc(
	HWND hwnd,  //handle to window ���ڵľ��
	UINT uMsg,   //message identifier ��Ϣ��ʶ��
	WPARAM wParam,  //first message parameter ��һ����Ϣ����
	LPARAM lParam  //second message parameter �ڶ�����Ϣ����
);  //�ص�����

void LoadWindows_MenuSet();
