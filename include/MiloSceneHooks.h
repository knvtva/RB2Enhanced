/*
    RB2Enhanced - MiloSceneHooks.h
    Hooks dealing with Milo scenes.
*/

#include "rb2/BinStream.h"
#include "rb2/DirLoader.h"
#include "rb2/Object.h"
#include "rb2/Vector.h"

// void DirLoaderOpenFileHook(DirLoader *thisDirLoader);
void LoadObj(Object *object, BinStream *stream);
void VertexReadHook(BinStream *thisBinStream, Vector3 *vec3);