#include <tty.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xb8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_ptr;

/* Returns 2-byte vga character value using current terminal color */
static uint16_t vga_char(uint8_t character) {
    return (uint16_t) character | (terminal_color << 8);
}

void init_terminal(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color =  7; // light grey
    terminal_ptr = (uint16_t *) VGA_MEMORY;

    /* Clear the screen */
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_ptr[index] = vga_char(' ');
        }
    }
}

static void terminal_put_at(uint8_t c, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_ptr[index] = vga_char(c);
}

// TODO: come back here and make the downscrolling better.
// TODO: more elegantly handle newlines
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_row++;
        terminal_column = 0;
        return;
    }
    terminal_put_at(c, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = 0;
        }
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_print(const char* data) {
    terminal_write(data, strlen(data));
}



