// Michał Kierul 322178

#include "heap.hpp"
#include "utils.hpp"

void insert_heap(int* heap, int heap_cap, int& heap_size, int idx, int* heap_pos, long long* vals){
    if(heap_cap == heap_size)
        ERROR("heap insert error");
    int pos = heap_size;
    int up_pos = (pos-1)/2;
    heap[pos] = idx;
    heap_size++;
    heap_pos[idx] = pos;
    while(pos > 0){
        if(vals[heap[up_pos]] > vals[idx]){
            heap[pos] = heap[up_pos];
            heap[up_pos] = idx;
            heap_pos[heap[pos]] = pos;
            heap_pos[idx] = up_pos;
            pos = up_pos;
            up_pos = (pos-1)/2;
        }else{
            break;
        }
    }
}

void remove_heap(int* heap, int& heap_size, int rem_pos, int* heap_pos, long long* vals){
    if(heap_size <= 0)
        ERROR("heap pop error");
    heap_pos[heap[rem_pos]] = -1;
    int pos = rem_pos;
    int lp, rp, up;
    heap_size--;
    heap[rem_pos] = heap[heap_size];
    heap_pos[heap[rem_pos]] = rem_pos;
    int tmp;
    for(;;){
        up = (pos-1)/2;
        lp = pos*2+1;
        rp = pos*2+2;
        if(pos > 0 && vals[heap[up]] > vals[heap[pos]]){
            tmp = heap[pos];
            heap[pos] = heap[up];
            heap[up] = tmp;
            heap_pos[heap[pos]] = pos;
            heap_pos[heap[up]] = up;
            pos = up;
        }else if(lp < heap_size && rp >= heap_size && vals[heap[lp]] < vals[heap[pos]]){
            tmp = heap[pos];
            heap[pos] = heap[lp];
            heap[lp] = tmp;
            heap_pos[heap[pos]] = pos;
            heap_pos[heap[lp]] = lp;
            break;
        }else if(rp < heap_size && ( vals[heap[lp]] < vals[heap[pos]] || vals[heap[rp]] < vals[heap[pos]]) ){
            if(vals[heap[lp]] < vals[heap[rp]]){
                tmp = heap[pos];
                heap[pos] = heap[lp];
                heap[lp] = tmp;
                heap_pos[heap[pos]] = pos;
                heap_pos[heap[lp]] = lp;
                pos = lp;
            }else{
                tmp = heap[pos];
                heap[pos] = heap[rp];
                heap[rp] = tmp;
                heap_pos[heap[pos]] = pos;
                heap_pos[heap[rp]] = rp;
                pos = rp;
            }
        }else{
            break;
        }
    }
}