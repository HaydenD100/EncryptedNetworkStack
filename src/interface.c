#include "interface.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>



char* trim(char* s) {
    char* end;
    if (*s == 0) return s;
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        end--;
    }
    end[1] = '\0';
    return s;
}

int ini_parse(const char* filename, ini_handler handler, void* user) {
    FILE* file = fopen(filename, "r");
    if (!file){ 
        printf("Could not read config file \n");
        return -1;
    }

    char line[512];
    char section[64] = "";
    char sub_section[64] = "";

    while (fgets(line, sizeof(line), file)) {
        char* start = trim(line);
        if (*start == ';' || *start == '#' || *start == '\0') continue; 

        if (*start == '[' && strchr(start, ']')) {
            strncpy(section, start + 1, sizeof(section) - 1);
            char* end = strchr(section, ']');
            if (end) *end = '\0';
        } 
        else if (*start == ' ' && *(start + 1) == '[' && strchr(start, ']')) {
            strncpy(sub_section, start + 2, sizeof(sub_section) - 1);
            char* end = strchr(sub_section, ']');
            if (end) *end = '\0';
        } else {
            char* end = strchr(start, '=');
            if (end) {
                *end = '\0';
                char* name = trim(start);
                char* value = trim(end + 1);
                if (!handler(user, section,sub_section, name, value)) break;
            }
        }
    }
    fclose(file);
    return 0;
}

static interface *find_interface(struct interface_handler_s *interface_handler, const char *interface_name){
    for(int i = 0; i < interface_handler->interface_count; i++){
        if(strcmp(interface_handler->interfaces[i].name, interface_name) == 0){
            return &interface_handler->interfaces[i];
        }
    }
    return NULL;
}

interface_type string_to_interface(const char* type_str) {
    if (type_str == NULL) return INTERFACE_NONE;

    if (strcasecmp(type_str, "SerialInterface") == 0) {
        return INTERFACE_SERIAL;
    } 
    else if (strcasecmp(type_str, "EthernetInterface") == 0) {
        return INTERFACE_ETHERNET;
    }

    return INTERFACE_NONE;
}

int config_handler(void* interface_handler, const char* section, const char* sub_section, const char* name, const char* value) {
    struct interface_handler_s *interface_handler_ptr = (struct interface_handler_s *)interface_handler;
    if(interface_handler_ptr == NULL){
        return 0;
    }

    if(strcmp(section,"interfaces") == 0){
        interface *selected_interface = find_interface(interface_handler_ptr, sub_section);
        if(selected_interface == NULL && interface_handler_ptr->interface_count < MAX_INTERFACES){
            interface_handler_ptr->interface_count++;
            selected_interface = &interface_handler_ptr->interfaces[interface_handler_ptr->interface_count - 1];
            strncpy(selected_interface->name, sub_section, sizeof(selected_interface->name) - 1);        
        }
        printf("%s\n", value);
        if(strcmp(name + 1, "Type") == 0){
            selected_interface->int_type = string_to_interface(value + 1);
        }
        if(strcmp(name + 1, "Port") == 0){
            strncpy(selected_interface->port, value + 1, sizeof(selected_interface->port) - 1);        }
        if(strcmp(name + 1, "Speed") == 0){
            selected_interface->speed = (u32)strtoul(value + 1, NULL, 10);
        }
    }
    return 1;
}

int read_interface_config(struct interface_handler_s *interface_handler, const char* file_path){
    ini_parse(file_path, config_handler, interface_handler);
}
#ifdef _WIN32

