#ifndef DEFINES_H
#define DEFINES_H

#include <stdint.h>

typedef uint8_t  u8;
typedef int8_t   i8;
typedef uint16_t u16;
typedef int16_t  i16;
typedef uint32_t u32;
typedef int32_t  i32;
typedef uint64_t u64;
typedef int64_t  i64;

#define PUBLIC_ID_BYTE_SIZE 14
#define SHA256_DIGEST_LENGTH 32
#define CURVE25519_BYTE_LENGTH 32
#define MAX_INTERFACES 32

enum __attribute__((packed)) message_type_e : u8 {
    ERROR = 0x01,
    PUBLIC_KEY = 0x02, //Might not be used
    MESSAGE = 0x03,
    ANOUCEMENT = 0x04
};


#endif

