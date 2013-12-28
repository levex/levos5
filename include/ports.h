#ifndef PORTS_H
#define PORTS_H

#include <stdint.h>

/*static uint16_t inw(uint16_t _port);
static uint8_t inb(uint16_t _port);
static uint32_t inl(uint16_t _port);
static void outw(uint16_t _port, uint16_t _data);
static void outb(uint16_t _port, uint8_t _data);
static void outl(uint16_t _port, uint32_t _data);*/

static inline uint16_t __attribute__((always_inline)) inw(uint16_t _port) {
	uint16_t result;
	__asm__ volatile ("inw %1, %0" : "=a" (result) : "Nd" (_port));
	return result;
}

/// in in byte ///
static inline uint8_t __attribute__((always_inline)) inb(uint16_t _port) {
	uint8_t result;
	__asm__ volatile ("inb %1, %0" : "=a" (result) : "Nd" (_port));
	return result;
}

/// in in long(32 bit) ///
static inline uint32_t __attribute__((always_inline)) inl(uint16_t _port) {
	uint32_t result;
	__asm__ volatile ("inl %1, %0" : "=a" (result) : "Nd" (_port));
	return result;
}


/// out in dword ///
static inline void __attribute__((always_inline)) outw(uint16_t _port, uint16_t _data) {
	__asm__ volatile ("outw %0, %1" : : "a" (_data), "Nd" (_port));
}

/// out in byte ///
static inline void __attribute__((always_inline)) outb(uint16_t _port, uint8_t _data) {
	__asm__ volatile ("outb %0, %1" : : "a" (_data), "Nd" (_port));
}

/// out in long(32 bit) ///
static inline void __attribute__((always_inline)) outl(uint16_t _port, uint32_t _data) {
	__asm__ volatile ("outl %0, %1" : : "a"(_data), "Nd" (_port));
}


#endif
