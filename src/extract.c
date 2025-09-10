/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdlib.h>
#include <zstd.h>

#include "extract.h"
#include "log.h"
#include "utils.h"

void bpl_free(BinaryPayload * bpl) {
    while(bpl) {
        BinaryPayload * next = bpl->next;
        mf_free(bpl->mf_dec);
        mf_free(bpl->mf_enc);
        free(bpl);
        bpl = next;
    }
}

BinaryPayload * bpl_init(MemFile * mf_dec, MemFile * mf_enc, uint32_t offset) {
    BinaryPayload * bpl = malloc(sizeof(BinaryPayload));
    bpl->offset = offset;
    bpl->mf_dec = mf_dec;
    bpl->mf_enc = mf_enc;
    bpl->next = NULL;
    return bpl;
}

BinaryPayload * extract_attempt_at(MemFile * dbi, uint32_t offset) {
    ZSTD_DStream* dstream = ZSTD_createDStream();
    if (!dstream) {
        return NULL;
    }

    uint32_t result = ZSTD_initDStream(dstream);
    if (ZSTD_isError(result)) {
        ZSTD_freeDStream(dstream);
        return NULL;
    }

    ZSTD_inBuffer input = { 
        .src = (const char*)dbi->data + offset, 
        .size = dbi->len - offset, 
        .pos = 0, 
    };
    
    uint32_t const outBufferSize = ZSTD_DStreamOutSize();
    void * outBuffer = malloc(outBufferSize);
    if (!outBuffer) {
        ZSTD_freeDStream(dstream);
        return NULL;
    }

    ZSTD_outBuffer output = { 
        .dst = outBuffer, 
        .size = outBufferSize, 
        .pos = 0,
    };

    int finished = 0;
    while(!finished) {
        uint32_t ret = ZSTD_decompressStream(dstream, &output, &input);
        if (ZSTD_isError(ret)) {
            break;
        }

        if (ret == 0) {
            finished = 1;
        }
        
        if (input.pos == dbi->len - offset) {
            break;
        }
    }

    BinaryPayload * ret = NULL;
    if(input.pos && output.pos) {
        MemFile * mf_enc = mf_init_mem(input.src, input.pos);
        MemFile * mf_dec = mf_init_mem(output.dst, output.pos);
        ret = bpl_init(mf_dec, mf_enc, offset);
    }

    free(outBuffer);
    ZSTD_freeDStream(dstream);

    return ret;
}

BinaryPayload * extract_list_files(MemFile * dbi) {    
    BinaryPayload * res = NULL;
    BinaryPayload * last = NULL;
    
    for(uint32_t i = 0; i < dbi->len; i++) {
        dbi->data[i] ^= DBI_XOR;
    }
    
    const uint8_t matches[] = {
        0x28, 0xB5, 0x2F, 0xFD
    };

    int state = 0;
    for(uint32_t i = 0; i < dbi->len; i++) {        
        if(dbi->data[i] == matches[0]) {
            state = 1;
        } else {
            switch(state) {
                case 1: case 2: case 3:
                    if(dbi->data[i] != matches[state]) {
                        state = 0;
                        break;
                    }

                    if(state++ == 3) {
                        state = 0;

                        BinaryPayload * bpl = extract_attempt_at(dbi, i - 3);
                        if(bpl != NULL) {
                            lf_i("loaded %8u B payload from %8u B at 0x%X", bpl->mf_dec->len, bpl->mf_enc->len, bpl->offset);
                            
                            if(res == NULL) {
                                res = bpl;
                            } else {
                                last->next = bpl;
                            }
                            
                            last = bpl;
                        }
                    }
                    break;
                    
                default:
                    state = 0;
                    break;
            }
        }
    }
    
    
    for(uint32_t i = 0; i < dbi->len; i++) {
        dbi->data[i] ^= DBI_XOR;
    }
    
    return res;
}

int extract(const char * path_dbi, const char * path_out) {
    lf_n("extracting '%s' to '%s'", path_dbi, path_out);
    
    if(mkpath(0755, "%s/", path_out) != 0) {
        lf_e("failed to create directory '%s'", path_out);
        return EXIT_FAILURE;
    }
    
    MemFile * dbi = mf_init_path(path_dbi);
    if(!dbi || dbi->len == 0) {
        lf_e("failed to load '%s'", path_dbi);
        
        mf_free(dbi);
        return EXIT_FAILURE;
    }
    
    lf_i("loaded %u B from '%s'", dbi->len, path_dbi);
    
    mf_write(dbi, "/tmp/repack.bin");
    
    uint32_t idx = 0;
    BinaryPayload * mfl = extract_list_files(dbi);
    BinaryPayload * mfl_iter = mfl;
    while(mfl_iter) {
        lf_i("extracted rec%u.bin: %8u B", idx, mfl_iter->mf_dec->len);
        
        mf_write(mfl_iter->mf_dec, "%s/rec%u.bin", path_out, idx);
        
        idx++;
        mfl_iter = mfl_iter->next;
    }
    
    bpl_free(mfl);
    mf_free(dbi);

    return EXIT_SUCCESS;
}