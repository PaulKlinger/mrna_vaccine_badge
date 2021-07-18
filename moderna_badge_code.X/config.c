#include "config.h"

#include "avr/eeprom.h"


typedef struct { 
    struct config cfg;
    uint8_t checksum;
} StoredConfig;
EEMEM StoredConfig stored_config;

struct config default_config = {
        .sequence = MODERNA
};

uint8_t calc_checksum(void *data, uint16_t size) {
    // add an offset, so the checksum is not correct when EEPROM is all zeros
    uint8_t checksum = 42;
    while (size > 0) {
        size--;
        checksum += *(((uint8_t *) data) + size);
    }
    return checksum;
}

void save_config(struct config conf) {
    // use update to save time if the values are same as the old ones
    // (read is >10x faster than write)
    StoredConfig new_store = {
        .cfg = conf,
        .checksum = calc_checksum(&conf, sizeof(conf)),
    };
    eeprom_update_block(&new_store, &stored_config, sizeof(new_store));
}

struct config load_config() {
    StoredConfig loaded_data;
    eeprom_read_block(&loaded_data, &stored_config, sizeof(loaded_data));
    if (calc_checksum(&(loaded_data.cfg), sizeof(loaded_data.cfg)) 
            != loaded_data.checksum){
        save_config(default_config);
        return default_config;
    } 
    return loaded_data.cfg;
}