/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   memfile.h
 *
 * Created on 9. září 2025, 12:24
 */

#ifndef MEMFILE_H
#define MEMFILE_H

#include <stdint.h>

typedef struct {
    uint8_t * data;
    uint32_t len;
} MemFile;

MemFile * mf_init_path(const char * path);

MemFile * mf_init_mem(const void * data, uint32_t len);

void mf_free(MemFile * mf);

void mf_write(MemFile * mf, const char * fmt, ...);

#endif /* MEMFILE_H */

