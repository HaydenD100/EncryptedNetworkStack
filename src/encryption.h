#ifndef ENCRYPTION_H
#define ENCRYPTION_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/sha.h>

void public_key_to_id(const unsigned char *pubkey, size_t pubkey_len, unsigned char *unique_id, size_t id_size);
int load_raw_key(const char *filename, unsigned char *buffer, size_t size);

#endif