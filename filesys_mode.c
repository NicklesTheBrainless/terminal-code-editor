#include "filesys_mode.h"

void handleInputFilesysMode(char c) {
    switch (c)
    {
        case '\n': case 'e':
            saveRowsAsFile(dirFiles[openFileI], rows, rowsCount);
            freeRows();
            rows = readFileAsRows(dirFiles[selectedFileI], &rowsCount, &rowsCap);
            openFileI = selectedFileI;
            filesysMode = false;
            break;

        case 'w':
            if (selectedFileI > 0)
                    selectedFileI--;
            break;

        case 's':
            if (selectedFileI < dirFilesCount-1)
                selectedFileI++;
            break;
    }
    return;
}

void handleArrowKeyFilesysMode(ArrowKey ak) {
    switch (ak)
    {
        case UP_ARROW:
            if (selectedFileI > 0)
                selectedFileI--;
            return;
        
        case DOWN_ARROW:
            if (selectedFileI < dirFilesCount-1)
                selectedFileI++;
            return;
    }
}