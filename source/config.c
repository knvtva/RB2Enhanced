/*
    RB2Enhanced - config.c
    Handles loading the rb2.ini configuration file to set plugin settings.
*/

#include <stdlib.h>
#include <string.h>
#ifdef RB2E_WII
#include <bslug.h>
#endif
#include "config.h"
#include "rb2enhanced.h"
#include "ports.h"
#include "inih.h"

RB2E_Config config;

// config given to the game by the loader
#define LAUNCHER_CONFIG_MAGIC 0x53443A44
int RB2E_Launcher_HasConfig;
char RB2E_Launcher_Config[0x1000];

#define CONFIG_FILENAME "rb2.ini"

void InitDefaultConfig()
{
    memset(&config, 0, sizeof(config));
    strcpy(config.RawfilesDir, "rb2");
#ifdef RB2E_WII
    // uncomment when GoCentral has a default instance that uses naswii auth
    // strcpy(config.NASServer, "naswii.rbenhanced.rocks");
#endif
    config.SongSpeedMultiplier = 1.0;
    config.TrackSpeedMultiplier = 1.0;
#ifdef RB2EDEBUG
    config.LogFileAccess = 1;
#endif
}

int HasLauncherConfig()
{
    return RB2E_Launcher_HasConfig == LAUNCHER_CONFIG_MAGIC;
}

static int INIHandler(void *user, const char *section, const char *name, const char *value)
{
    RB2E_DEBUG("%s - %s : %s", section == NULL ? "none" : section, name, value);
    if (strcmp(section, "General") == 0)
    {
        if (strcmp(name, "SongSpeedMultiplier") == 0)
            config.SongSpeedMultiplier = (float)atof(value);
        if (strcmp(name, "TrackSpeedMultiplier") == 0)
            config.TrackSpeedMultiplier = (float)atof(value);
        if (strcmp(name, "ForcedVenue") == 0 && strcmp(value, "false") != 0)
            strncpy(config.ForcedVenue, value, RB2E_MAX_CONFIG_LEN);
        if (strcmp(name, "GameOriginIcons") == 0)
            config.GameOriginIcons = RB2E_CONFIG_BOOL(value);
        if (strcmp(name, "LogFileAccess") == 0)
            config.LogFileAccess = RB2E_CONFIG_BOOL(value);
        if (strcmp(name, "UnlockClothing") == 0)
            config.UnlockClothing = RB2E_CONFIG_BOOL(value);
        if (strcmp(name, "LanguageOverride") == 0 && strlen(value) == RB2E_LANG_LEN)
            strncpy(config.LanguageOverride, value, RB2E_LANG_LEN);
        if (strcmp(name, "RawfilesDir") == 0 && !RB2E_CONFIG_FALSE(value))
            strncpy(config.RawfilesDir, value, RB2E_MAX_CONFIG_LEN);
        if (strcmp(name, "DisableRawfiles") == 0)
            config.DisableRawfiles = RB2E_CONFIG_BOOL(value);
        if (strcmp(name, "QuazalLogging") == 0)
            config.QuazalLogging = RB2E_CONFIG_BOOL(value);
    }
    if (strcmp(section, "Events") == 0)
    {
        if (strcmp(name, "EnableEvents") == 0)
            config.EnableEvents = RB2E_CONFIG_BOOL(value);
        if (strcmp(name, "BroadcastTarget") == 0)
            strncpy(config.BroadcastTarget, value, RB2E_MAX_DOMAIN);
    }
    if (strcmp(section, "GoCentral") == 0)
    {
        if (strcmp(name, "EnableGoCentral") == 0)
            config.EnableGoCentral = RB2E_CONFIG_BOOL(value);
        if (strcmp(name, "GoCentralAddress") == 0)
            strncpy(config.GoCentralAddress, value, RB2E_MAX_CONFIG_LEN);
    }
    if (strcmp(section, "HTTP") == 0)
    {
        if (strcmp(name, "EnableHTTPServer") == 0)
            config.EnableHTTPServer = RB2E_CONFIG_BOOL(value);
        if (strcmp(name, "AllowCORS") == 0)
            config.AllowCORS = RB2E_CONFIG_BOOL(value);
    }
    if (strcmp(section, "Network") == 0)
    {
        if (strcmp(name, "EnableNATPMP") == 0)
            config.EnableNATPMP = RB2E_CONFIG_BOOL(value);
        if (strcmp(name, "EnableUPnP") == 0)
            config.EnableUPnP = RB2E_CONFIG_BOOL(value);
    }
#ifdef RB2E_WII
    if (strcmp(section, "Wii") == 0)
    {
        if (strcmp(name, "NASServer") == 0)
            strncpy(config.NASServer, value, RB2E_MAX_DOMAIN);
        if (strcmp(name, "LegacySDMode") == 0)
            config.LegacySDMode = RB2E_CONFIG_BOOL(value);
    }
#elif RB2E_XBOX
    if (strcmp(section, "Xbox360") == 0)
    {
        if (strcmp(name, "EnableLiveless") == 0)
            config.EnableLiveless = RB2E_CONFIG_BOOL(value);
        if (strcmp(name, "LivelessAddress") == 0)
            strncpy(config.LivelessAddress, value, RB2E_MAX_DOMAIN);
        if (strcmp(name, "DirectConnectIP") == 0)
            strncpy(config.DirectConnectIP, value, RB2E_MAX_IP_LEN);
        if (strcmp(name, "ExternalIP") == 0)
            strncpy(config.ExternalIP, value, RB2E_MAX_IP_LEN);
        if (strcmp(name, "STUNServer") == 0)
            strncpy(config.STUNServer, value, RB2E_MAX_DOMAIN);
        if (strcmp(name, "STUNServerPort") == 0)
            config.STUNServerPort = atoi(value);
    }
#endif
    if (strcmp(section, "Graphics") == 0)
    {
#ifdef RB2E_XBOX
        if (strcmp(name, "RenderResX") == 0)
            config.RenderResX = atoi(value);
        if (strcmp(name, "RenderResY") == 0)
            config.RenderResY = atoi(value);
#endif
        if (strcmp(name, "DisablePostProcessing") == 0)
            config.DisablePostProcessing = RB2E_CONFIG_BOOL(value);
    }
#ifdef RB2EDEBUG
    if (strcmp(section, "Debug") == 0)
    {
        if (strcmp(name, "LogMemoryOverview") == 0)
            config.LogMemoryOverview = RB2E_CONFIG_BOOL(value);
    }
#endif
    return 1;
}

