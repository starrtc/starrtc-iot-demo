#ifndef _G711_H_
#define _G711_H_
#include <stdio.h>

typedef unsigned char byte;

void g711_init();
void g711_encode(byte * pcm, size_t pcm_size, byte * g711);
void g711_decode(byte * g711, size_t g711_size, byte * pcm);
#endif