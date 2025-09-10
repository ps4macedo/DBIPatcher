/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "convert.h"
#include "log.h"
#include "utils.h"
#include "memfile.h"

#define MAX_TEXT_LEN    1024
#define TERM_NULLS      3

static void convert_keys_placeholders(FILE * out_keys, const char * str, uint32_t len) {
    for(uint32_t i = 0; i < len; i++) {
        switch(str[i]) {
            case '{':
            case '}':
                fwrite(&str[i], 1, 1, out_keys);
                break;
            default:
                break;
        }
    }
    fwrite("\n", 1, 1, out_keys);
}

static int convert_unpack(MemFile * in, FILE * out, FILE * out_keys) {
    uint32_t len = 0;
    uint32_t nulls = 0;
    
    for(uint32_t i = 0; i < in->len; i++) {
        if(in->data[i] == 0) {
            if(++nulls == TERM_NULLS) {
                // we want to leave the final null
                len = (i + 1) - (TERM_NULLS - 1);
                break;
            }
        } else {
            nulls = 0;
        }
    }

    if(in->len - len != TERM_NULLS - 1) {
        lf_e("valid terminating sequence not found");
        return EXIT_FAILURE;
    }
    
    lf_i("adjusted length %u B to %u B", in->len, len);
    
    char * start = (char*)in->data;
    char * ptr = start;
    
    uint32_t key_cnt = 0;
    uint8_t key = 1;
    while(ptr - start < len) {
        uint32_t cur_len = strlen(ptr);
        
        fwrite(ptr, 1, cur_len, out);
        
        if(key++ & 1) {
            key_cnt++;
            fwrite("=", 1, 1, out);
            
            if(out_keys) {
                fwrite(ptr, 1, cur_len, out_keys);
                fwrite(";", 1, 1, out_keys);
            }
        } else {
            if(out_keys) {
                convert_keys_placeholders(out_keys, ptr, cur_len);
            }
            
            fwrite("\n", 1, 1, out);
        }
        
        ptr += cur_len + 1;
    }
    
    lf_i("converted %u keys", key_cnt);
    
    return EXIT_SUCCESS;
}

static int convert_pack(MemFile * in, FILE * out, FILE * out_keys) {
    uint32_t key_cnt = 0;
    uint32_t line_start = 0;
    uint8_t key_found = 0;
    uint8_t * val_start = NULL;
    for(uint32_t i = 0; i < in->len; i++) {

        switch(in->data[i]) {
            case '\n':
                key_found = 0;
                line_start = i;
                break;
            case '=':
                if(!key_found) {
                    key_cnt++;

                    if(line_start != 0) {
                        in->data[line_start] = 0; 
                    }
                    in->data[i] = 0;
                    key_found = 1;
                    
                    if(val_start && out_keys) {
                        convert_keys_placeholders(out_keys, (const char *)val_start, strlen((const char *)val_start));
                    }
                    
                    val_start = &in->data[i + 1];
                    
                    if(out_keys) {                        
                        const char * start = (const char *)&in->data[line_start];
                        if(line_start) {
                            start++;
                        }
                        
                        fwrite(start, 1, strlen(start), out_keys);
                        fwrite(";", 1, 1, out_keys);
                    }
                }
                break;
            default:
                break;
        }
    }
    
    // last value
    if(line_start != 0) {
        in->data[line_start] = 0; 
        
        if(val_start && out_keys) {
            convert_keys_placeholders(out_keys, (const char *)val_start, strlen((const char *)val_start));
        }
    }
    
    lf_i("converted %u keys", key_cnt);
    
    fwrite(in->data, 1, in->len, out);
    for(uint32_t i = 0; i < (TERM_NULLS - 1); i++) {
        fwrite("", 1, 1, out);
    }
    
    return EXIT_SUCCESS;
}

int convert(const char * path_in, const char * path_out, const char * path_out_keys) {
    lf_n("converting '%s' to '%s'", path_in, path_out);
    
    if(mkpath(0755, "%s", path_out) != 0) {
        lf_e("failed to create directory '%s'", path_out);
        return EXIT_FAILURE;
    }
    
    if(path_out_keys) {
        if(mkpath(0755, "%s", path_out_keys) != 0) {
            lf_e("failed to create directory '%s'", path_out_keys);
            return EXIT_FAILURE;
        }
    }
    
    MemFile * in = mf_init_path(path_in);
    if(!in || in->len == 0) {
        lf_e("failed to load '%s'", path_in);
        mf_free(in);
        return EXIT_FAILURE;
    }
    
    FILE * dst = fopen(path_out, "w+");
    if(!dst) {
        lf_e("failed to open '%s' for writing", path_out);
        mf_free(in);
        return EXIT_FAILURE;
    }
    
    FILE * dst_keys = NULL;
    
    if(path_out_keys) {
        dst_keys = fopen(path_out_keys, "w+");
        if(!dst) {
            lf_e("failed to open '%s' for writing", path_out_keys);
            mf_free(in);
            fclose(dst);
            return EXIT_FAILURE;
        } else {
            lf_i("will store keys in '%s'", path_out_keys);
        }
    }
    
    int ret = EXIT_SUCCESS;
    
    uint8_t pack = 1;
    for(uint32_t i = 0; i < in->len; i++) {
        if(in->data[i] == 0) {
            pack = 0;
            break;
        }
    }
    
    if(pack) {
        lf_i("'%s' does not contain NULL, assume text->bin", path_in);
        ret = convert_pack(in, dst, dst_keys);
    } else {
        lf_i("'%s' contains NULL, assume bin->text", path_out);
        ret = convert_unpack(in, dst, dst_keys);
    }
    
    mf_free(in);
    fflush(dst);
    fclose(dst);
    
    if(dst_keys) {
        fflush(dst_keys);
        fclose(dst_keys);
    }
    
    if(ret == EXIT_SUCCESS) {
        lf_i("convert done");
    }
    
    return ret;
}