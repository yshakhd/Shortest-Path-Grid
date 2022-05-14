#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "dijkstra.h"

static void free_grid(short** grid, short r) {
    for(short i = 0; i < r; i++)
        free(grid[i]);
    free(grid);
}

int main(int argc, char** argv) {
    if(argc != 5)
        return EXIT_FAILURE;
    short r, c;
    short** grid = read_write_grid(argv[1], argv[2], &r, &c);
    if(!grid)
        return EXIT_FAILURE;

    FILE* fptr = fopen(argv[3], "wb");
    if(!fptr) {
        free_grid(grid, r);
        return EXIT_FAILURE;
    }
    FILE* fptr2 = fopen(argv[4], "wb");
    if(!fptr2) {
        free_grid(grid, r);
        fclose(fptr);
        return EXIT_FAILURE;
    }
    StartNode* path_head = best_route(grid, r, c, fptr, fptr2);
    write_path(path_head, fptr2);

    fclose(fptr);
    fclose(fptr2);
    while(path_head) {
        StartNode* temp = path_head;
        path_head = path_head -> next;
        free(temp);
    }
    free_grid(grid, r);

    return EXIT_SUCCESS;
}