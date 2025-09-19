/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <zstd.h>
#include <stdio.h>
#include <string.h>

#include "patch.h"
#include "log.h"
#include "utils.h"
#include "memfile.h"
#include "extract.h"

static int compress_mf_inplace(MemFile * in, int compressionLevel) {
    int ret = EXIT_FAILURE;
    
    ZSTD_CStream * cstream = ZSTD_createCStream();
    ZSTD_initCStream(cstream, compressionLevel);

    // I guess we can be reasonably safe to tell that compressed file wont be 
    // larger than plain text input
    const uint32_t buff_out_sz = in->len;
    void* buff_out = malloc(buff_out_sz);
    memset(buff_out, 0, buff_out_sz);
    
    if(!buff_out) {
        goto cleanup;
    }

    lf_i("compress %u B", in->len);
    
    ZSTD_inBuffer input = { 
        .src = in->data, 
        .size = in->len, 
        .pos = 0,
    };
    
    ZSTD_outBuffer output = { 
        .dst = buff_out, 
        .size = buff_out_sz, 
        .pos = 0,
    };

    uint8_t ended = 0;
    while(1) {
        if(input.pos == input.size) {
            ended = 1;
        }

        if(!ended) {
            ret = ZSTD_compressStream(cstream, &output, &input);
        } else {
            ret = ZSTD_endStream(cstream, &output);
        }
        
        if(ZSTD_isError(ret)) {
            goto cleanup;
        }

        if(ret == 0 && ended) {
            break;
        }
    }
    
    if(output.pos) {
        memcpy(in->data, output.dst, buff_out_sz);
        in->len = output.pos;
    }
    
    ret = EXIT_SUCCESS;
    
cleanup:
    free(buff_out);
    ZSTD_freeCStream(cstream);
    return ret;
}

int patch(const char * dbi, const char * patch, uint32_t slot, const char * path_out) {
    lf_n("patching '%s' slot %u with '%s' to '%s'", dbi, slot, patch, path_out);
    
    if(mkpath(0755, "%s", path_out) != 0) {
        lf_e("failed to create directory '%s'", path_out);
        return EXIT_FAILURE;
    }
    
    MemFile * mf_patch = mf_init_path(patch);
    if(!mf_patch || mf_patch->len == 0) {
        lf_e("failed to load '%s'", patch);
        mf_free(mf_patch);
        return EXIT_FAILURE;
    }
    
    MemFile * mf_dbi = mf_init_path(dbi);
    if(!mf_dbi || mf_dbi->len == 0) {
        lf_e("failed to load '%s'", dbi);
        mf_free(mf_patch);
        mf_free(mf_dbi);
        return EXIT_FAILURE;
    }
    
    BinaryPayload * bpl_dbi = extract_list_files(mf_dbi);
    if(!bpl_dbi) {
        lf_e("failed to load slots");
        mf_free(mf_patch);
        mf_free(mf_dbi);
        return EXIT_FAILURE;
    }
    
    BinaryPayload * bpl_iter = bpl_dbi;
    for(uint32_t i = 0; i < slot; i++) {
        if(!bpl_iter->next) {
            lf_e("failed to locate slot %u", slot);
            mf_free(mf_patch);
            mf_free(mf_dbi);
            return EXIT_FAILURE;
        }
        
        bpl_iter = bpl_iter->next;
    }

    int ret = EXIT_FAILURE;
    
    ret = compress_mf_inplace(mf_patch, 22);
    
    if(ret != EXIT_FAILURE) {
        if(mf_patch->len <= bpl_iter->mf_enc->len) {            
            lf_i("patched %u B/ %u B", mf_patch->len, bpl_iter->mf_enc->len);
            for(uint32_t i = 0; i < mf_patch->len; i++) {
                mf_patch->data[i] ^= DBI_XOR;
            }
            
            memcpy(&mf_dbi->data[bpl_iter->offset], mf_patch->data, mf_patch->len);
            
            mf_write(mf_dbi, "%s", path_out);
            ret = EXIT_SUCCESS;
        } else {
            lf_e("patch does nor fit %u B/ %u B", mf_patch->len, bpl_iter->mf_enc->len);;
            ret = EXIT_FAILURE;
        }
    }
    
    bpl_free(bpl_dbi);
    mf_free(mf_patch);
    mf_free(mf_dbi);
    
    return ret;
}