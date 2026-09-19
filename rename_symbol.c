#include "rename_symbol.h"

#include "editor.h"
#include "input.h"
#include "super_mode.h"

#include <ctype.h>

void handleInputRenameSymbol(char c) {

    if ((c == 6 && !superMode) || (c == 'f' && superMode))
    {
        handleSuperKey('f');

    } else if (c == '\x1b') {
        RenameControlKey rck = getRenameControlKey();
        if (rck == ESCAPE)
        {
            renameSymbolMode = 0;
            newSymbolBufferLength = 0;
            newSymbolBufferI = 0;

            free(originalSymbol);
            symbolPosCount = 0;;

            selected1 = false;
            selected2 = false;

        } else if (rck == MOVE_LEFT) {
            if (newSymbolBufferI > 0)
                newSymbolBufferI--;
        } else if (rck == MOVE_RIGHT) {
            if (newSymbolBufferI < newSymbolBufferLength-1)
                newSymbolBufferI++;
        }
        return;
    } else if (c == '\n') {
        if (newSymbolBufferLength > 0) {
            replaceAllSymbolPos();
        }
        renameSymbolMode = 0;
        newSymbolBufferLength = 0;
        newSymbolBufferI = 0;

        free(originalSymbol);
        symbolPosCount = 0;;

        selected1 = false;
        selected2 = false;
        
    } else if (c == 127 && newSymbolBufferI > 0) {
        memmove(newSymbolBuffer + newSymbolBufferI - 1, newSymbolBuffer + newSymbolBufferI, newSymbolBufferLength - newSymbolBufferI);
        newSymbolBufferLength--;
        newSymbolBufferI--;

    } else if (c != '/' && c != '\0' && !iscntrl(c) && newSymbolBufferLength < 1023) {
        memmove(newSymbolBuffer + newSymbolBufferI + 1, newSymbolBuffer + newSymbolBufferI, newSymbolBufferLength - newSymbolBufferI);
        newSymbolBuffer[newSymbolBufferI] = c;
        newSymbolBufferLength++;
        newSymbolBufferI++;
    }

}

void replaceAllSymbolPos() {
    int currentY = -1;
    int currentOffsetX = 0;
    for (int i = 0; i < symbolPosCount; i++)
    {
        Pos sp = symbolPositions[i];
        if (currentY != sp.y) {
            currentY = sp.y;
            currentOffsetX = 0;
        }

        deleteText(sp.x + currentOffsetX, sp.y, sp.x + originalSymbolLength + currentOffsetX, sp.y);
        writeToRow(sp.x + currentOffsetX, sp.y, newSymbolBuffer, newSymbolBufferLength);
        
        currentOffsetX += newSymbolBufferLength - originalSymbolLength;
    }
}



void selectAllInCurrentScope(int originalX, int originalY) {

    int originalBracesLevel = 0;
    for (int y = 0; y <= originalY; y++)
    {
        Row r = rows[y];
        for (int x = 0; x < r.length; x++)
        {
            if (y == originalY && x == originalX)
                break;
            if (r.chars[x] == '{')
                originalBracesLevel++;
            else if (r.chars[x] == '}')
                originalBracesLevel--;
        }
    }

    int currentBracesLevel = 0;
    int stopSearching = false;
    for (int y = 0; y < rowsCount; y++)
    {
        Row r = rows[y];
        for (int x = 0; x < r.length; x++)
        {
            if (r.chars[x] == '{')
                currentBracesLevel++;
            else if (r.chars[x] == '}')
                currentBracesLevel--;

            if (currentBracesLevel < originalBracesLevel)
                continue;

            if (x + originalSymbolLength > r.length)
                continue;

            bool isSymbol = true;
            for (int i = 0; i < originalSymbolLength; i++)
            {
                if (r.chars[x + i] != originalSymbol[i]) {
                    isSymbol = false;
                    break;
                }
            }

            if (isSymbol)
            {
                symbolPositions[symbolPosCount].x = x;
                symbolPositions[symbolPosCount].y = y;
                symbolPosCount++;
                if (symbolPosCount >= MAX_SYMBOL_POS_COUNT)
                    stopSearching = true;
            }
        }

        if (stopSearching)
            break;
    }
}

void selectAllInCurrentFile() {
    for (int y = 0; y < rowsCount; y++)
    {
        Row r = rows[y];
        for (int x = 0; x < r.length; x++)
        {
            bool isSymbol = true;
            for (int i = 0; i < originalSymbolLength; i++)
            {
                if (r.chars[x + i] != originalSymbol[i]) {
                    isSymbol = false;
                    break;
                }
            }

            if (isSymbol)
            {
                symbolPositions[symbolPosCount].x = x;
                symbolPositions[symbolPosCount].y = y;
                symbolPosCount++;
            }
        }
    }
}

void selectAllInWorkingDir() {

}