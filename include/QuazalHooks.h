/*
    RB2Enhanced - QuazalHooks.h
    Hooks for Quazal-related functions.
    Quazal is the creator of the networking middleware used by RB2.
*/

#include "rb2/Quazal/StepSequenceJobStep.h"

extern void OperatorEqualsFmt(char *r3, char *r4);
void OperatorEqualsFmtHook(char *r3, char *r4);
extern int StepSequenceJobSetStep(int *unk, StepSequenceJobStep *step);
int StepSequenceJobSetStepHook(int *unk, StepSequenceJobStep *step);
