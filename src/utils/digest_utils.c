#include "digest_utils.h"
#include <stdio.h>

BOOL writeDigest()
{
    FILE* digestFile = fopen(DIGEST_PATH, "wb");
    if (digestFile)
    {
        fwrite(digest_bin, 1, digest_size, digestFile);
        fclose(digestFile);
        return TRUE;
    }

    return FALSE;
}