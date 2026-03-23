#ifndef INTERFACE_H
#define INTERFACE_H

#include "defines.h"

#define WIN32_LEAN_AND_MEAN
#ifdef _WIN32
    #include <windows.h>
    #define WIN32_LEAN_AND_MEAN
#else
    #include <fcntl.h> 
    #include <errno.h> 
    #include <termios.h>
    #include <unistd.h> 
#endif

typedef enum interface_type_e{
    INTERFACE_NONE,
    INTERFACE_SERIAL,
    INTERFACE_ETHERNET
} interface_type;



typedef struct interface_s{
    char name[64];
    interface_type int_type;
    //Interface Specfications
    char port[27];
    u32 speed;
    #ifdef _WIN32
        HANDLE hSerial;
    #else 
        int fd;
    #endif
    
} interface;

struct interface_handler_s{
    u32 interface_count;
    interface interfaces[MAX_INTERFACES];
};

typedef int (*ini_handler)(void*,const char*, const char*, const char*, const char*);
interface_type string_to_interface(const char* type_str);
int config_handler(void* interface_handler, const char* section, const char* sub_section, const char* name, const char* value);

char* trim(char* s);
int ini_parse(const char* filename, ini_handler handler, void* user);
int read_interface_config(struct interface_handler_s *interface_handler, const char* file_path);

int init_interfaces(struct interface_handler_s *interface_handler);
int shutdown_interfaces(struct interface_handler_s *interface_handler);

int init_serial(interface *serial_interface);
int write_serial(interface *serial_interface, char buffer[255], u32 buffersize);
int close_serial(interface *serial_interface);

#endif