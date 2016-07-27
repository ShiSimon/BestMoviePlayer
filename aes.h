/*
 * copyright (c) 2007 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef FFMPEG_AES_H
#define FFMPEG_AES_H

#include <stdint.h>

extern const int av_aes_size;

typedef struct AVAES{
    // Note: round_key[16] is accessed in the init code, but this only
    // overwrites state, which does not matter (see also r7471).
    uint8_t round_key[15][4][4];
    uint8_t state[2][4][4];
    int rounds;
}AVAES;

enum{
    AVAES_ENCRYPT = 0,
    AVAES_DECRYPT
};
/**
 * Initializes an AVAES context.
 * @param key_bits 128, 192 or 256
 * @param decrypt 0 for encryption, 1 for decryption
 */
int av_aes_init(struct AVAES *a, const uint8_t *key, int key_bits, int decrypt);

/**
 * Encrypts / decrypts.
 * @param count number of 16 byte blocks
 * @param dst destination array, can be equal to src
 * @param src source array, can be equal to dst
 * @param iv initialization vector for CBC mode, if NULL then ECB will be used
 * @param decrypt 0 for encryption, 1 for decryption
 */
void av_aes_crypt(struct AVAES *a, uint8_t *dst, uint8_t *src, int count, uint8_t *iv, int decrypt);

#endif /* FFMPEG_AES_H */
