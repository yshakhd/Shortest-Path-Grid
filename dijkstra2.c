#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "dijkstra.h"

short** read_write_grid(const char* path, const char* dest, short* r, short* c) {
    FILE * fptr = fopen(path, "rb");
	if(!fptr)
		return NULL;

    FILE * fptr2 = fopen(dest, "wb");
    if(!fptr2)
        return NULL;

    short rows, cols;
	fread(&rows, sizeof(short), 1, fptr);
    fread(&cols, sizeof(short), 1, fptr);
    fprintf(fptr2, "%hd %hd\n", rows, cols);
    *r = rows + 1;
    *c = cols;
    short** grid = malloc(sizeof(short *) * (rows + 1));
	for(short i = 0; i < *r; i++)
		grid[i] = malloc(sizeof(short) * (*c));
    for(short i = 0; i < rows; i++) {
        for(short j = 0; j < cols; j++) {
            short num;
            fread(&num, sizeof(short), 1, fptr);
            grid[i][j] = num;
            fprintf(fptr2, "%hd", num);
            if(j == cols - 1) {
                fprintf(fptr2, "\n");
            } else {
                fprintf(fptr2, " ");
            }
        }
    }
    memset(grid[rows], 0, cols * sizeof(grid[rows][0])); // bottom row of 0's serves as a prestarter node for dijkstra
    fclose(fptr);
    fclose(fptr2);
    return grid;
}

static void heapify_up(int* pq, int* times, int* index, int idx) {
    idx = index[idx];
    int curr = pq[idx];
    int parent = (idx - 1) / 2;
    while(idx && times[pq[parent]] > times[pq[idx]]) {
        int temp = pq[idx];
        pq[idx] = pq[parent];
        pq[parent] = temp;
        index[pq[idx]] = idx;
        index[pq[parent]] = parent;
        idx = parent;
        parent = (idx - 1) / 2;
    }
    pq[idx] = curr;
}

static void check_neighbor(int dir, int curr, int* times, int* index, int* parents, int* pq, short** grid, int heap_size, int cols) {
    if((heap_size > index[dir]) && (times[dir] > (times[curr] + grid[dir / cols][dir % cols]))) {
        times[dir] = times[curr] + grid[dir / cols][dir % cols];
        parents[dir] = curr;
        heapify_up(pq, times, index, dir);
    }
}

static StartNode* stack(StartNode** path_head_a, StartNode* path_tail, StartNode* new_node) {
    if(!path_tail)
        *path_head_a = new_node;
    else
        path_tail -> next = new_node;
    return new_node;
}

void write_path(StartNode* path_head, FILE* fptr) {
    StartNode* node = path_head;
    while(node) {
		fwrite(&(node -> r), sizeof(short), 1, fptr);
		fwrite(&(node -> c), sizeof(short), 1, fptr);
		node = node -> next;
	}
}

static void heapify_down(int* pq, int* times, int* index, int heap_size, int idx) {
    idx = index[idx];
    int shortest = idx;
    int if_child = 2 * shortest + 1;
    while(if_child < heap_size) {
        int left = 2 * shortest + 1;
        int right = 2 * shortest + 2;
        if_child = left;
        if(left < heap_size && times[pq[shortest]] > times[pq[left]])
            shortest = left;
        if(right < heap_size && times[pq[shortest]] > times[pq[right]])
            shortest = right;
        if(shortest != idx) {
            int temp = pq[idx];
            pq[idx] = pq[shortest];
            pq[shortest] = temp;
            index[pq[shortest]] = shortest;
            index[pq[idx]] = idx;
            idx = shortest;
        } else break;
    }
}

static int get_quickest_tile(int* pq, int* times, int* index, int heap_size) {
    int top = pq[0];
    pq[0] = pq[heap_size];
    pq[heap_size] = top;
    index[top] = heap_size;
    index[pq[0]] = 0;
    heapify_down(pq, times, index, heap_size, pq[0]);
    return top;
}

StartNode* best_route(short ** grid, int r, int c, FILE* fptr, FILE* fptr2) {
	int heap_size = r * c;
	
    int* times = malloc(sizeof(int) * heap_size);
    int* index = malloc(sizeof(int) * heap_size);
    int* parents = malloc(sizeof(int) * heap_size);
    int* pq = malloc(sizeof(int) * heap_size);

	for(int i = 0; i < heap_size; i++) {
		times[i] = INT_MAX;
		parents[i] = -1;
		pq[i] = i;
		index[i] = i;
	}

	index[heap_size - c] = 0;
	index[0] = heap_size - c;
	times[heap_size - c] = 0;
	pq[heap_size - c] = 0;
	pq[0] = heap_size - c;
	
	while(heap_size) {
        heap_size--;
		int curr = get_quickest_tile(pq, times, index, heap_size);
		if(curr / c) {
            // up
			int up = curr - c;
            check_neighbor(up, curr, times, index, parents, pq, grid, heap_size, c);
		}
		if(curr / c + 1 < r) {
            // down
			int down = curr + c;
            check_neighbor(down, curr, times, index, parents, pq, grid, heap_size, c);
		}
		if(curr % c) {
            // left
			int left = curr - 1;
            check_neighbor(left, curr, times, index, parents, pq, grid, heap_size, c);
		}
		if(curr % c + 1 < c) {
            // right
			int right = curr + 1;
            check_neighbor(right, curr, times, index, parents, pq, grid, heap_size, c);
		}
	}
	
	int start_pos = 0;
	int fastest_time = times[0];
    fwrite(&c, sizeof(short), 1, fptr);
	for(int i = 0; i < c; i++) {
        fwrite(&(times[i]), sizeof(int), 1, fptr);
		if(times[i] < fastest_time) {
            start_pos = i;
			fastest_time = times[i];
		}
	}
	fwrite(&fastest_time, sizeof(int), 1, fptr2);
	
	StartNode * path_head = NULL;
	StartNode * path_tail = NULL;
	
	int pos = start_pos;
	int locations = 0;
	int p;
	for(int i = 0; i + 1 != r; i = p / c) {
    	locations++;
		StartNode* new_node = malloc(sizeof(StartNode));
		new_node -> r = i;
		new_node -> c = pos;
		new_node -> next = NULL;
        path_tail = stack(&path_head, path_tail, new_node);
		p = parents[i * c + pos];
		pos = p % c;
	}
    fwrite(&locations, sizeof(int), 1, fptr2);

    free(pq);
    free(index);
    free(times);
    free(parents);
    return path_head;
}