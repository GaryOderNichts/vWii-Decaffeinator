#ifndef _SETTING_GENERATOR_H_
#define _SETTING_GENERATOR_H_

#include <wut_types.h>
#include <coreinit/mcp.h>

#include "main.h"

#define SETTING_TXT_PATH "dev:/title/00000001/00000002/data/setting.txt"

static const char* const settingRegionStrings[] = { "JPN", "USA", "EUR", "ERR" };
static const char* const gameRegionStrings[] = { "JP", "US", "EU", "XX" };

BOOL regenerateSettingFile(int fsaFd, MCPSysProdSettings prod, Region region);

#endif