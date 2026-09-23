#pragma once

#define LINENUM_W 5
#define FILESYS_W 38
#define BAR_H 2

#define MIN_CURSOR_Y_SIDE_DISTANCE 3

#define MAX_RENAME_LENGTH 1024
#define MAX_SYMBOL_POS_COUNT 4096

#define TAB_SIZE 4

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#define ERROR(s) fprintf(stderr, "Error at %s:%d: %s", __FILE__, __LINE__, s); fflush(stderr)
#define ERROR_EXIT(s) fprintf(stderr, "Error at %s:%d: %s", __FILE__, __LINE__, s); fflush(stderr); exit(EXIT_FAILURE)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct
{
    char *chars;
    int length;
    int cap;
} Row;

typedef struct
{
    int x1;
    int y1;
    int x2;
    int y2;
} Selection;

typedef struct
{
    int x;
    int y;
} Pos;