#pragma once

#include <stdint.h>

void clear_bond();
void clear_bond_secrets();
void set_peripheral_addr(uint8_t *data, int idx);
void clear_peripheral(int idx);
void get_peripheral_addr(uint8_t *data, int idx);
