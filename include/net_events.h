/*
    RB2Enhanced - net_events.h
    Local network events API for sending out game state.
*/

#ifdef RB2E_XBOX
// MSVC notation
#define PACKSTART __pragma(pack(push, 1))
#define PACKEND __pragma(pack(pop))
#define PACKED
#else
// GCC notation
#define PACKSTART
#define PACKEND
#define PACKED __attribute__((packed))
#endif

// events API
// maximum packet size
#define RB2E_EVENTS_MAXPACKET 0xFF
// protocol version
#define RB2E_EVENTS_PROTOCOL 0
// protocol magic ('RB2E')
#define RB2E_EVENTS_MAGIC 0x52423345

// types of packets that can be sent via the local socket
typedef enum _RB2E_Events_PacketTypes
{
    RB2E_EVENT_ALIVE,          // content is a string of the RB2E_BUILDTAG value
    RB2E_EVENT_STATE,          // content is a char - 00=menus, 01=ingame
    RB2E_EVENT_SONG_NAME,      // content is a string of the current song name
    RB2E_EVENT_SONG_ARTIST,    // content is a string of the current song artist
    RB2E_EVENT_SONG_SHORTNAME, // content is a string of the current shortname
    RB2E_EVENT_SCORE,          // content is a RB2E_EventScore struct with score info
    RB2E_EVENT_STAGEKIT,       // content is a RB2E_EventStagekit struct with stagekit info
    RB2E_EVENT_BAND_INFO,      // content is a RB2E_EventBandInfo struct with band info
    RB2E_EVENT_VENUE_NAME,     // content is a string of the current venue name
    RB2E_EVENT_SCREEN_NAME,    // content is a string of the current screen name
    RB2E_EVENT_DX_DATA,        // content is a RB2E_EventModData struct with info given by DTA
} RB2E_Events_EventTypes;
// identifiers for each platform RB2E can or will run on
typedef enum _RB2E_Events_PlatformIDs
{
    RB2E_PLATFORM_XBOX,          // RB2E is running on a physical Xbox 360
    RB2E_PLATFORM_XENIA,         // RB2E is running on Xenia emulator
    RB2E_PLATFORM_WII,           // RB2E is running on a physical Wii
    RB2E_PLATFORM_DOLPHIN,       // RB2E is running on Dolphin emulator
    RB2E_PLATFORM_PS3,           // RB2E is running on a physical PS3
    RB2E_PLATFORM_RPCS3,         // RB2E is running on RPCS3 emulator
    RB2E_PLATFORM_UNKNOWN = 0xFF // just a nice placeholder
} RB2E_Events_PlatformIDs;

PACKSTART
// header for all local packets
typedef struct _RB2E_EventHeader
{
    int ProtocolMagic;
    char ProtocolVersion;
    char PacketType;
    char PacketSize;
    char Platform;
} PACKED RB2E_EventHeader;
// local socket packet
typedef struct _RB2E_EventPacket
{
    RB2E_EventHeader Header;
    char Data[RB2E_EVENTS_MAXPACKET];
} PACKED RB2E_EventPacket;
// packet that contains scoring information
typedef struct _RB2E_EventScore
{
    int TotalScore;
    int MemberScores[4];
    char Stars;
} PACKED RB2E_EventScore;
// packet that contains stagekit rumble data
typedef struct _RB2E_EventStagekit
{
    char LeftChannel;
    char RightChannel;
} PACKED RB2E_EventStagekit;
// packet that contains band member data
typedef struct _RB2E_EventBandInfo
{
    char MemberExists[4];
    char Difficulty[4];
    char TrackType[4];
} PACKED RB2E_EventBandInfo;
// packet meant for mods (rb2dx) to send out arbitrary information
typedef struct _RB2E_EventModData
{
    char IdentifyValue[10];
    char String[240];
} PACKED RB2E_EventModData;
PACKEND

// start up the events socket
void RB2E_InitEvents();
// send an event, maximum size is 0xFF
void RB2E_SendEvent(int type, void *packet, int size);
