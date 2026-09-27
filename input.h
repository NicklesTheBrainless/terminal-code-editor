#pragma once

#include "_project.h"

typedef enum {UP_ARROW, DOWN_ARROW, LEFT_ARROW, RIGHT_ARROW} ArrowKey;
typedef enum {NONE, ESCAPE, MOVE_LEFT, MOVE_RIGHT} RenameControlKey;

void handleInput(char c);
void moveCursor(ArrowKey ak);
void updateRowOffset();
RenameControlKey getRenameControlKey();
bool isVariableNameChar(char c);

void enableRawMode();
void disableRawMode();

