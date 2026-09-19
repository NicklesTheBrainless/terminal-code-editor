#pragma once

#include "_project.h"

#include "file_utils.h"

extern char *workingDir;
extern char **dirFiles;
extern int dirFilesCount;

extern int openFileI;
extern int selectedFileI;

extern bool showLineNums;
extern bool showInfoText;
extern char infoText[512];

extern bool withFilesys;
extern bool filesysMode;
extern bool superMode;

extern bool quit;

extern bool renamingFile;
extern char renameFileBuffer[MAX_RENAME_LENGTH];
extern int renameFileBufferLength;
extern int renameFileBufferI;

extern bool selected1;
extern bool selected2;
extern Selection selection;

extern int renameSymbolMode;

extern char *originalSymbol;
extern int originalSymbolLength;

extern char newSymbolBuffer[MAX_RENAME_LENGTH];
extern int newSymbolBufferLength;
extern int newSymbolBufferI;

extern Pos symbolPositions[MAX_SYMBOL_POS_COUNT];
extern int symbolPosCount;

extern Row *clipboardRows;
extern int cbrCount;

extern Row *rows;
extern int rowsCount;
extern int rowsCap;

extern int targetCX;
extern int cx;
extern int cy;
extern int rowOffset;

void writeToRow(int x, int y, char *s, int n);
void deleteText(int startX, int startY, int endX, int endY);
void freeRows();
void freeClipboardRows();
void getNormalizedSelection(int *x1, int *y1, int *x2, int *y2);