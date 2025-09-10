/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "memfile.h"

MemFile * mf_init_path(const char * path) {
    FILE * f = fopen(path, "r");
    if(!f) {
        return NULL;
    }
    
    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    MemFile * mf = malloc(sizeof(MemFile));
    mf->data = malloc(len);
    mf->len = len;

    fread(mf->data, 1, mf->len, f);
    fclose(f);
    
    return mf;
}

MemFile * mf_init_mem(const void * data, uint32_t len) {
    if(!(data && len)) {
        return NULL;
    }
    
    MemFile * mf = malloc(sizeof(MemFile));
    mf->data = malloc(len);
    mf->len = len;
    
    memcpy(mf->data, data, len);
    
    return mf;
}

void mf_free(MemFile * mf) {
    if(mf) {
        if(mf->data) {
            free(mf->data);
        }
        
        free(mf);
    }
}

void mf_write(MemFile * mf, const char * fmt, ...) {
    char * path = malloc(FILENAME_MAX);
    
    va_list args;
    va_start(args, fmt);
    vsnprintf(path, FILENAME_MAX, fmt, args);
    va_end(args);
    
    FILE * f = fopen(path, "w+");
    free(path);
    
    if(!f) {
        return;
    }
    
    fwrite(mf->data, 1, mf->len, f);
    fflush(f);
    fclose(f);
}
