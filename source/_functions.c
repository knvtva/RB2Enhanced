/*
    RB2Enhanced - _functions.c
    Definitions used by the compiler to call/hook functions.
    This file should not be included by any other file, to avoid conflicts.
*/

#include "ports.h"
#include "ppcasm.h"
// get line number
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
// define small stub using line number to avoid optimisations
#ifdef RB2E_XBOX // for xbox, use MSVC notation
#define RB2E_STUB(x)           \
    __declspec(naked) void x() \
    {                          \
        __asm { li r3, __LINE__ }     \
    }
#else // other platforms should use GCC notation
#define RB2E_STUB(x)                   \
    void x()                           \
    {                                  \
        asm("li 0, " LINE_STRING ";"); \
    }
#endif

// function stub definitions
#ifndef RB2E_WII
RB2E_STUB(AppConstructor) // AppConstructor is handled by the BrainSlug engine
#endif
RB2E_STUB(ExecuteDTA)
RB2E_STUB(SymbolConstruct)
RB2E_STUB(ModifierActive)
RB2E_STUB(HmxFactoryFuncAt)
RB2E_STUB(BandLabelSetDisplayText)
RB2E_STUB(RandomInt)
RB2E_STUB(DataNodeEvaluate)
RB2E_STUB(DataFindArray)
RB2E_STUB(DataFindData)
RB2E_STUB(FileExists)
RB2E_STUB(QueueMessage)
RB2E_STUB(GetMetadata)
RB2E_STUB(GetSongIDFromShortname)
RB2E_STUB(GetSongShortname)
RB2E_STUB(GetBandUsers)
RB2E_STUB(GetBandUserFromSlot)
RB2E_STUB(ChunkStreamConstructor)
RB2E_STUB(FileStreamConstructor)
RB2E_STUB(Dynamic_Cast)
RB2E_STUB(GameGetActivePlayer)
RB2E_STUB(PropAnimSetFrame)
RB2E_STUB(ObjectFindUIPanel)
RB2E_STUB(JoypadGetPadData)
RB2E_STUB(SongMgrGetRankedSongs)
RB2E_STUB(InitSongMetadata)
RB2E_STUB(MemPrintOverview)
RB2E_STUB(MemPrint)
RB2E_STUB(MemNumHeaps)
RB2E_STUB(MemAlloc)
RB2E_STUB(MemFree)
// hooked function stubs
RB2E_STUB(Localize)
RB2E_STUB(SetVenue)
RB2E_STUB(ModifierManagerConstructor)
RB2E_STUB(NewFile)
RB2E_STUB(WillBeNoStrum)
RB2E_STUB(AddGameGem)
RB2E_STUB(SetSongAndArtistName)
RB2E_STUB(SetMusicSpeed)
RB2E_STUB(UpdateTrackSpeed)
RB2E_STUB(SetAddress)
RB2E_STUB(GetWidgetByName)
RB2E_STUB(GetSlotColor)
RB2E_STUB(UsbWiiGetType)
RB2E_STUB(StartDNSLookup)
RB2E_STUB(SetSystemLanguage)
RB2E_STUB(DataReadFile)
RB2E_STUB(StagekitSetState)
RB2E_STUB(GameConstruct)
RB2E_STUB(GameDestruct)
RB2E_STUB(SetSongNameFromNode)
RB2E_STUB(PrepareSomeVectorMaybe)
RB2E_STUB(SomeVectorPushBackMaybe)
RB2E_STUB(MusicLibrarySelect)
RB2E_STUB(SongSortByRecent)
RB2E_STUB(GetSymbolByGameOrigin)
RB2E_STUB(GetGameOriginBySymbol)
RB2E_STUB(JoypadGetCachedXInputCaps)
RB2E_STUB(SymbolPreInit)
RB2E_STUB(QueuingSocketBind)
RB2E_STUB(OperatorEqualsFmt)
RB2E_STUB(UpdatePresence)
RB2E_STUB(StepSequenceJobSetStep)
RB2E_STUB(BinstreamWrite)
RB2E_STUB(BinstreamRead)
RB2E_STUB(BinstreamReadEndian)
RB2E_STUB(BinstreamWriteEndian)