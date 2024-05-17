/*
    RB2Enhanced - gocentral.c
    GoCentral connection hooks.
*/

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "ports.h"
#include "config.h"
#include "gocentral.h"
#include "quazal/InetAddress.h"

#ifdef RB2E_WII
int StartDNSLookupHook(void *wiiNetInit, char *domain)
{
    RB2E_DEBUG("WiiNetInit::StartDNSLookup %s", domain);
    if ((strcmp(domain + 10, ".hmxservices.com") == 0) && config.EnableGoCentral)
        domain = config.GoCentralAddress;
    return StartDNSLookup(wiiNetInit, domain);
}
#endif

void *SetAddressHook(void *thisInetAddress, char *addr)
{
    RB2E_DEBUG("InetAddress::SetAddress %s", addr);
#ifdef RB2E_XBOX
    // on xbox, detect our stub IP address and replace it with our direct connection IP address
    if (strcmp(addr, REDIRECT_IP_ADDRESS) == 0 && config.EnableLiveless)
        return SetAddress(thisInetAddress, config.DirectConnectIP);
#endif

    // check if the domain is Rock Central or the Quazal dummy address and return our GoCentral address
    if ((strcmp(addr + 10, ".hmxservices.com") == 0 || strcmp(addr, "DummySandboxAddress.quazal.com") == 0) && config.EnableGoCentral)
        return SetAddress(thisInetAddress, config.GoCentralAddress);

    return SetAddress(thisInetAddress, addr);
}
