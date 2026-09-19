#include "editor.h"

#include "input.h"
#include "render.h"

char *workingDir;
char **dirFiles;
int dirFilesCount;

int openFileI = 1;
int selectedFileI = 0;

bool showLineNums = true;
bool showInfoText = true;
char infoText[512];

bool withFilesys = true;
bool filesysMode = false;
bool superMode = false;

bool quit = false;

bool renamingFile = false;
char renameFileBuffer[MAX_RENAME_LENGTH];
int renameFileBufferLength = 0;
int renameFileBufferI = 0;

bool selected1 = false;
bool selected2 = false;
Selection selection;

int renameSymbolMode = 0;

char *originalSymbol;
int originalSymbolLength = 0;

char newSymbolBuffer[MAX_RENAME_LENGTH];
int newSymbolBufferLength = 0;
int newSymbolBufferI = 0;

Pos symbolPositions[MAX_SYMBOL_POS_COUNT];
int symbolPosCount = 0;

Row *clipboardRows = NULL;
int cbrCount;

Row *rows;
int rowsCount;
int rowsCap;

int targetCX = -1;
int cx = 0;
int cy = 0;
int rowOffset = 0;

int main() {

    printf("\x1b[?1049h");
    enableRawMode();

    workingDir = getWorkingDirectory();
    dirFiles = getFileNamesInDir(workingDir, &dirFilesCount);
    rows = readFileAsRows(dirFiles[openFileI], &rowsCount, &rowsCap);
    
    sprintf(infoText, "write mode");
    drawWithFilesys();

    signal(SIGWINCH, handleResize);
    char c;
    while (!quit)
    {

        int result = read(STDIN_FILENO, &c, 1);
        if (result == -1)
            break;

        if (result == 0 && terminalResized == 0)
            continue;
        
        if (result == 1)
            handleInput(c);

        if (withFilesys)
            drawWithFilesys();
        else
            draw();
    }

    disableRawMode();
    printf("\x1b[2J\x1b[?1049l");
    
}



void writeToRow(int x, int y, char *s, int n) {

    Row r = rows[y];

    if (r.length + n >= r.cap) {
        r.cap += n + 64;
        r.chars = realloc(r.chars, r.cap);
        if (r.chars == NULL) {
            ERROR_EXIT("realloc failed!");
        }
    }

    if (x < r.length)
        memmove(r.chars + x + n, r.chars + x, r.length - x);
    memcpy(r.chars + x, s, n);
    r.length += n;

    rows[y] = r;
}

void deleteText(int startX, int startY, int endX, int endY) {
    if (startY == endY)
    {
        Row *row = &rows[startY];

        int selectedLength = endX - startX;
        if (endX < row->length)
            memmove(row->chars + startX, row->chars + endX, row->length - endX);
        row->length = row->length - selectedLength;
    }
    else
    {
        Row *startRow = &rows[startY];
        Row *endRow   = &rows[endY];

        int newLength = startX + (endRow->length - endX);
        if (startRow->cap <= newLength)
        {
            startRow->cap = newLength + 64;
            startRow->chars = realloc(startRow->chars, startRow->cap);
            if (startRow->chars == NULL) {
                ERROR_EXIT("realloc failed!");
            }
        }

        if (endX < endRow->length)
            memmove(startRow->chars + startX, endRow->chars + endX, endRow->length - endX);
        startRow->length = newLength;

        for (int i = startY + 1; i <= endY; i++)
            free(rows[i].chars);

        memmove(&rows[startY + 1], &rows[endY + 1], (rowsCount - endY - 1) * sizeof(Row));
        rowsCount -= endY - startY;
    }
}

void freeRows() {
    for (int i = 0; i < rowsCount; i++)
        free(rows[i].chars);
    free(rows);
    rows = NULL;
}

void freeClipboardRows() {
    for (int i = 0; i < cbrCount; i++)
        free(clipboardRows[i].chars);
    free(clipboardRows);
    clipboardRows = NULL;
}

void getNormalizedSelection(int *x1, int *y1, int *x2, int *y2) {
    *x1 = selection.x1;
    *y1 = selection.y1;
    *x2 = selection.x2;
    *y2 = selection.y2;
        if (*y1 > *y2)
        {
            *x1 = selection.x2;
            *y1 = selection.y2;
            *x2 = selection.x1;
            *y2 = selection.y1;
        }
        else if (*y1 == *y2 && *x1 > *x2)
        {
            *x1 = selection.x2;
            *y1 = selection.y2;
            *x2 = selection.x1;
            *y2 = selection.y1;
        }
}