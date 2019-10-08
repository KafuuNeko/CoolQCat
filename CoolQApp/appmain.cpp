#include "stdafx.h"
#include "appmain.h"
#include "win32-set.h"

using namespace std;

int gAuthCode = -1;
bool gEnabled = false;


CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}

CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	gAuthCode = AuthCode;
	return 0;
}

CQEVENT(int32_t, __eventStartup, 0)() {

	return 0;
}

CQEVENT(int32_t, __eventExit, 0)() {

	return 0;
}

CQEVENT(int32_t, __eventEnable, 0)() {
	gEnabled = true;
	return 0;
}

CQEVENT(int32_t, __eventDisable, 0)() {
	gEnabled = false;
	return 0;
}

CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t msgId, int64_t fromQQ, const char *msg, int32_t font) {
	AppDispose(PRIVATE_MSG, msg, fromQQ, 0);
	return EVENT_IGNORE;
}

CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t msgId, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font) {
	AppDispose(GROUP_MSG, msg, fromQQ, fromGroup);
	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}

CQEVENT(int32_t, __eventDiscussMsg, 32)(int32_t subType, int32_t msgId, int64_t fromDiscuss, int64_t fromQQ, const char *msg, int32_t font) {
	AppDispose(DISCUSS_MSG, msg, fromQQ, fromDiscuss);
	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}

CQEVENT(int32_t, __menuSet, 0)() {
	LoadWindows_MenuSet();
	return 0;
}
