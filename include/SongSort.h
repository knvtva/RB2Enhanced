/*
    RB2Enhanced - SongSort.h
    Song sorting patches allowing for custom song sources.
*/

#include "rb2/Symbol.h"
#include "rb2/SongSortByRecentEntry.h"

SongSortByRecentEntry *SongSortByRecentHook(int *songSortByRecent, int *unk, char *songName, char *gameOrigin, int *unk2);
Symbol *GetSymbolByGameOriginHook(Symbol *sym, int gameOrigin);
int GetGameOriginBySymbolHook(Symbol sym);
