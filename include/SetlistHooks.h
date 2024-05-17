/*
    RB2Enhanced - SetlistHooks.h
    Hooks for modifying the setlist/song selection screen.
*/

#include "rb2/BandLabel.h"
#include "rb2/SortNode.h"

void SetSongAndArtistNameHook(BandLabel *label, SortNode *sortNode);
void SetSongNameFromNodeHook(BandLabel *label, SortNode *sortNode);
