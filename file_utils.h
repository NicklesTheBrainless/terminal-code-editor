#include "_project.h"

char** getFileNamesInDir(const char *dirPath, int *pCount);

Row* readFileAsRows(char *fileName, int *pRowsCount, int *pRowsCap);

void saveRowsAsFile(char *fileName, Row* _rows, int _rowsCount);

char* getWorkingDirectory();

void sortFilenames(char **fileNames, int n);

