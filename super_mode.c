#include "super_mode.h"
#include "rename_symbol.h"

Selection copiedCoords;

void handleSuperKey(char c) {

    int startX, startY, endX, endY;
    switch (c)
    {
    case 'w':
        moveCursor(UP_ARROW);
        break;
    case 's':
        moveCursor(DOWN_ARROW);
        break;
    case 'a':
        moveCursor(LEFT_ARROW);
        break;
    case 'd':
        moveCursor(RIGHT_ARROW);
        break;

    case 'w' & 0x1F:
        for (int i = 0; i < 5; i++)
            moveCursor(UP_ARROW);
        break;
    case 's' & 0x1F:
        for (int i = 0; i < 5; i++)
            moveCursor(DOWN_ARROW);
        break;
    case 'a' & 0x1F:
        for (int i = 0; i < 5; i++)
            moveCursor(LEFT_ARROW);
        break;
    case 'd' & 0x1F:
        for (int i = 0; i < 5; i++)
            moveCursor(RIGHT_ARROW);
        break;

    case 'q':
        bool before1 = selected1;
        selected1 = true;
        if (selection.x1 == cx && selection.y1 == cy && before1)
        {
            selection.x1 = 0;
            break;
        }
        selection.x1 = MIN(cx, rows[cy].length-1);
        selection.y1 = cy;
        break;
    case 'e':
        bool before2 = selected2;
        selected2 = true;
        if (selection.x2 == cx && selection.y2 == cy && before2)
        {
            selection.x2 = rows[cy].length-1;
            break;
        }
        selection.x2 = MIN(cx, rows[cy].length-1);
        selection.y2 = cy;
        break;
    case 'r':
        selected1 = false;
        selected2 = false;
        break;

    case 'c':
        if (copiedCoords.x1 == selection.x1 &&
            copiedCoords.y1 == selection.y1 &&
            copiedCoords.x2 == selection.x2 &&
            copiedCoords.y2 == selection.y2)
        {
            selected1 = false;
            selected2 = false;
            break;
        }
        copyToClipboard();
        copiedCoords = selection;
        break;
    case 'v':
        if (selected1 && selected2)
        {
            getNormalizedSelection(&startX, &startY, &endX, &endY);
            endX++;

            deleteText(startX, startY, endX, endY);

            cy = startY;
            cx = startX;
            pasteFromClipboard(true);

            selected1 = false;
            selected2 = false;
        } else {
            pasteFromClipboard(true);
        }
        break;
    case 'x':
        copyToClipboard();
        getNormalizedSelection(&startX, &startY, &endX, &endY);
        endX++;
        deleteText(startX, startY, endX, endY);
        cx = startX;
        selected1 = false;
        selected2 = false;
        break;

    case 'f':
        if (!selected1 || !selected2)
            break;
        if (selection.y1 != selection.y2) {
            sprintf(infoText, "find occurrences does not work with multiline selection");
            break;
        }

        Row sr = rows[selection.y1];

        startX = MIN(selection.x1, selection.x2);
        endX = MAX(selection.x1, selection.x2) + 1;
        char beforeChar = 0;
        char afterChar = 0;
        if (startX-1 >= 0)
            beforeChar = sr.chars[startX-1];
        if (endX < sr.length)
            afterChar = sr.chars[endX];
        if (isVariableNameChar(beforeChar) || isVariableNameChar(beforeChar)) {
            sprintf(infoText, "find occurrences only works with whole symbols %c %c", beforeChar, afterChar);    
            break;
        }

        originalSymbol = malloc(endX - startX + 1);
        if (originalSymbol == NULL)
        {
            ERROR_EXIT("malloc failed!");
        }
        memcpy(originalSymbol, rows[selection.y1].chars + startX, endX - startX);
        originalSymbolLength = endX - startX;
        originalSymbol[originalSymbolLength] = '\0';

        originalX = startX;
        originalY = selection.y1;

        memcpy(newSymbolBuffer, originalSymbol, originalSymbolLength);
        newSymbolBufferLength = originalSymbolLength;
        newSymbolBufferI = originalSymbolLength;

        if (renameSymbolMode >= 3)
            renameSymbolMode = 1;
        else
            renameSymbolMode++;

        if (renameSymbolMode == 1) {
            selectAllInCurrentScope();
            sprintf(infoText, "found %d occurrences of \"%s\" in current scope", symbolPosCount, originalSymbol);
        } else if (renameSymbolMode == 2) {
            selectAllInCurrentFile();
            sprintf(infoText, "found %d occurrences of \"%s\" in current file", symbolPosCount, originalSymbol);
        } else if (renameSymbolMode == 3) {
            selectAllInWorkingDir();
            sprintf(infoText, "found %d occurrences of \"%s\" in working directory", symbolPosCount, originalSymbol);
        }
        break;
    }
}

