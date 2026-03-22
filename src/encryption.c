#include "encryption.h"
#include "defines.h"


void public_key_to_id(const unsigned char *pubkey, size_t pubkey_len, unsigned char *unique_id, size_t id_size){
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(pubkey, pubkey_len, hash);
    memcpy(unique_id, hash, id_size);
}

int load_raw_key(const char *filename, unsigned char *buffer, size_t size) {
    FILE *fp = fopen(filename, "rb"); 
    if (!fp) {
        perror("File opening failed");
        return 0;
    }

    size_t bytes_read = fread(buffer, 1, size, fp);
    fclose(fp);

    if (bytes_read != size) {
        fprintf(stderr, "Error: Expected %zu bytes, but read %zu\n", size, bytes_read);
        return 0;
    }

    return 1;
}