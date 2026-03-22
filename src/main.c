#include "encryption.h"
#include "defines.h"
#include "interface.h"

struct interface_handler_s interface_handler;

int main(void){
    unsigned char pubkey[32];
    unsigned char id[10];
    //Load public key and generate a 14 byte hash to use as an ID
    load_raw_key("keys/public.pem", pubkey, CURVE25519_BYTE_LENGTH);
    public_key_to_id(pubkey, CURVE25519_BYTE_LENGTH, id, PUBLIC_ID_BYTE_SIZE );

    printf("Unique ID (hex): ");
    for (int i = 0; i < PUBLIC_ID_BYTE_SIZE; i++) {
        printf("%02x", id[i]);
    }
    printf("\n");

    read_interface_config(&interface_handler, "config/config");
    printf("\n--- Detected Interfaces: %u ---\n", interface_handler.interface_count);
    
    for (uint32_t i = 0; i < interface_handler.interface_count; i++) {
        interface *ptr = &interface_handler.interfaces[i];
        
        printf("[%d] Name: %s\n", i + 1, ptr->name);
        
        // Convert the enum to a string for the user
        const char* type_str = "Unknown";
        if (ptr->int_type == INTERFACE_SERIAL) type_str = "Serial";
        else if (ptr->int_type == INTERFACE_ETHERNET) type_str = "Ethernet";
        
        printf("    Type:  %s\n", type_str);
        printf("    Port:  %s\n", ptr->port);
        printf("    Speed: %u bps\n", ptr->speed);
        printf("-------------------------------\n");
    }
    return 0;
}