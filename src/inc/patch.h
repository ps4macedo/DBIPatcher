/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   patch.h
 *
 * Created on 9. září 2025, 16:35
 */

#ifndef PATCH_H
#define PATCH_H

#include <stdint.h>

int patch(const char * dbi, const char * patch, uint32_t slot, const char * output);

#endif /* PATCH_H */

