#pragma once
#include <Windows.h>
#include "app.h"

#define IDB_BUTTON_SAVE 1
#define IDB_EDIT_DISTINCT 2

LRESULT CALLBACK WinSunProc(
	HWND hwnd,  //handle to window 窗口的句柄
	UINT uMsg,   //message identifier 消息标识符
	WPARAM wParam,  //first message parameter 第一个消息参数
	LPARAM lParam  //second message parameter 第二个消息参数
);  //回调函数

void LoadWindows_MenuSet();
