// Michał Kierul 322178

#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

void ERROR(const char* str);
int write_int_to_char(char* dest, int x, int pos);
int read_int(u_int8_t* src, int& pos);
bool is_number(char* x);

#endif
