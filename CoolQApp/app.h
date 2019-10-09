#pragma once

#include <vector>
#include <io.h>
#include <time.h>
#include <sstream>

#include "apphelper.h"
#include "Http.h"
#include "cqp.h"
#include "./md5/md5.h"
#include "./json/json.h"

using std::string;

void AppDispose(int8_t pType, const char *pMsg, int64_t pFromQQ, int64_t pFromGroup);

namespace CatPhoto {
	string GetPhotoPexels();
	string GetPhotoBaidu();
	string GetPhotoBing();
}
