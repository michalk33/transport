// Michał Kierul 322178

#ifndef HEAP_HPP
#define HEAP_HPP

void insert_heap(int* heap, int heap_cap, int& heap_size, int idx, int* heap_pos, long long* vals);
void remove_heap(int* heap, int& heap_size, int rem_pos, int* heap_pos, long long* vals);

#endif