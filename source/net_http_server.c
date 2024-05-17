/*
    RB2Enhanced - net_http_server.c
    Hacky HTTP server for interacting with the game over the local network.
*/

#ifdef RB2E_XBOX

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "quazal/InetAddress.h"
#include "rb2/SongMetadata.h"
#include "rb2/BandSongMgr.h"
#include "rb2enhanced.h"
#include "version.h"
#include "ports.h"
#include "config.h"
#include "MusicLibrary.h"
#include "net.h"
#include "net_events.h"

#define SERVER_PORT 0x524E // 'RN' for uhhh rocknet or something

int RB2E_HTTPSocket = -1;
static int InitFailed = 0;
static char *PendingShortname = NULL;

typedef struct _song_list_vector
{
    int *start;
    int *end;
    int unk1;
} song_list_vector;
static song_list_vector song_list = {0};

void HTTP_Server_Init()
{
    RB2E_HTTPSocket = RB2E_CreateSocket(RB2E_TYPE_TCP);
    // TODO(Emma): will the socket api ever return another error?
    if (RB2E_HTTPSocket == -1)
    {
        InitFailed = 1;
        return;
    }
    // bind the local port and set it to be non-blocking
    RB2E_SetNonBlocking(RB2E_HTTPSocket, 1);
    RB2E_BindPort(RB2E_HTTPSocket, SERVER_PORT);
    // start listening for new connections
    RB2E_TCP_Listen(RB2E_HTTPSocket);
    RB2E_MSG("HTTP server running!", NULL);
}

typedef enum _HTTP_Request_Status
{
    HTTP_REQUEST_METHOD,
    HTTP_REQUEST_PATH,
    HTTP_REQUEST_VERSION,
    HTTP_REQUEST_HEADER,
    HTTP_REQUEST_DONE
} HTTP_Request_Status;

