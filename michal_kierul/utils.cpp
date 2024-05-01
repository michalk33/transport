// Michał Kierul 322178

#include "utils.hpp"

void ERROR(const char* str){
    fprintf(stderr, "%s: %s\n", str, strerror(errno));
    exit(EXIT_FAILURE);
}

int write_int_to_char(char* dest, int x, int pos){
    char tmp[20];
    int l = 0;
    if(x == 0){
        dest[pos] = '0';
        return pos+1;
    }
    for(;x > 0;){
        tmp[l] = (x%10) + '0';
        l++;
        x /= 10;
    }
    for(int i = 0; i < l; i++)
        dest[pos+i]=tmp[l-1-i];
    return pos+l;
}

int read_int(u_int8_t* src, int& pos){
    int ret = 0;
    for(;src[pos] >= '0' && src[pos] <= '9';){
        ret *= 10;
        ret += src[pos] - '0';
        pos++;
    }
    return ret;
}


bool is_number(char* x){
    for(int i = 0;;i++){
        if(x[i] == '\n' || x[i] == '\0')
            return true;
        if(!(x[i] >= '0' && x[i] <= '9'))
            return false;
    }
    return false;
}
