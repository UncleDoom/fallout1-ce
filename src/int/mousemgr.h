#pragma once


namespace fallout {

using MouseManagerNameMangler = char*(char* fileName);
using MouseManagerRateProvider = int();
using MouseManagerTimeProvider = int();

void mousemgrSetNameMangler(MouseManagerNameMangler* func);
void mousemgrSetTimeCallback(MouseManagerRateProvider* rateFunc, MouseManagerTimeProvider* currentTimeFunc);
void initMousemgr();
void mousemgrClose();
void mousemgrUpdate();
int mouseSetFrame(char* fileName, int a2);
bool mouseSetMouseShape(char* fileName, int a2, int a3);
bool mouseSetMousePointer(char* fileName);
void mousemgrResetMouse();
void mouseHide();
void mouseShow();

} // namespace fallout
