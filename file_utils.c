#include "file_utils.h"

#include <dirent.h>
#include <fcntl.h>

char** getFileNamesInDir(const char *dirPath, int *pCount)
{
    DIR *dir = opendir(dirPath);
    if (dir == NULL)
        return NULL;

    int capacity = 32;
    int count = 0;

    char **fileNames = malloc(capacity * sizeof(char *));

    if (fileNames == NULL)
    {
        closedir(dir);
        return NULL;
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if ( (strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0) )
            continue;

        if (count >= capacity)
        {
            capacity += 32;

            char **temp = realloc(fileNames, capacity * sizeof(char *));

            if (temp == NULL)
                goto emergency_cleanup1;

            fileNames = temp;
        }

        fileNames[count] = malloc(strlen(entry->d_name) + 1);

        if (fileNames[count] == NULL)
            goto emergency_cleanup1;

        strcpy(fileNames[count], entry->d_name);
        count++;
    }

    closedir(dir);

    sortFilenames(fileNames, count);

    *pCount = count;
    return fileNames;

emergency_cleanup1:
    for (int i = 0; i < count; i++)
        free(fileNames[i]);

    free(fileNames);
    closedir(dir);
    return NULL;
}

Row* readFileAsRows(char *fileName, int *pRowsCount, int *pRowsCap) {

    FILE *file = fopen(fileName, "r");
    if (file == NULL)
        return NULL;

    char buffer[1024];

    int rowcap = 256;
    int rowi = 0;
    Row *rowsBuffer = malloc(rowcap * sizeof(Row));
    if (rowsBuffer == NULL) {
        fclose(file);
        ERROR("malloc failed!");
        return NULL;
    }

    int cap = 64;
    int li = 0;
    char *line = malloc(cap);
    bool newline = false;

    while (fgets(buffer, sizeof(buffer), file)) {

        if (newline) {
            cap = 64;
            li = 0;
            line = malloc(cap);
            newline = false;
        }
        
        if (line == NULL) {
            ERROR("malloc failed!");
            goto emergency_cleanup2;
        }
        
        for (int i = 0; i < sizeof(buffer); i++)
        {
            char c = buffer[i];
            if (c == '\n' || c == '\0') {
                rowsBuffer[rowi].chars = line;
                rowsBuffer[rowi].length = li;
                rowsBuffer[rowi].cap = cap;
                rowi++;
                if (rowi >= rowcap) {
                    rowcap += 256;
                    Row *temp = realloc(rowsBuffer, rowcap * sizeof(Row));
                    if (temp == NULL) {
                        free(line);
                        ERROR("realloc failed!");
                        goto emergency_cleanup2;
                    }
                    rowsBuffer = temp;
                }
                newline = true;
                break;
            } else {
                line[li] = c;
                li++;
                if (li >= cap) {
                    cap += 64;
                    char *temp = realloc(line, cap);
                    if (temp == NULL) {
                        free(line);
                        ERROR("realloc failed!");
                        goto emergency_cleanup2;
                    }
                    line = temp;
                }
            }
        }
    }

    *pRowsCount = rowi;
    *pRowsCap = rowcap;
    return rowsBuffer;

emergency_cleanup2:
    for (int i = 0; i < rowi; i++)
        free(rowsBuffer[i].chars);
    free(rowsBuffer);
    fclose(file);
    return NULL;
}

void saveRowsAsFile(char *fileName, Row *_rows, int _rowsCount)
{

    int fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        ERROR("failed to open file!");
    }

    for (int i = 0; i < _rowsCount; i++)
    {
        Row r = _rows[i];

        if (r.length > 0)
        {
            ssize_t result = write(fd, r.chars, r.length);

            if (result == -1)
            {
                ERROR("failed to write file!");
            }
        }

        if (i < _rowsCount - 1)
        {
            ssize_t result = write(fd, "\n", 1);

            if (result == -1) {
                ERROR("failed to write file!");
            }
        }
    }

    close(fd);
}

char* getWorkingDirectory() {

    char *cwd = malloc(4096);

    if (cwd == NULL)
        return NULL;

    if (getcwd(cwd, 4096) == NULL)
    {
        free(cwd);
        return NULL;
    }

    return cwd;
}


int compareFilenames(const void *a, const void *b)
{
    const char *fileA = *(const char **)a;
    const char *fileB = *(const char **)b;

    return strcmp(fileA, fileB);
}

void sortFilenames(char **fileNames, int n)
{
    qsort(fileNames, n, sizeof(char *), compareFilenames);
}