void HTTP_Server_Accept(void *connection)
{
    int s = (int)connection;
    int conn_time_passed = 0;
    char work_buffer[512] = {0};
    char *work_ptr = work_buffer;
    int req_state = HTTP_REQUEST_METHOD;
    int state_read_bytes = 0;
    int read_bytes = 0;
    char request_path[250] = {0};
    int request_valid = 0;
    char last_byte = '\0';
    const char ok_response[] = "HTTP/1.1 200 OK\r\nServer: RB2Enhanced " RB2E_BUILDTAG "\r\nContent-Type: text/html\r\nX-Clacks-Overhead: GNU maxton\r\nContent-Length: 2\r\n\r\nOK";
    const char notfound_response[] = "HTTP/1.1 404 Not Found\r\nServer: RB2Enhanced " RB2E_BUILDTAG "\r\nX-Clacks-Overhead: GNU maxton\r\nContent-Length: 9\r\n\r\nNot Found";
    char response_buffer[2000] = {0};
    int song_id = 0;
    char *shortname = NULL;
    SongMetadata *song_metadata = NULL;
    int list_count = 0;
    int i = 0;
    char *file_path = NULL;
    int file_r = 0;
    int file_pointer = 0;
    int file_fd = 0;

    // TODO(Emma): Properly evaluate the security (and stability!) of this.
    while (req_state < HTTP_REQUEST_DONE)
    {
        if (RB2E_TCP_Recv(s, work_ptr, 1) != -1)
        {
            last_byte = *work_ptr;
            // if we've read more than we can chew, back out immediately
            if (state_read_bytes > sizeof(work_buffer))
                goto close_connection;
            // advance the buffers forward
            work_ptr++;
            read_bytes++;
            state_read_bytes++;
        }
        // check if we had an error that wasn't due to blocking, or if the request took too long
        else if (RB2E_LastError() != 10035 || conn_time_passed > 1000)
        {
            goto close_connection;
        }
        // sleep the thread and count up
        else
        {
            RB2E_Sleep(5);
            conn_time_passed += 5;
            continue;
        }
        if (req_state == HTTP_REQUEST_METHOD && last_byte == ' ')
        {
            if (strcmp(work_buffer, "GET ") == 0)
            {
                req_state = HTTP_REQUEST_PATH;
                memset(work_buffer, 0, sizeof(work_buffer));
                work_ptr = work_buffer;
                state_read_bytes = 0;
            }
            else
            {
                RB2E_DEBUG("Got unknown request method '%s', closing connection", work_buffer);
                goto close_connection;
            }
        }
        else if (req_state == HTTP_REQUEST_PATH && last_byte == ' ')
        {
            // null us off
            work_ptr[-1] = '\0';
            strncpy(request_path, work_buffer, sizeof(request_path));
            req_state = HTTP_REQUEST_VERSION;
            memset(work_buffer, 0, sizeof(work_buffer));
            work_ptr = work_buffer;
            state_read_bytes = 0;
        }
        else if (req_state == HTTP_REQUEST_VERSION && memcmp(work_ptr - 2, "\r\n", 2) == 0)
        {
            req_state = HTTP_REQUEST_HEADER;
            memset(work_buffer, 0, sizeof(work_buffer));
            work_ptr = work_buffer;
            state_read_bytes = 0;
        }
        else if (req_state == HTTP_REQUEST_HEADER && memcmp(work_ptr - 2, "\r\n", 2) == 0)
        {
            // TODO(Emma): Validate Origin header
            work_ptr[-1] = '\0';
            work_ptr[-2] = '\0';
            if (state_read_bytes > 2)
            {
                RB2E_DEBUG("Got HTTP header '%s'", work_buffer);
                req_state = HTTP_REQUEST_HEADER;
            }
            else
                req_state = HTTP_REQUEST_DONE;
            memset(work_buffer, 0, sizeof(work_buffer));
            work_ptr = work_buffer;
            state_read_bytes = 0;
        }
    }
    RB2E_DEBUG("Got a request for '%s' over HTTP", request_path);
    if (strstr(request_path, "/song_") == request_path)
    {
        if (sscanf(request_path + sizeof("/song"), "%i", &song_id) == 1)
        {
            RB2E_DEBUG("Fetching song metadata for %i over HTTP", song_id);
            song_metadata = GetMetadata((BandSongMgr *)PORT_THESONGMGR, song_id);
            if (song_metadata == NULL)
            {
                RB2E_TCP_Send(s, (void *)notfound_response, sizeof(notfound_response));
                goto close_connection;
            }
            // TODO(Emma): Good fucking christ what is this
            strcat(response_buffer, "HTTP/1.1 200 OK\r\n");
            strcat(response_buffer, "Server: RB2Enhanced " RB2E_BUILDTAG "\r\n");
            if (config.AllowCORS)
                strcat(response_buffer, "Access-Control-Allow-Origin: *\r\n");
            strcat(response_buffer, "Content-Type: text/plain\r\n");
            strcat(response_buffer, "\r\n");
            strcat(response_buffer, "shortname=");
            strcat(response_buffer, song_metadata->shortname);
            strcat(response_buffer, "\r\n");
            strcat(response_buffer, "title=");
            strcat(response_buffer, song_metadata->title.buf);
            strcat(response_buffer, "\r\n");
            strcat(response_buffer, "artist=");
            strcat(response_buffer, song_metadata->artist.buf);
            strcat(response_buffer, "\r\n");
            strcat(response_buffer, "album=");
            strcat(response_buffer, song_metadata->album.buf);
            strcat(response_buffer, "\r\n");
            strcat(response_buffer, "origin=");
            strcat(response_buffer, song_metadata->gameOrigin);
            strcat(response_buffer, "\r\n");
            strcat(response_buffer, "\r\n");
            RB2E_TCP_Send(s, (void *)response_buffer, strlen(response_buffer));
            goto close_connection;
        }
    }
    else if (strstr(request_path, "/jump?shortname=") == request_path)
    {
        // TODO(Emma): This will crash if you're not in the music library. Figure out if we can detect what screen we're on
        shortname = request_path + 16;
        RB2E_DEBUG("Jumping to song %s in Music Library", shortname);
        // hacky solution to send the shortname to the main thread
        PendingShortname = shortname;
        while (PendingShortname != NULL)
            RB2E_Sleep(1);
        RB2E_TCP_Send(s, (void *)ok_response, strlen(ok_response));
        goto close_connection;
    }
    else if (strcmp(request_path, "/list_songs") == 0)
    {
        if (song_list.start == NULL)
            SongMgrGetRankedSongs((BandSongMgr *)PORT_THESONGMGR, &song_list, 0, 0);
        list_count = ((int)song_list.end - (int)song_list.start) / 4;
        // TODO(Emma): ...This, also, is not very great.
        strcat(response_buffer, "HTTP/1.1 200 OK\r\n");
        strcat(response_buffer, "Server: RB2Enhanced " RB2E_BUILDTAG "\r\n");
        strcat(response_buffer, "Content-Type: text/plain\r\n");
        if (config.AllowCORS)
            strcat(response_buffer, "Access-Control-Allow-Origin: *\r\n");
        strcat(response_buffer, "\r\n");
        // flush our buffer and send each song as its own packet
        RB2E_TCP_Send(s, (void *)response_buffer, strlen(response_buffer));
        memset(response_buffer, 0, sizeof(response_buffer));
        for (i = 0; i < list_count; i++)
        {
            song_id = song_list.start[i];
            song_metadata = GetMetadata((BandSongMgr *)PORT_THESONGMGR, song_id);
            if (song_metadata != NULL)
            {
                strcat(response_buffer, "[");
                strcat(response_buffer, song_metadata->shortname);
                strcat(response_buffer, "]\r\n");
                strcat(response_buffer, "shortname=");
                strcat(response_buffer, song_metadata->shortname);
                strcat(response_buffer, "\r\n");
                strcat(response_buffer, "title=");
                strcat(response_buffer, song_metadata->title.buf);
                strcat(response_buffer, "\r\n");
                strcat(response_buffer, "artist=");
                strcat(response_buffer, song_metadata->artist.buf);
                strcat(response_buffer, "\r\n");
                strcat(response_buffer, "album=");
                strcat(response_buffer, song_metadata->album.buf);
                strcat(response_buffer, "\r\n");
                strcat(response_buffer, "origin=");
                strcat(response_buffer, song_metadata->gameOrigin);
                strcat(response_buffer, "\r\n");
                strcat(response_buffer, "\r\n");
                if (strlen(response_buffer) > 1500)
                {
                    RB2E_TCP_Send(s, (void *)response_buffer, strlen(response_buffer));
                    memset(response_buffer, 0, sizeof(response_buffer));
                    // attempt to somewhat stagger the packets out
                    RB2E_Sleep(1);
                }
            }
        }
        if (strlen(response_buffer) > 0)
        {
            RB2E_TCP_Send(s, (void *)response_buffer, strlen(response_buffer));
        }
        goto close_connection;
    }
    else if (strcmp(request_path, "/") == 0)
    {
        file_path = RB2E_GetRawfilePath("rb2e_index.html", 1);
        if (file_path == NULL)
        {
            RB2E_TCP_Send(s, (void *)notfound_response, sizeof(notfound_response));
            goto close_connection;
        }
        file_fd = RB2E_OpenFile(file_path, 0);
        strcat(response_buffer, "HTTP/1.1 200 OK\r\n");
        strcat(response_buffer, "Server: RB2Enhanced " RB2E_BUILDTAG "\r\n");
        strcat(response_buffer, "Content-Type: text/html\r\n");
        strcat(response_buffer, "X-Clacks-Overhead: GNU maxton\r\n");
        if (config.AllowCORS)
            strcat(response_buffer, "Access-Control-Allow-Origin: *\r\n");
        strcat(response_buffer, "\r\n");
        // flush our buffer and send out the file in split chunks
        RB2E_TCP_Send(s, (void *)response_buffer, strlen(response_buffer));
        memset(response_buffer, 0, sizeof(response_buffer));
        while ((file_r = RB2E_ReadFile(file_fd, file_pointer, response_buffer, sizeof(response_buffer) - 1) > 0))
        {
            // fix an issue where a bunch of 0xFFs get added by the xbonx fs API
            work_ptr = strstr(response_buffer, "\xFF");
            if (work_ptr != NULL)
                work_ptr[0] = 0x00;
            RB2E_TCP_Send(s, (void *)response_buffer, strlen(response_buffer));
            file_pointer += strlen(response_buffer);
            memset(response_buffer, 0, sizeof(response_buffer));
            // attempt to somewhat stagger the packets out
            RB2E_Sleep(1);
        }
        goto close_connection;
    }
    else if (strcmp(request_path, "/jsonrpc") == 0)
    {
        file_path = RB2E_GetRawfilePath("discordrp.json", 1);
        if (file_path == NULL)
        {
            RB2E_TCP_Send(s, (void *)notfound_response, sizeof(notfound_response));
            goto close_connection;
        }
        file_fd = RB2E_OpenFile(file_path, 0);
        strcat(response_buffer, "HTTP/1.1 200 OK\r\n");
        strcat(response_buffer, "Server: RB2Enhanced " RB2E_BUILDTAG "\r\n");
        strcat(response_buffer, "Content-Type: application/json\r\n");
        strcat(response_buffer, "X-Clacks-Overhead: GNU maxton\r\n");
        if (config.AllowCORS)
            strcat(response_buffer, "Access-Control-Allow-Origin: *\r\n");
        strcat(response_buffer, "\r\n");
        // flush our buffer and send out the file in split chunks
        RB2E_TCP_Send(s, (void *)response_buffer, strlen(response_buffer));
        memset(response_buffer, 0, sizeof(response_buffer));
        while ((file_r = RB2E_ReadFile(file_fd, file_pointer, response_buffer, sizeof(response_buffer) - 1) > 0))
        {
            // fix an issue where a bunch of 0xFFs get added by the xbonx fs API
            work_ptr = strstr(response_buffer, "\xFF");
            if (work_ptr != NULL)
                work_ptr[0] = 0x00;
            RB2E_TCP_Send(s, (void *)response_buffer, strlen(response_buffer));
            file_pointer += strlen(response_buffer);
            memset(response_buffer, 0, sizeof(response_buffer));
            // attempt to somewhat stagger the packets out
            RB2E_Sleep(1);
        }
        RB2E_CloseFile(file_fd);
        goto close_connection;
    }
    else
    {
        // endpoint not found
        RB2E_TCP_Send(s, (void *)notfound_response, sizeof(notfound_response));
    }
close_connection:
    RB2E_DisposeSocket(s);
}

void HTTP_Server_RunLoop()
{
    int r = 0;
    unsigned int connected_ip = 0;
    unsigned short connected_port = 0;
    // init the HTTP socket if we haven't already
    if (RB2E_HTTPSocket == -1)
    {
        HTTP_Server_Init();
        if (InitFailed)
            return;
    }
    // check if we have a shortname to fuck with
    if (PendingShortname != NULL)
    {
        MusicLibrarySelectSong(PendingShortname);
        PendingShortname = NULL;
    }
    // accept incoming connections
    // TODO(Emma): add some sort of subnet restriction and ratelimit
    r = RB2E_TCP_Accept(RB2E_HTTPSocket, &connected_ip, &connected_port);
    if (r != -1)
    {
        RB2E_MSG("New HTTP connection from %08X:%i", connected_ip, connected_port);
        RB2E_CreateThread(HTTP_Server_Accept, (void *)r, 0x1000);
    }
}

#endif