void copyToClipboard()
{
    if (clipboardRows != NULL)
        freeClipboardRows();

    if (!selected1 || !selected2)
    {
        clipboardRows = malloc(sizeof(Row));
        cbrCount = 1;
        Row r = rows[cy];
        clipboardRows[0].length = r.length;
        clipboardRows[0].cap = r.cap;
        clipboardRows[0].chars = malloc(r.cap);
        if (clipboardRows[0].chars == NULL)
        {
            ERROR_EXIT("malloc failed!");
        }
        memcpy(clipboardRows[0].chars, r.chars, r.length);
        return;
    }

    int x1, y1, x2, y2;
    getNormalizedSelection(&x1, &y1, &x2, &y2);

    cbrCount = y2-y1+1;
    clipboardRows = malloc(cbrCount * sizeof(Row));
    if (clipboardRows == NULL)
    {
        ERROR_EXIT("malloc failed!");
    }

    for (int i = 0; i < cbrCount; i++)
    {
        int ri = y1 + i;
        Row r = rows[y1 + i];

        int start = 0;
        int end = 0;

        if (ri == y1 && ri == y2)
        {
            start = x1;
            end = x2+1;
        }
        else if (ri == y1)
        {
            start = x1;
            end = r.length;
        }
        else if (ri > y1 && ri < y2)
        {
            start = 0;
            end = r.length;
        }
        else if (ri == y2)
        {
            start = 0;
            end = x2+1;
        }

        int length = end - start;
        clipboardRows[i].length = length;
        clipboardRows[i].cap = length + 64;
        clipboardRows[i].chars = malloc(length + 64);
        memcpy(clipboardRows[i].chars, r.chars + start, length);
    }
}

void pasteFromClipboard(bool tabCorrection)
{
    if (clipboardRows == NULL || cbrCount <= 0)
        return;

    Row *current = &rows[cy];

    if (cbrCount == 1)
    {
        Row *clip = &clipboardRows[0];

        int newLength = current->length + clip->length;
        if (newLength > current->cap)
        {
            current->cap = newLength + 64;
            current->chars = realloc(current->chars, current->cap);
            if (current->chars == NULL) {
                ERROR_EXIT("realloc failed!");
            }
        }

        memmove(current->chars + cx + clip->length, current->chars + cx, current->length - cx);
        memcpy(current->chars + cx, clip->chars, clip->length);
        current->length = newLength;

        cx += clip->length;
        return;
    }

    int preSpace = 0;
    if (tabCorrection)
        preSpace = cx - (cx % TAB_SIZE);

    int suffixLength = current->length - cx;
    char *suffix = NULL;
    if (suffixLength > 0)
    {
        suffix = malloc(suffixLength);
        if (suffix == NULL) {
            ERROR_EXIT("malloc failed!");
        }
        memcpy(suffix, current->chars + cx, suffixLength);
    }

    int rowsToAdd = cbrCount - 1;
    if (rowsCount + rowsToAdd > rowsCap)
    {
        rowsCap = rowsCount + rowsToAdd + 64;
        rows = realloc(rows, rowsCap * sizeof(Row));

        if (rows == NULL) {
            ERROR_EXIT("realloc failed!");
        }
    }

    memmove(&rows[cy + cbrCount], &rows[cy + 1], (rowsCount - cy - 1) * sizeof(Row));

    Row *first = &clipboardRows[0];

    int firstLength = cx + first->length;
    if (firstLength > current->cap)
    {
        current->cap = firstLength + 64;
        current->chars = realloc(current->chars, current->cap);
        if (current->chars == NULL) {
            ERROR_EXIT("realloc failed!");
        }
    }

    memcpy(current->chars + cx, first->chars, first->length);
    current->length = firstLength;

    for (int i = 1; i < cbrCount; i++)
    {
        Row *src = &clipboardRows[i];
        Row *dst = &rows[cy + i];

        dst->length = src->length + preSpace;
        dst->cap = dst->length + 64;
        dst->chars = malloc(dst->cap);
        if (dst->chars == NULL) {
            ERROR_EXIT("malloc failed!");
        }

        memcpy(dst->chars + preSpace, src->chars, src->length);
        if (preSpace > 0)
            memset(dst->chars, ' ', preSpace);
    }

    Row *last = &rows[cy + cbrCount - 1];

    int lastLength = last->length + suffixLength;
    if (lastLength > current->cap)
    {
        current->cap = lastLength + 64;
        current->chars = realloc(current->chars, current->cap);
        if (current->chars == NULL) {
            ERROR_EXIT("realloc failed!");
        }
    }

    if (suffixLength > 0)
        memcpy(last->chars + last->length, suffix, suffixLength);
    last->length = lastLength;
    rowsCount += rowsToAdd;

    cy += cbrCount - 1;
    cx = last->length - suffixLength;
    updateRowOffset();

    free(suffix);
}