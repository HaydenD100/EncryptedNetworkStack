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