#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct _StartNode {
    int r;
    int c;
    struct _StartNode* next;
} StartNode;

short** read_write_grid(const char* path, const char* dest, short* r, short* c);
StartNode* best_route(short** grid, int r, int c, FILE* fptr, FILE* fptr2);
void write_path(StartNode* path_head, FILE* fptr2);