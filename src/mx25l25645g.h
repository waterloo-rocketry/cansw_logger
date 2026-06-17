#ifndef MX25L25645G_H
#define MX25L25645G_H

#include <stdint.h>

void mx25l25645g_init(void);
void mx25l25645g_erase(uint32_t address);
void mx25l25645g_erase_chip(void);
void mx25l25645g_write(uint32_t address, uint32_t npages, const uint8_t *buffer);
void mx25l25645g_read(uint32_t address, uint32_t npages, uint8_t *buffer);

#endif
