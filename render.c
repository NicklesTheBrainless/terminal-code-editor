#include "editor.h"
#include "render.h"

#include <sys/ioctl.h>

#define colorReset "\x1b[0m"
#define workingDirColor "\x1b[1;38;5;198m"
#define dirFileColor "\x1b[38;5;211m"
#define selectedFileBackground "\x1b[48;5;225m"
#define borderColor "\x1b[38;5;204m"
#define renamingFileBackground "\x1b[48;5;189m"
#define openFileColor "\x1b[1;34m"
#define lineNumColor "\x1b[38;5;242m"
#define selectionBackground "\x1b[48;5;153m"

#define writeModeColor "\x1b[38;5;118m"
#define superModeColor "\x1b[38;5;227m"
#define filesysModeColor "\x1b[38;5;116m"

char *buffChars;
int buffLength;
int buffCap;

int tw, th = 0;
volatile sig_atomic_t terminalResized = 1;

char* shortenWorkingDirPath();
void getTerminalSize(int *width, int *height);
void buffAlloc();
void buffAppend(char *str);
void buffAppendN(char *str, int length);
void buffFillAppend(char c, int length);

void draw() {

    if (terminalResized == 1)
    {
        getTerminalSize(&tw, &th);
        terminalResized = 0;
    }

    buffAlloc();
    buffAppend("\x1b[2J\x1b[H");

    int lnOff = showLineNums ? LINENUM_W : 0;
    for (int i = 0; i < th; i++) {
        if (i > 0)
            buffAppend("\n");

        if (rowOffset+i >= rowsCount)
            break;

        
        if (showLineNums)
        {
            char numBuffer[5];
            sprintf(numBuffer, "%3d", (rowOffset+i+1) % 1000);
            buffAppend(lineNumColor);
            buffAppendN(numBuffer, 3);
            buffAppend(" \x1b[0m");
        } else {
            buffAppend(colorReset);
        }
        

        Row r = rows[rowOffset + i];
        int textSpace = tw - lnOff;
        if (r.length > textSpace)
            buffAppendN(r.chars, textSpace);
        else
            buffAppendN(r.chars, r.length);
    }

    char cursorMoveBuffer[64];
    sprintf(cursorMoveBuffer, "\x1b[%d;%dH", cy - rowOffset + 1, cx + lnOff + 1);
    buffAppend(cursorMoveBuffer);

    write(STDOUT_FILENO, buffChars, buffLength);
    fflush(stdout);
}

