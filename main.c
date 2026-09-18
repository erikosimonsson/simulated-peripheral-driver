#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    OFF,
    STARTING,
    READY
} Power;

typedef struct {
    uint8_t register_bank[4];
    Power power;
    int power_tick;
} Chip;

//General functions
bool get_mask(char *mode, unsigned int *or_mask);
unsigned int get_mode(unsigned int state);
bool read_register(const Chip *chip, size_t requested_address, uint8_t *result);
bool write_register(Chip *chip, size_t requested_address, uint8_t byte_val);

//Chip behaviours
void chip_enable(Chip *chip);
void chip_disable(Chip *chip);
void chip_increment_power(Chip *chip);
bool chip_wait_ready(Chip *chip, int time_budget);

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Error: Invalid amount of launch arguments. Usage: .\\chip_sim.exe [MODE_INT]\nMODE_INT = 0: Standby mode\nMODE_INT = 1: Single mode\nMODE_INT = 2: Continuous mode");
        return 1;
    }

    uint8_t register_value;
    unsigned int and_mask = 0xF3;
    unsigned int or_mask;
    bool valid_mask = get_mask(argv[argc-1], &or_mask);
    bool valid_reg_read;
    bool valid_reg_write;

    if (!valid_mask) {
        printf("Error: Invalid mode argument. Usage: .\\chip_sim.exe [MODE_INT]\nMODE_INT = 0: Standby mode\nMODE_INT = 1: Single mode\nMODE_INT = 2: Continuous mode");
        return 1;
    }

    Chip chip;
    chip.power = OFF;
    chip_enable(&chip);
    bool active = chip_wait_ready(&chip, 50);

    if (active) {
        printf("Chip is on");
        return 0;
    }
    else {
        printf("Chip not ready within time budget");
        return 1;
    }
}

bool get_mask(char *mode, unsigned int *or_mask) {
    if (!strcmp(mode, "0")) {
        *or_mask = 0x0;
    }
    else {
        if (!strcmp(mode, "1")) {
            *or_mask = 0x1 << 2;
        }
        else if (!strcmp(mode, "2")) {
            *or_mask = 0x1 << 3;
        }
        else {
            return false;
        }
    }
    return true;
}

unsigned int get_mode(unsigned int state) {
    return (state & 0x0C) >> 2;
}

bool read_register(const Chip *chip, size_t requested_address, uint8_t *result) {
    if (chip == NULL || result == NULL) {
        printf("Error: Pointer missing object destination\n");
        return false;
    }

    if (chip->power != READY) {
        printf("Error: Can not read from non-ready chip\n");
        return false;
    }

    if (4 <= requested_address) {
        printf("Error: Requested registry entry out of bounds\n");
        return false;
    }
    *result = chip->register_bank[requested_address];
    return true;
}

bool write_register(Chip *chip, size_t requested_address, uint8_t byte_val) {
    if (chip == NULL) {
        printf("Error: Can not write byte to NULL register\n");
        return false;
    }

    if (chip->power != READY) {
        printf("Error: Can not write to non-ready chip\n");
        return false;
    }
    
    if (4 <= requested_address) {
        printf("Error: Adress out of bound on registry write");
        return false;
    }
    
    if (requested_address != 1) {
        printf("Error: Driver can not write to register address %zu\n", requested_address);
        return false;
    }

    unsigned int requested_mode = get_mode(byte_val);
    if (requested_mode == 3) {
        printf("Error: Can not write mode 3 to registry\n");
        return false;
    }

    chip->register_bank[requested_address] = byte_val;
    return true;
}

void chip_enable(Chip *chip) {
    if (chip->power == OFF) {
        chip->power = STARTING;
        chip->power_tick = 0;
        chip->register_bank[0] = 0x42;
        chip->register_bank[1] = 0x00;
        chip->register_bank[2] = 0x00;
        chip->register_bank[3] = 0x00;
    }
}

void chip_disable(Chip *chip) {
    chip->power = OFF;
    chip->power_tick = 0;
}

void chip_increment_power(Chip *chip) {
    if (chip->power == STARTING && chip->power_tick < 3) {
        chip->power_tick += 1;
        if (chip->power_tick == 3) {
            chip->power = READY;
        }
    }
}

bool chip_wait_ready(Chip *chip, int time_budget) {
    if (chip == NULL || chip->power == OFF) {
        return false;
    }
    if (chip->power == READY) {
        return true;
    }
    for (int i = 0; i < time_budget; i++) {
        chip_increment_power(chip);
        if (chip->power == READY) {
            return true;
        }
    }
    return false;
}
