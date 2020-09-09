#include "setting_generator.h"

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <whb/log.h>
#include <whb/log_console.h>
#include <iosuhax.h>

// from http://www.hacksden.com/downloads.php?do=file&id=135
static void scrypt(char *buf, BOOL is_enc)
{
	char *buf_ptr = buf;
	unsigned int key = 0x73B5DBFA;
	int i, len;
	
	if (is_enc)
	{
		len = 256;
	} else {
		len = strlen(buf);
	}
	
	for(i = 0; i < len; i++)
	{
		buf_ptr[i] ^= key & 0xff;
		key = (key << 1) | (key >> 31);
	}
}

BOOL regenerateSettingFile(int fsaFd, MCPSysProdSettings prod, Region region)
{
    if (region == REGION_UNSUPPORTED)
    {
        WHBLogPrintf("Error: Unsupported Region");
        WHBLogConsoleDraw();
        return FALSE;
    }

    // change the code to match the vWiis pattern
    prod.code_id[0] = prod.code_id[0] == 'G' ? 'I' : 'O';

    char* settings = calloc(1, 256);
    snprintf(settings, 256, 
        "AREA=%s\n"
        "MODEL=RVL-001(%s)\n"
        "DVD=0\n"
        "MPCH=0x7FFE\n"
        "CODE=%s\n"
        "SERNO=%s\n"
        "VIDEO=%s\n"
        "GAME=%s\n",
        settingRegionStrings[region],
        settingRegionStrings[region],
        prod.code_id,
        prod.serial_id,
        prod.ntsc_pal,
        gameRegionStrings[region]);

    // encrypt settings
    scrypt(settings, FALSE);

    FILE* f = fopen(SETTING_TXT_PATH, "wb");
    if (!f)
    {
        WHBLogPrintf("Error: Cannot create setting.txt file!");
        WHBLogConsoleDraw();
        return FALSE;
    }

    fwrite(settings, 1, 256, f);
    fclose(f);

    int ret = IOSUHAX_FSA_ChangeMode(fsaFd, "/vol/storage_slccmpt01/title/00000001/00000002/data/setting.txt", 0x666);
    if (ret < 0)
    {
        WHBLogPrintf("Warning: Cannot set setting.txt mode");
        WHBLogConsoleDraw();
    } 
    else
    {
        WHBLogPrintf("setting.txt chmod success");
        WHBLogConsoleDraw();
    }

    return TRUE;
}