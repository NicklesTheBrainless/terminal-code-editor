#include "editor.h"
#include "input.h"
#include "super_mode.h"

void handleInputRenameSymbol(char c);
void replaceAllGivenSymbolPos(Row *_rows, int *p_rowsCount, Pos *givenSymbolPositions, int givenSymbolPosCount);

void selectAllInCurrentScope();
void selectAllInCurrentFile();
void selectAllInWorkingDir();

void selectAllInSearchRows(Row* sRows, int sRowsCount, Pos *symbolPositionsBuffer, int *pSymbolPosCount);