/*
    RB2Enhanced - GameHooks.c
    Hooks for the Game state.
*/

#include <string.h>
#include <stdio.h>
#include "ports.h"
#include "net_events.h"
#include "rb2/Game.h"
#include "rb2/Symbol.h"
#include "rb2/SongMetadata.h"
#include "rb2/BandUser.h"
#include "rb2/BandUserMgr.h"
#include "rb2/BandSongMgr.h"
#include "rb2/MetaPerformer.h"
#include "rb2enhanced.h"

void *GameConstructHook(void *theGame) // You just lost
{
    char in_game = 0x01;
    int song_id;
    int i;
    SongMetadata *metadata;
    BandUser *bandUser;
    RB2E_EventBandInfo bandevent = {0};
#ifdef RB2E_XBOX
    Symbol song;
    GetSongShortname(&song, *(int *)PORT_THEMETAPERFORMER);
#else
    Symbol song = GetSongShortname(*(int *)PORT_THEMETAPERFORMER);
#endif
    if (song.sym != NULL)
    {
        RB2E_SendEvent(RB2E_EVENT_SONG_SHORTNAME, song.sym, strlen(song.sym));
        song_id = GetSongIDFromShortname((BandSongMgr *)PORT_THESONGMGR, song, 1);
        RB2E_DEBUG("GetSongIDFromShortname(%s) = %i", song.sym, song_id);
        metadata = GetMetadata((BandSongMgr *)PORT_THESONGMGR, song_id);
        if (metadata != NULL)
        {
            RB2E_DEBUG("Metadata: %p", metadata);
            RB2E_DEBUG("Started song: '%s' - %s (ID: %i, %s)", metadata->title.buf, metadata->artist.buf, metadata->song_id, metadata->shortname);
            RB2E_SendEvent(RB2E_EVENT_SONG_NAME, metadata->title.buf, metadata->title.length);
            RB2E_SendEvent(RB2E_EVENT_SONG_ARTIST, metadata->artist.buf, metadata->artist.length);
        }
    }
    for (i = 0; i < 4; i++)
    {
        bandUser = GetBandUserFromSlot(*(int *)PORT_THEBANDUSERMGR, i);
        if (bandUser != NULL)
        {
            RB2E_DEBUG("BandUser %i: %p - Track: %i, Controller: %i, Difficulty: %i", i, bandUser, bandUser->trackType, bandUser->controllerType, bandUser->difficulty);
            bandevent.MemberExists[i] = 1;
            bandevent.Difficulty[i] = bandUser->difficulty;
            bandevent.TrackType[i] = bandUser->trackType;
        }
        else
        {
            bandevent.MemberExists[i] = 0;
        }
    }
    RB2E_SendEvent(RB2E_EVENT_BAND_INFO, &bandevent, sizeof(bandevent));
    RB2E_SendEvent(RB2E_EVENT_STATE, &in_game, sizeof(in_game));
    return GameConstruct(theGame);
}

void *GameDestructHook(void *theGame, int r4)
{
    char in_game = 0x00;
    RB2E_SendEvent(RB2E_EVENT_STATE, &in_game, sizeof(in_game));
    return GameDestruct(theGame, r4);
}
