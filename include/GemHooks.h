/*
    RB2Enhanced - GemHooks.h
    Hooks for dealing with gems.
*/

#include "rb2/GameGem.h"
#include "rb2/Symbol.h"

int WillBeNoStrumHook(int *gameGemListPtr, int *multiGemInfoPtr);
int *GetWidgetByNameHook(int *gemManager, Symbol sym);
int AddGameGemHook(int *gameGemList, GameGem *gem, NoStrumState gemType);
Symbol GetSlotColorHook(int *bandUser);