void LoadConfig()
{
    char buf[0x2000] = {0};
    int file = -1;
    int read = 0;
    char *filepath;
    // check if the launcher has provided a valid INI to the module
    if (HasLauncherConfig())
    {
        RB2E_MSG("Loading config from launcher...", NULL);
        RB2E_DEBUG("%i byte config provided by launcher at %p, parsing", strlen(RB2E_Launcher_Config), RB2E_Launcher_Config);
        strncpy(buf, RB2E_Launcher_Config, sizeof(RB2E_Launcher_Config));
    }
    // load from mounted drives like usual
    else
    {
        if (!RB2E_Mounted)
        {
            RB2E_MSG("No drives mounted, using default config.", NULL);
            return;
        }
        // get the file path of the config file
        filepath = RB2E_GetRawfilePath(CONFIG_FILENAME, 1);
        if (filepath == NULL)
        {
            RB2E_MSG("No config file found, using default config.", NULL);
            return;
        }
        // open the file, read all the contents, then close
        RB2E_MSG("Loading config from %s...", filepath);
        file = RB2E_OpenFile(filepath, 0);
        read = RB2E_ReadFile(file, 0, buf, sizeof(buf));
        RB2E_CloseFile(file);
        RB2E_DEBUG("Read %i bytes from config file, parsing", read);
    }
    // parse the INI file
    if (ini_parse_string(buf, INIHandler, NULL) < 0)
    {
        RB2E_MSG("Failed to load config file, using default config", NULL);
        // re-initialise default config
        // because some values might've been parsed previously
        InitDefaultConfig();
    }
    else
    {
        RB2E_MSG("Successfully loaded config!", NULL);
    }
}

#ifdef RB2E_WII
// export these two functions to the brainslug loader
// TODO: exporting these on 360?
BSLUG_EXPORT(RB2E_Launcher_HasConfig);
BSLUG_EXPORT(RB2E_Launcher_Config);
#endif
