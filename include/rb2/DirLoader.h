#ifndef _DIRLOADER_H
#define _DIRLOADER_H

#include "rb2/String.h"

typedef struct _DirLoader
{
    // Loader base stuff
    int *vtable;
    int unk;
    int loaderPos;
    String mFilePath;
    int unk3;

    // rest of DirLoader
} DirLoader;

extern void DirLoaderOpenFile(DirLoader *thisDirLoader);

#endif // _DIRLOADER_H