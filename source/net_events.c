/*
    RB2Enhanced - net_events.c
    Local network events API for sending out game state.
*/

#include <stddef.h>
#include <stdio.h>
#include "quazal/InetAddress.h"
#include "rb2enhanced.h"
#include "version.h"
#include "ports.h"
#include "config.h"
#include "net.h"
#include "net_events.h"

#define BROADCAST_IP 0xFFFFFFFF // 255.255.255.255, UDP multicast address
#define BROADCAST_PORT 0x524E   // 'RN' for uhhh rocknet or something

int RB2E_EventsSocket = -1;
static int InitFailed = 0;
static int EventsIP = BROADCAST_IP;
static short EventsPort = BROADCAST_PORT;

void RB2E_InitEvents()
{
    InetAddress address;
    // if event broadcasting is disabled, or if we failed before, don't bother doing anything
    if (config.EnableEvents == 0 || InitFailed)
        return;
    // parse the IP from the config file using Quazal
    if (config.BroadcastTarget[0] != '\0')
    {
        memset(&address, 0, sizeof(InetAddress));
        SetAddress(&address, config.BroadcastTarget);
        if (address.address != 0)
            EventsIP = address.address;
        if (address.port != 0)
            EventsPort = address.port;
    }
    RB2E_DEBUG("Events target: %08x:%i", EventsIP, EventsPort);
    // try to create the socket, but this might fail, if it does then don't try doing anything else
    RB2E_EventsSocket = RB2E_CreateSocket(RB2E_TYPE_UDP);
    // TODO(Emma): will the socket api ever return another error?
    if (RB2E_EventsSocket == -1)
    {
#ifdef RB2E_WII
        int wii_lasterror = RB2E_LastError();
        // check for SO_ENOENT/SO_ENOLINK and just fail out without setting failed
        // the game mightn't have initialised networking yet
        // TODO(Emma): is there any Xbox edge case similar to this?
        if (wii_lasterror == -48 || wii_lasterror == -45)
            return;
#endif
        InitFailed = 1;
        return;
    }
    // bind the local port and set it to be non-blocking
    RB2E_SetNonBlocking(RB2E_EventsSocket, 1);
    RB2E_BindPort(RB2E_EventsSocket, BROADCAST_PORT);
    // send an init message
    // TODO(Emma): put this in a loop and send it out every 10 seconds or so
    RB2E_SendEvent(RB2E_EVENT_ALIVE, RB2E_BUILDTAG, sizeof(RB2E_BUILDTAG));
}

void RB2E_SendEvent(int type, void *data, int size)
{
    RB2E_EventPacket packet;
    // dont bother trying to set stuff up if things are disabled
    if (config.EnableEvents == 0 || InitFailed)
        return;
    // if we don't have a socket, initialise one
    if (RB2E_EventsSocket == -1)
    {
        RB2E_InitEvents();
        if (RB2E_EventsSocket == -1)
            return;
    }
    // truncate packets bigger than this, to avoid buffer overflows and network congestion
    if (size > RB2E_EVENTS_MAXPACKET)
        size = RB2E_EVENTS_MAXPACKET;
    // sort out the packet we're sending
    packet.Header.ProtocolMagic = RB2E_EVENTS_MAGIC;
    packet.Header.ProtocolVersion = RB2E_EVENTS_PROTOCOL;
    packet.Header.PacketType = type;
    packet.Header.PacketSize = size;
#ifdef RB2E_XBOX
    packet.Header.Platform = RB2E_IsEmulator() ? RB2E_PLATFORM_XENIA : RB2E_PLATFORM_XBOX;
#elif RB2E_WII
    packet.Header.Platform = RB2E_IsEmulator() ? RB2E_PLATFORM_DOLPHIN : RB2E_PLATFORM_WII;
#else
    packet.Header.Platform = RB2E_PLATFORM_UNKNOWN;
#endif
    memcpy(packet.Data, data, size);
    // Deploy!
    RB2E_UDP_SendTo(RB2E_EventsSocket, EventsIP, EventsPort, &packet, sizeof(RB2E_EventHeader) + size);
}
