#pragma once


#include "int/intrpret.h"

namespace fallout {

using IntLibProgramDeleteCallback = void(Program*);

void interpretFadePalette(unsigned char* oldPalette, unsigned char* newPalette, int a3, float duration);
int intlibGetFadeIn();
void interpretFadeOut(float duration);
void interpretFadeIn(float duration);
void interpretFadeOutNoBK(float duration);
void interpretFadeInNoBK(float duration);
int checkMovie(Program* program);
int getTimeOut();
void setTimeOut(int value);
void soundCloseInterpret();
int soundStartInterpret(char* fileName, int mode);
void updateIntLib();
void intlibClose();
void initIntlib();
void interpretRegisterProgramDeleteCallback(IntLibProgramDeleteCallback* callback);
void removeProgramReferences(Program* program);

} // namespace fallout
