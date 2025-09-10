/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   extract.h
 *
 * Created on 9. září 2025, 12:23
 */

#ifndef EXTRACT_H
#define EXTRACT_H

#include "memfile.h"

#define DBI_XOR     0x37

typedef struct _BinaryPayload BinaryPayload;

typedef struct _BinaryPayload {
    uint32_t offset;
    MemFile * mf_enc;
    MemFile * mf_dec;
    BinaryPayload * next;
} BinaryPayload;

void bpl_free(BinaryPayload * bpl);

BinaryPayload * extract_list_files(MemFile * dbi);

int extract(const char * path_dbi, const char * path_out);

#endif /* EXTRACT_H */