void drawWithFilesys() {

    if (terminalResized == 1)
    {
        getTerminalSize(&tw, &th);
        terminalResized = 0;
    }

    buffAlloc();
    buffAppend("\x1b[2J\x1b[H\x1b[?25l");

    buffAppend(workingDirColor);
    buffAppend("🗀 ");
    char *shortWdir = shortenWorkingDirPath();
    int wdirLength = strlen(shortWdir);
    buffAppendN(shortWdir, wdirLength);

    int emptySpace = FILESYS_W - (wdirLength+2);
    if (emptySpace > 0)
        buffFillAppend(' ', emptySpace);

    buffAppend(borderColor);
    buffAppend("│ ");

    int barWidth = tw - (FILESYS_W+1);
    int openFileSpace = barWidth/3;
    int infoTextSpace = barWidth - openFileSpace - 2;

    if (!showInfoText)
        openFileSpace = barWidth;

    int openFileLength;
    buffAppend(openFileColor);
    if (renamingFile)
    {        
        buffAppend(renamingFileBackground);
        buffAppendN(renameFileBuffer, renameFileBufferI);
        openFileLength = renameFileBufferI;
    } else {
        buffAppendN(dirFiles[openFileI], MIN(strlen(dirFiles[openFileI]), openFileSpace));
        openFileLength = strlen(dirFiles[openFileI]);
    }

    if (showInfoText)
    {
        buffAppend(colorReset);
        
        if (openFileSpace > openFileLength)
            buffFillAppend(' ', openFileSpace - openFileLength);
        buffAppend(borderColor);
        buffAppend("│ ");
        if (filesysMode)
        {
            buffAppend(filesysModeColor);
        } else {
            if (superMode)
                buffAppend(superModeColor);
            else
                buffAppend(writeModeColor);
        }
        buffAppendN(infoText, MIN(strlen(infoText), infoTextSpace));
        buffAppend("\n");
    } else {
        buffAppend("\x1b[0m\n");
    }

    buffAppend(dirFileColor);
    buffAppend("├─ ");
    if (selectedFileI == 0)
        buffAppend(selectedFileBackground);
    buffAppend(dirFiles[0]);
    buffAppend(colorReset);

    emptySpace = FILESYS_W - (strlen(dirFiles[0]) + 3);
    buffFillAppend(' ', emptySpace);

    buffAppend(borderColor);
    buffAppend("├");
    for (int i = 0; i < barWidth; i++) {
        if (i == openFileSpace+1)
            buffAppend("┴");
        else
            buffAppend("─");
    }

    int lnOff = showLineNums ? LINENUM_W : 0;
    for (int i = 0; i < th-WITHFILESYS_OFFSET_Y; i++) {
        buffAppend("\n");

        if (i+1 < dirFilesCount)
        {
            buffAppend(dirFileColor);
            buffAppend("├─ ");
            if (selectedFileI == i+1)
                buffAppend(selectedFileBackground);
            buffAppend(dirFiles[i+1]);
            buffAppend(colorReset);
            emptySpace = FILESYS_W - (strlen(dirFiles[i+1]) + 3);
            if (emptySpace > 0)
                buffFillAppend(' ', emptySpace);
        } else {
            buffFillAppend(' ', FILESYS_W);
        }
        
        buffAppend(borderColor);
        buffAppend("│ ");

        if (rowOffset+i >= rowsCount)
            continue;

        if (showLineNums)
        {
            char numBuffer[5];
            sprintf(numBuffer, "%3d", (rowOffset+i+1) % 1000);
            buffAppend(lineNumColor);
            buffAppendN(numBuffer, 3);
            buffAppend(" \x1b[0m");
        } else {
            buffAppend(colorReset);
        }

        int ri = rowOffset + i;
        Row r = rows[ri];

        int textSpace = tw - WITHFILESYS_OFFSET_X - lnOff;
        int visibleLength = MIN(r.length, textSpace);

        if (!selected1 || !selected2)
        {
            buffAppendN(r.chars, visibleLength);
        }
        else
        {
            int x1, y1, x2, y2;
            getNormalizedSelection(&x1, &y1, &x2, &y2);

            int highlightStart = 0;
            int highlightEnd = 0;
            bool highlighted = false;

            if (ri == y1 && ri == y2)
            {
                highlightStart = MIN(x1, visibleLength);
                highlightEnd = MIN(x2+1, visibleLength);
                highlighted = true;
            }
            else if (ri == y1)
            {
                highlightStart = MIN(x1, visibleLength);
                highlightEnd = visibleLength;
                highlighted = true;
            }
            else if (ri > y1 && ri < y2)
            {
                highlightStart = 0;
                highlightEnd = visibleLength;
                highlighted = true;
            }
            else if (ri == y2)
            {
                highlightStart = 0;
                highlightEnd = MIN(x2+1, visibleLength);
                highlighted = true;
            }

            if (!highlighted)
            {
                buffAppendN(r.chars, visibleLength);
            }
            else
            {
                buffAppendN(r.chars, highlightStart);
                buffAppend(selectionBackground);
                buffAppendN(r.chars + highlightStart, highlightEnd - highlightStart);
                buffAppend(colorReset);
                buffAppendN(r.chars + highlightEnd, visibleLength - highlightEnd);
            }
        }
    }
        

    char cursorMoveBuffer[64];
    if (renamingFile)
        sprintf(cursorMoveBuffer, "\x1b[1;%dH", FILESYS_W + 3 + renameFileBufferI);
    else
        sprintf(cursorMoveBuffer, "\x1b[%d;%dH", cy - rowOffset + WITHFILESYS_OFFSET_Y + 1, cx + WITHFILESYS_OFFSET_X + 1);
    buffAppend(cursorMoveBuffer);
    buffAppend("\x1b[?25h");

    write(STDOUT_FILENO, buffChars, buffLength);
    fflush(stdout);
}

char* shortenWorkingDirPath() {

    char first6[7];
    memcpy(first6, workingDir, 6);
    first6[6] = '\0';
    if (strcmp(first6, "/home/") != 0)
        return workingDir;

    int wdirLength = strlen(workingDir);
    int i;
    int slashCount = 0;
    for (i = 0; i < wdirLength; i++)
    {
        if (workingDir[i] == '/')
        {
            slashCount++;
            if (slashCount == 3)
                break;
        }
    }

    int cpyLength = wdirLength - i;
    char *shortened = malloc(cpyLength + 2);
    if (shortened == NULL)
    {
        free(buffChars);
        ERROR("malloc failed!");
        return NULL;
    }

    memcpy(shortened + 1, workingDir + i, cpyLength);
    shortened[0] = '~';
    shortened[cpyLength + 1] = '\0';
    return shortened;
}

void getTerminalSize(int *width, int *height)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
    {
        *width = 0;
        *height = 0;
        return;
    }

    *width = ws.ws_col;
    *height = ws.ws_row;
}

void handleResize(int signal)
{
    terminalResized = 1;
}



void buffAlloc() {
    buffChars = malloc(16384);
    buffCap = 16384;
    buffLength = 0;
}

void buffAppend(char *str) {

    int length = strlen(str);
    buffAppendN(str, length);    
}

void buffAppendN(char *str, int length) {

    if (buffLength + length >= buffCap)
    {
        buffCap += 16384;
        char *temp = realloc(buffChars, buffCap);
        if (temp == NULL) {
            free(buffChars);
            ERROR("realloc failed!");
        }
        buffChars = temp;
    }

    memcpy(buffChars + buffLength, str, length);
    buffLength += length;
}

void buffFillAppend(char c, int length) {

    if (buffLength + length >= buffCap)
    {
        buffCap += 16384;
        char *temp = realloc(buffChars, buffCap);
        if (temp == NULL) {
            free(buffChars);
            ERROR("realloc failed!");
        }
        buffChars = temp;
    }

    memset(buffChars + buffLength, c, length);
    buffLength += length;
}