int init_serial(interface *serial_interface){
    DCB dcbSerialParams = {0};
    char portName[32];

    snprintf(portName, sizeof(portName), "\\\\.\\%s", serial_interface->port);
    serial_interface->hSerial = CreateFile(portName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (serial_interface->hSerial == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Could not open %s. Error code: %lu\n", portName, GetLastError());
        return -1;
    }

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (GetCommState(serial_interface->hSerial, &dcbSerialParams)) {
        dcbSerialParams.BaudRate = serial_interface->speed;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity   = NOPARITY;
        SetCommState(serial_interface->hSerial, &dcbSerialParams);
    }
    return 0;
}

int write_serial(interface *serial_interface, char buffer[255], u32 buffersize){
    DWORD bytesWritten;
    if (!WriteFile(serial_interface->hSerial, buffer, (DWORD)buffersize, &bytesWritten, NULL)) {
        fprintf(stderr, "Write failed. Error: %lu\n", GetLastError());
        CloseHandle(serial_interface->hSerial);
        return -1;
    }

    return (int)bytesWritten;
}
int close_serial(interface *serial_interface){
    CloseHandle(serial_interface->hSerial);
}

#else
    int init_serial(interface *serial_interface) {
        struct termios tty;
        char portPath[300];

        if (serial_interface->port[0] == '/') {
            snprintf(portPath, sizeof(portPath), "%s", serial_interface->port);
        } else {
            snprintf(portPath, sizeof(portPath), "/dev/%s", serial_interface->port);
        }

        serial_interface->fd = open(portPath, O_RDWR | O_NOCTTY);

        if (serial_interface->fd < 0) {
            fprintf(stderr, "Error %i from open: %s\n", errno, strerror(errno));
            return -1;
        }

        if (tcgetattr(serial_interface->fd, &tty) != 0) {
            fprintf(stderr, "Error %i from tcgetattr: %s\n", errno, strerror(errno));
            return -1;
        }

        speed_t baud;
        switch (serial_interface->speed) {
            case 9600:   baud = B9600;   break;
            case 19200:  baud = B19200;  break;
            case 38400:  baud = B38400;  break;
            case 57600:  baud = B57600;  break;
            case 115200: baud = B115200; break;
            default:     baud = B9600;   break;
        }
        
        cfsetospeed(&tty, baud);
        cfsetispeed(&tty, baud);

        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;
        tty.c_cflag &= ~CRTSCTS;
        tty.c_cflag |= CREAD | CLOCAL;

        tty.c_lflag &= ~ICANON;
        tty.c_lflag &= ~ECHO;
        tty.c_lflag &= ~ECHOE;
        tty.c_lflag &= ~ECHONL;
        tty.c_lflag &= ~ISIG;
        
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);
        tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
        
        tty.c_oflag &= ~OPOST;
        tty.c_oflag &= ~ONLCR;

        tty.c_cc[VTIME] = 10;
        tty.c_cc[VMIN] = 0;

        if (tcsetattr(serial_interface->fd, TCSANOW, &tty) != 0) {
            fprintf(stderr, "Error %i from tcsetattr: %s\n", errno, strerror(errno));
            return -1;
        }

        return 0;
    }
    int write_serial(interface *serial_interface, char buffer[255], u32 buffersize) {
        int bytesWritten = write(serial_interface->fd, buffer, buffersize);
        
        if (bytesWritten < 0) {
            fprintf(stderr, "Write failed. Error: %s\n", strerror(errno));
            return -1;
        }

        return bytesWritten;
    }

    int close_serial(interface *serial_interface) {
        if (serial_interface->fd >= 0) {
            return close(serial_interface->fd);
        }
        return 0;
    }
#endif

int init_interfaces(struct interface_handler_s *interface_handler){
    for(int i = 0; i < interface_handler->interface_count; i++){
        if(interface_handler->interfaces[i].int_type == INTERFACE_SERIAL){
            int result = init_serial(&interface_handler->interfaces[i]);
            if(result != 0){
                printf("Error Initing Interface %s\n",interface_handler->interfaces[i].name);
            }else{
                printf("Succesfully Inited Interface %s\n",interface_handler->interfaces[i].name);
            }
        }
    }
}

int shutdown_interfaces(struct interface_handler_s *interface_handler){
    for(int i = 0; i < interface_handler->interface_count; i++){
        if(interface_handler->interfaces[i].int_type == INTERFACE_SERIAL){
            close_serial(&interface_handler->interfaces[i]);
        }
    }
}