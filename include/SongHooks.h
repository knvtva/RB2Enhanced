/*
    RB2Enhanced - SongHooks.h
    Hooks related to loading songs into the game.
*/

#include "rb2/Data.h"

int MetadataSongIDHook(DataNode *song_id);
int GetSongIDHook(DataArray *song, DataArray *missing_data_maybe);
SongMetadata *InitSongMetadataHook(SongMetadata *data);
