#ifndef _FS_UTILS_H
#define _FS_UTILS_H

#include <wut_types.h>

BOOL clearFolderWithBlackList(const char* path, const char* const* blacklist, int blacklist_size, int currentDepth, BOOL* outRemoveParent);
BOOL deleteFolder(const char* path);

#endif