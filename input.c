#include "input.h"
#include "editor.h"
#include "render.h"

#include "filesys_mode.h"
#include "super_mode.h"
#include "write_mode.h"
#include "rename_symbol.h"

#include <termio.h>
#include <ctype.h>

void handleEscSeq();
void handleAlt(char c);

void handleInput(char c) {

    if (renameSymbolMode > 0)
    {
        handleInputRenameSymbol(c);
        return;
    }

    if (renamingFile)
    {
        if (c == '\x1b') {
            RenameControlKey rck = getRenameControlKey();
            if (rck == ESCAPE)
            {
                renamingFile = false;
                renameFileBufferLength = 0;
                renameFileBufferI = 0;
            } else if (rck == MOVE_LEFT) {
                if (renameFileBufferI > 0)
                    renameFileBufferI--;
            } else if (rck == MOVE_RIGHT) {
                if (renameFileBufferI < renameFileBufferLength-1)
                    renameFileBufferI++;
            }
            return;
        } else if (c == '\n') {
            if (renameFileBufferLength > 0)
            {
                renameFileBuffer[renameFileBufferLength] = '\0';
                if (rename(dirFiles[openFileI], renameFileBuffer) != 0)
                {
                    ERROR_EXIT("failed to rename file!");
                }
                dirFiles = getFileNamesInDir(workingDir, &dirFilesCount);
                openFileI = 0;
                for (int i = 0; i < dirFilesCount; i++)
                {
                    if (strcmp(dirFiles[i], renameFileBuffer) == 0) {
                        openFileI = i;
                        break;
                    }
                }
                
            }
            renamingFile = false;
            renameFileBufferLength = 0;
            renameFileBufferI = 0;
            
        } else if (c == 127 && renameFileBufferI > 0) {
            memmove(renameFileBuffer + renameFileBufferI - 1, renameFileBuffer + renameFileBufferI, renameFileBufferLength - renameFileBufferI);
            renameFileBufferLength--;
            renameFileBufferI--;

        } else if (c != '/' && c != '\0' && !iscntrl(c) && renameFileBufferLength < 1023) {
            memmove(renameFileBuffer + renameFileBufferI + 1, renameFileBuffer + renameFileBufferI, renameFileBufferLength - renameFileBufferI);
            renameFileBuffer[renameFileBufferI] = c;
            renameFileBufferLength++;
            renameFileBufferI++;
        }
        return;
    }
    

    if (c == '\x1b') {
        handleEscSeq();
        return;
    }

    if (filesysMode) {
        handleInputFilesysMode(c);
        return;
    }

    if (!superMode) {

        if (c >= 1 && c <= 26 && c != '\t' && c != '\n')
        {
            char superKey = c + 'a' - 1;
            handleSuperKey(superKey);
        } else {
            handleInputWriteMode(c);
        }
    } else {
        handleSuperKey(c);
    }
    
}

void handleEscSeq()
{
    char seq[2];
    if (read(STDIN_FILENO, &seq[0], 1) != 1)
        return;
    if (seq[0] != '[') {
        handleAlt(seq[0]);
        return;
    }
    if (read(STDIN_FILENO, &seq[1], 1) != 1)
        return;

    switch (seq[1])
    {
        case 'A':
            if (filesysMode)
                handleArrowKeyFilesysMode(UP_ARROW);
            else
                moveCursor(UP_ARROW);
            break;
        case 'B':
            if (filesysMode)
                handleArrowKeyFilesysMode(DOWN_ARROW);
            else
                moveCursor(DOWN_ARROW);
            break;
        case 'C':
            if (!filesysMode)
                moveCursor(RIGHT_ARROW);
            break;
        case 'D':
            if (!filesysMode)
                moveCursor(LEFT_ARROW);
            break;

        case 'Z':
            if (!filesysMode)
                superMode = !superMode;
            break;
    }

    return;
}

void moveCursor(ArrowKey ak) {

    switch (ak)
    {
        case UP_ARROW:
            if (cy == 0)
                return;
            cy--;
            if (targetCX == -1)
            {
                if (cx > rows[cy].length) {
                    targetCX = cx;
                    cx = rows[cy].length;
                }
            } else {
                if (cx > rows[cy].length) {
                    cx = rows[cy].length;
                } else {
                    if (targetCX <= rows[cy].length)
                    {
                        cx = targetCX;
                        targetCX = -1;
                    } else {
                        cx = rows[cy].length;
                    }
                }
            }
            break;

        case DOWN_ARROW:
            if (cy == rowsCount-1)
                return;
            cy++;
            if (targetCX == -1)
            {
                if (cx > rows[cy].length) {
                    targetCX = cx;
                    cx = rows[cy].length;
                }
            } else {
                if (cx > rows[cy].length) {
                    cx = rows[cy].length;
                } else {
                    if (targetCX <= rows[cy].length)
                    {
                        cx = targetCX;
                        targetCX = -1;
                    } else {
                        cx = rows[cy].length;
                    }
                }
            }
            break;

        case RIGHT_ARROW:
            if (cx < rows[cy].length) {
                cx++;
            } else if (cy < rowsCount-1) {
                cy++;
                cx = 0;
            }
            break;

        case LEFT_ARROW:
            if (cx > 0) {
                cx--;
            } else if (cy > 0) {
                cy--;
                cx = rows[cy].length;
            }
            break;
    }

    updateRowOffset();
}

void updateRowOffset() {
    int subtractY = 1;
    if (withFilesys)
        subtractY = WITHFILESYS_OFFSET_Y + 1;
    
    if (cy - rowOffset < MIN_CURSOR_Y_SIDE_DISTANCE)
    {
        rowOffset = cy - MIN_CURSOR_Y_SIDE_DISTANCE;

        if (rowOffset < 0)
            rowOffset = 0;
    }
    else if (cy - rowOffset > th - MIN_CURSOR_Y_SIDE_DISTANCE - subtractY)
    {
        rowOffset = cy - (th - MIN_CURSOR_Y_SIDE_DISTANCE - subtractY);

        if (rowOffset > rowsCount - 1)
            rowOffset = rowsCount - 1;
    }    
}

void handleAlt(char c) {

    switch (c)
    {
    case 'q':
        quit = true;
        break;
    
    case 's':
        saveRowsAsFile(dirFiles[openFileI], rows, rowsCount);
        break;

    case 'r':
        if (withFilesys)
            renamingFile = true;
        break;
    
    case 'f':
        withFilesys = !withFilesys;
        filesysMode = withFilesys;
        break;

    case 'g':
        filesysMode = !filesysMode;
        break;

    case 'i':
        showInfoText = !showInfoText;
        break;

    case 'l':
        showLineNums = !showLineNums;
        break;
    }
}

RenameControlKey getRenameControlKey() {

    char seq[2];
    int result = read(STDIN_FILENO, &seq[0], 1);
    if (result == 0)
        return ESCAPE;
    if (result != 1)
        return NONE;

    if (seq[0] != '[') {
        return NONE;
    }

    if (read(STDIN_FILENO, &seq[1], 1) != 1)
        return NONE;

    switch (seq[1])
    {
        case 'C':
            return MOVE_RIGHT;
        case 'D':
            return MOVE_LEFT;
    }

    return NONE;
}



struct termios original;

void enableRawMode()
{
    tcgetattr(STDIN_FILENO, &original);
    struct termios raw = original;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_iflag &= ~(IXON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 2;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}