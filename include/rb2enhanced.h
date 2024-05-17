/*
    RB2Enhanced - rb2enhanced.h
    Initialisation functions and basic patches for the game.
*/

// Applies "patches" to the game - single instruction/address pokes
void ApplyPatches();

// Initialises functions called by RB2Enhanced
void InitialiseFunctions();

// Hooks functions used by RB2Enhanced
void ApplyHooks();

// Startup hook
void StartupHook(void *ThisApp, int argc, char **argv);

// OS functions - defined in their platform specific C files
extern int RB2E_Mounted;
void RB2E_MountFileSystems();
char *RB2E_GetRawfilePath(char *path, int root_allowed);
int RB2E_FileExists(char *filename);
int RB2E_OpenFile(char *filename, char readWrite);
int RB2E_FileSize(int file);
int RB2E_ReadFile(int file, int offset, void *buffer, int size);
void RB2E_CloseFile(int file);
int RB2E_CreateThread(void *address, void *arg, int stack_size);
void RB2E_Sleep(int ms);
int RB2E_RelaunchGame();

// loaded song count - done in SongHooks.c
extern int RB2E_LoadedSongCount;

// Emulator detection
int RB2E_IsEmulator();
