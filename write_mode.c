#include "write_mode.h"

void handleInputWriteMode(char c) {

    bool selectionjustDeleted = false;
    if (selected1 && selected2)
    {
        int startX, startY, endX, endY;
        getNormalizedSelection(&startX, &startY, &endX, &endY);
        endX++;

        deleteText(startX, startY, endX, endY);

        cy = startY;
        cx = startX;

        selected1 = false;
        selected2 = false;
        selectionjustDeleted = true;
    }
    
    Row r = rows[cy];

    if (c == '\n') {

        Row newRow;
        int newLength = r.length - cx;

        newRow.cap = newLength + 64;
        newRow.length = newLength;
        newRow.chars = malloc(newRow.cap);
        if (newRow.chars == NULL) {
            ERROR_EXIT("malloc failed!");
        }

        memcpy(newRow.chars, r.chars + cx, newLength);

        r.length = cx;
        rowsCount++;

        if (rowsCount >= rowsCap)
        {
            rowsCap += 256;
            rows = realloc(rows, rowsCap * sizeof(Row));
            if (rows == NULL) {
                ERROR_EXIT("realloc failed!");
            }
        }

        memmove(&rows[cy + 2], &rows[cy + 1], (rowsCount - cy - 1) * sizeof(Row));
        rows[cy] = r;
        rows[cy + 1] = newRow;

        cy++;
        cx = 0;
        return;
    }

    if (c == 127 && !selectionjustDeleted)
    {
        if (cx == 0 && cy == 0)
            return;
        
        if (cx == 0) {
            Row upRow = rows[cy-1];
            int oldUpRowLen = upRow.length;

            if (upRow.length+r.length >= upRow.cap) {
                upRow.cap += 64 + r.length;
                upRow.chars = realloc(upRow.chars, upRow.cap);
                if (upRow.chars == NULL) {
                    ERROR_EXIT("realloc failed!");
                }
            }

            memcpy(upRow.chars + upRow.length, r.chars, r.length);
            upRow.length += r.length;
            rows[cy-1] = upRow;

            free(r.chars);
            memmove(&rows[cy], &rows[cy+1], (rowsCount-cy-1)*sizeof(Row));\
            rowsCount--;

            cy--;
            cx = oldUpRowLen;

        } else {
            memmove(r.chars+cx-1, r.chars+cx, r.length - cx);
            r.length--;
            cx--;
            rows[cy] = r;
        }
        return;
    }

    if (c == '\t')
    {
        writeToRow(cx, cy, "    ", 4);
        cx += 4;
        return;
    }

    writeToRow(cx, cy, &c, 1);
    cx++;
}

