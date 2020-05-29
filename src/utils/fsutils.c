#include "fsutils.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include <whb/log.h>
#include <whb/log_console.h>

BOOL clearFolderWithBlackList(const char* path, const char* const* blacklist, int blacklist_size, int currentDepth, BOOL* outRemoveParent)
{
    for (int i = 0; i < blacklist_size; i++)
    {
        if (strcmp(path, blacklist[i]) == 0)
        {
			if (outRemoveParent && *outRemoveParent)
				*outRemoveParent = FALSE;

            WHBLogPrintf("Blacklist hit %s", path);
            WHBLogConsoleDraw();
            return FALSE;
        }
    }

    DIR* dir = opendir(path);
	if (dir == NULL)
	{
		WHBLogPrintf("Cannot open %s", path);
        WHBLogConsoleDraw();
		return FALSE;
	}
		
	size_t path_len = strlen(path);

	BOOL removeParent = TRUE;
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) 
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		char* full_path = calloc(path_len + strlen(entry->d_name) + 1, sizeof(char));
		strcpy(full_path, path);
		strcat(full_path, "/");
		strcat(full_path, entry->d_name);

        BOOL blacklistHit = FALSE;
        for (int i = 0; i < blacklist_size; i++)
        {
            if (strcmp(full_path, blacklist[i]) == 0)
            {
                blacklistHit = TRUE;
                break;
            }
        }

        if (blacklistHit)
        {
            WHBLogPrintf("Blacklist hit %s", full_path);
            WHBLogConsoleDraw();
            free(full_path);

			if (outRemoveParent && *outRemoveParent)
				*outRemoveParent = FALSE;

			removeParent = FALSE;
            continue;
        }

		if (entry->d_type & DT_DIR)
		{
			clearFolderWithBlackList(full_path, blacklist, blacklist_size, currentDepth + 1, &removeParent);
		}
		else
		{
			if (remove(full_path) != 0)
			{
				WHBLogPrintf("Cannot remove %s", full_path);
                WHBLogConsoleDraw();
			}
            else
            {
                WHBLogPrintf("removed %s", full_path);
                WHBLogConsoleDraw();
            }          
		}
		free(full_path);
	}

	closedir(dir);

	if (!removeParent)
		if (outRemoveParent)
			*outRemoveParent = FALSE;	

	if (currentDepth > 0 && removeParent)
	{
		if (remove(path) == 0)
		{
            WHBLogPrintf("removed %s", path);
            WHBLogConsoleDraw();
		}
        else
        {
            WHBLogPrintf("Cannot remove %s", path);
            WHBLogConsoleDraw();
        }    
	}	

	return TRUE;
}

BOOL deleteFolder(const char* path)
{
    DIR* dir = opendir(path);
	if (dir == NULL)
	{
		WHBLogPrintf("Cannot open %s", path);
        WHBLogConsoleDraw();
		return FALSE;
	}
		
	size_t path_len = strlen(path);

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) 
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		char* full_path = calloc(path_len + strlen(entry->d_name) + 1, sizeof(char));
		strcpy(full_path, path);
		strcat(full_path, "/");
		strcat(full_path, entry->d_name);

		if (entry->d_type & DT_DIR)
		{
			deleteFolder(full_path);
		}
		else
		{
			if (remove(full_path) != 0)
			{
				WHBLogPrintf("Cannot remove %s", full_path);
                WHBLogConsoleDraw();
			}
            else
            {
                WHBLogPrintf("removed %s", full_path);
                WHBLogConsoleDraw();
            }          
		}
		free(full_path);
	}

	closedir(dir);

    if (remove(path) != 0)
    {
        WHBLogPrintf("Cannot remove %s", path);
        WHBLogConsoleDraw();
    }
    else
    {
        WHBLogPrintf("removed %s", path);
        WHBLogConsoleDraw();
    }    

	return TRUE;    
}