#include "rename_symbol.h"

#include <ctype.h>

Pos **dirSymbolPositions = NULL;
int *dirSymbolsPosCounts = NULL;
int oldDirFilesCount;

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
            if (newSymbolBufferI < newSymbolBufferLength)
                newSymbolBufferI++;
        }
        return;
    } else if (c == '\n') {
        if (newSymbolBufferLength > 0) {
            if (renameSymbolMode == 3)
            {
                for (int i = 0; i < dirFilesCount; i++)
                {
                    int writeRowsCount;
                    int writeRowsCap;
                    Row* writeRows;
                    if (i == openFileI)
                    {
                        writeRows = rows;
                        writeRowsCount = rowsCount;
                    } else {
                        writeRows = readFileAsRows(dirFiles[i], &writeRowsCount, &writeRowsCap);
                    }
                    
                    replaceAllGivenSymbolPos(writeRows, &rowsCount, dirSymbolPositions[i], dirSymbolsPosCounts[i]);
                    saveRowsAsFile(dirFiles[i], writeRows, rowsCount);
                }

            } else {
                replaceAllGivenSymbolPos(rows, &rowsCount, symbolPositions, symbolPosCount);
            }
        }
        cx = MIN(selection.x1, selection.x2) + newSymbolBufferLength;
        cy = selection.y1;
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

void replaceAllGivenSymbolPos(Row *_rows, int *p_rowsCount, Pos *givenSymbolPositions, int givenSymbolPosCount) {
    int currentY = -1;
    int currentOffsetX = 0;
    for (int i = 0; i < givenSymbolPosCount; i++)
    {
        Pos sp = givenSymbolPositions[i];
        if (currentY != sp.y) {
            currentY = sp.y;
            currentOffsetX = 0;
        }

        deleteTextHelper(_rows, p_rowsCount, sp.x + currentOffsetX, sp.y, sp.x + originalSymbolLength + currentOffsetX, sp.y);
        writeToRowHelper(_rows, sp.x + currentOffsetX, sp.y, newSymbolBuffer, newSymbolBufferLength);
        
        currentOffsetX += newSymbolBufferLength - originalSymbolLength;
    }
}



void selectAllInCurrentScope() {

    int currentBracesLevel = 0;
    int stopSearching = false;
    for (int y = originalY; y < rowsCount; y++)
    {
        Row r = rows[y];
        for (int x = originalX; x < r.length; x++)
        {
            if (r.chars[x] == '{')
                currentBracesLevel++;
            else if (r.chars[x] == '}')
                currentBracesLevel--;

            if (currentBracesLevel < 0)
                break;

            if (x + originalSymbolLength > r.length)
                continue;
            
            char beforeChar = 0;
            char afterChar = 0;
            if (originalX-1 >= 0)
                beforeChar = r.chars[originalX-1];
            if (originalX+originalSymbolLength < r.length)
                afterChar = r.chars[originalX+originalSymbolLength];
            if (isVariableNameChar(beforeChar) || isVariableNameChar(afterChar))
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
                if (symbolPosCount >= MAX_SYMBOL_POS_COUNT) {
                    stopSearching = true;
                    break;
                }
                x += originalSymbolLength-1;
            }
        }

        if (stopSearching)
            break;
    }
}

void selectAllInCurrentFile() {
    selectAllInSearchRows(rows, rowsCount, symbolPositions, &symbolPosCount);
}

void selectAllInWorkingDir() {

    if (dirSymbolPositions != NULL)
    {
        for (int i = 0; i < oldDirFilesCount; i++)
            free(dirSymbolPositions[i]);
        free(dirSymbolPositions);
        free(dirSymbolsPosCounts);
    }

    ERROR("after free");

    dirSymbolPositions = malloc(dirFilesCount * sizeof(Pos*));
    for (int i = 0; i < dirFilesCount; i++)
    {
        dirSymbolPositions[i] = malloc(MAX_SYMBOL_POS_COUNT * sizeof(Pos));
        if (dirSymbolPositions[i] == NULL)
        {
            ERROR_EXIT("malloc failed");
        }
    }
    dirSymbolsPosCounts = malloc(dirFilesCount * sizeof(int));
    oldDirFilesCount = dirFilesCount;

    ERROR("after malloc");

    for (int i = 0; i < dirFilesCount; i++)
    {
        int readRowsCount;
        int readRowsCap;
        Row* readRows = readFileAsRows(dirFiles[i], &readRowsCount, &readRowsCap);
        ERROR("after read");
        dirSymbolsPosCounts[i] = 0;
        selectAllInSearchRows(readRows, readRowsCount, dirSymbolPositions[i], &(dirSymbolsPosCounts[i]));
        ERROR("after select");
        fprintf(stderr, "%d\n", dirSymbolsPosCounts[i]);
        globalSymbolPosCount += dirSymbolsPosCounts[i];
        ERROR("loop read");
    }

    ERROR("after read loop, before end");

    memcpy(symbolPositions, dirSymbolPositions[openFileI], dirSymbolsPosCounts[openFileI]);
    symbolPosCount = dirSymbolsPosCounts[openFileI];

    ERROR("search end");
}

void selectAllInSearchRows(Row* sRows, int sRowsCount, Pos *symbolPositionsBuffer, int *pSymbolPosCount) {
    bool stopSearching = false;
    for (int y = 0; y < sRowsCount; y++)
    {
        Row r = sRows[y];
        ERROR("got row");
        for (int x = 0; x < r.length; x++)
        {
            if (x + originalSymbolLength > r.length)
                continue;
            
            char beforeChar = 0;
            char afterChar = 0;
            if (x-1 >= 0)
                beforeChar = r.chars[x-1];
            if (x+originalSymbolLength < r.length)
                afterChar = r.chars[x+originalSymbolLength];
            if (isVariableNameChar(beforeChar) || isVariableNameChar(afterChar))
                continue;

            ERROR("after char check");

            bool isSymbol = true;
            for (int i = 0; i < originalSymbolLength; i++)
            {
                if (r.chars[x + i] != originalSymbol[i]) {
                    isSymbol = false;
                    break;
                }
            }

            ERROR("match check");

            if (isSymbol)
            {
                ERROR("is symbol start");
                symbolPositionsBuffer[*pSymbolPosCount].x = x;
                symbolPositionsBuffer[*pSymbolPosCount].y = y;
                (*pSymbolPosCount)++;
                if (*pSymbolPosCount >= MAX_SYMBOL_POS_COUNT) {
                    stopSearching = true;
                    break;
                }
                x += originalSymbolLength-1;
                ERROR("is symbol end");
            }
        }

        if (stopSearching)
            break;
    }
}