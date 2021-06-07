#pragma once
#include <stdint.h>
#include <stddef.h>
#include "compiler.h"

typedef uint16_t ioport_t;

extern void volatile *port_io_window;

template<typename T>
static inline T read_port(ioport_t port)
{
    return *((T volatile *)((char volatile *)port_io_window + port));
}

template<typename T>
static inline void write_port(ioport_t port, T value)
{
    *((T volatile *)((char volatile *)port_io_window + port)) = value;
}

static _always_inline uint8_t inb(ioport_t port)
{
    return read_port<uint8_t>(port);
}

static _always_inline uint16_t inw(ioport_t port)
{
    if (!(port & -2)) {
        // aligned
        return read_port<uint16_t>(port);
    } else {
        // not aligned - do byte halves
        uint8_t lo = read_port<uint8_t>(port);
        uint8_t hi = read_port<uint8_t>(port + 1);
        return (hi << 8) | lo;
    }
}

static _always_inline uint32_t ind(ioport_t port)
{
    if (!(port & -4)) {
        // aligned
        return read_port<uint8_t>(port);
    } else if (!(port & -2)) {
        // not aligned - but can do 16 bit halves
        uint16_t lo = read_port<uint16_t>(port);
        uint16_t hi = read_port<uint16_t>(port + 2);
        return (hi << 16) | lo;
    } else {
        // wow, horribly misaligned
        uint32_t result;
        result = read_port<uint8_t>(port);
        result |= read_port<uint8_t>(port + 1);
        result |= read_port<uint8_t>(port + 2);
        result |= read_port<uint8_t>(port + 3);
        return result;
    }
}

static _always_inline void outb(ioport_t port, uint8_t value)
{
    write_port<uint8_t>(port, value);
}

static _always_inline void outw(ioport_t port, uint16_t value)
{
    if (!(port & -2)) {
        // aligned
        write_port<uint16_t>(port, value);
    } else {
        // not aligned - do byte halves
        write_port<uint8_t>(port, value & 0xFF);
        write_port<uint8_t>(port, value >> 8);
    }
}

static _always_inline void outd(ioport_t port, uint32_t value)
{
    if (!(port & -4)) {
        // aligned
        return write_port<uint8_t>(port, value);
    } else if (!(port & -2)) {
        // not aligned - but can do 16 bit halves
        write_port<uint16_t>(port, value & 0xFFFF);
        write_port<uint16_t>(port + 2, value >> 16);
    } else {
        // wow, horribly misaligned
        write_port<uint8_t>(port, value & 0xFF);
        write_port<uint8_t>(port + 1, (value >> 8) & 0xFF);
        write_port<uint8_t>(port + 2, (value >> 16) & 0xFF);
        write_port<uint8_t>(port + 3, (value >> 24) & 0xFF);
    }
}

//
// Block I/O

static _always_inline void insb(ioport_t port, void *values, intptr_t count)
{
    uint8_t *result = (uint8_t*)values;
    for (intptr_t i = 0; i < count; ++i)
        result[i] = inb(port);
}

static _always_inline void insw(ioport_t port, void *values, intptr_t count)
{
    uint16_t *result = (uint16_t*)values;
    for (intptr_t i = 0; i < count; ++i)
        result[i] = inw(port);
}

static _always_inline void insd(ioport_t port, void *values, intptr_t count)
{
    uint32_t *result = (uint32_t*)values;
    for (intptr_t i = 0; i < count; ++i)
        result[i] = ind(port);
}

static _always_inline void outsb(
        ioport_t port, void const *values, size_t count)
{
    uint8_t *src = (uint8_t*)values;
    for (size_t i = 0; i < count; ++i)
        outb(port, src[i]);
}

static _always_inline void outsw(
        ioport_t port, void const *values, size_t count)
{
    uint16_t *src = (uint16_t*)values;
    for (size_t i = 0; i < count; ++i)
        outw(port, src[i]);
}

static _always_inline void outsd(
        ioport_t port, void const *values, size_t count)
{
    uint32_t *src = (uint32_t*)values;
    for (size_t i = 0; i < count; ++i)
        outd(port, src[i]);
}
