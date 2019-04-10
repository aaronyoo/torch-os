#include <keyboard.h>
#include <isr.h>
#include <stdint.h>
#include <io.h>
#include <logger.h>
#include <tty.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

// https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html

#define KEYMAP_SIZE 256
#define SHIFT 1
#define NORMAL 0

char keymap[2][KEYMAP_SIZE];
int shift = 0;   // > 1 if the shift key is pressed

void process_key_press(uint8_t scan_code) {
    /* Figure out which map we need to use based
       on if the SHIFT keys are being pressed */
    unsigned int modifier;
    if (shift > 0) {
        modifier = SHIFT;
    } else {
        modifier = NORMAL;
    }

    if (scan_code == 0x2a || scan_code == 0x36) {
        /* A SHIFT is being pressed */
        shift++;
        return;
    }

    /* Convert the scan code into a character */
    if (keymap[modifier][scan_code] == 0) {
        logf("[PANIC] Unhandled keypress: %x", scan_code);
        while(1); // TODO: change to panic
    }

    /* It's safe to convert to a character */
    char c = keymap[modifier][scan_code];

    // TODO: add to the character buffer
    // But for now, let's just print to the screen
    terminal_putchar(c);
}

void process_key_release(uint8_t scan_code) {
    /* Convert into pressed scancode by unsetting the
       higher order bit */
    scan_code -= 0x80;

    if (scan_code == 0x2a || scan_code == 0x36) {
        /* A SHIFT is being released */
        shift--;
        if (shift < 0) {
            logf("[PANIC] More shift keys were released than pressed");
            while(1); // TODO: change to panic
        }
    }
}

void keyboard_handler(context_t* context) {
    /* Read in the keypress. Note that this reading step is required
       before the PIC will generate more keyboard interrupts */
    uint8_t scan_code = inb(0x60);
    
    if (scan_code & 0x80) {
        /* A key was just released */
        process_key_release(scan_code);
    } else {
        /* A key was just pressed */
        process_key_press(scan_code);
    }
}

void init_keyboard() {
    /* Set up the standard keyboard map */
    memset(keymap, 0, sizeof(char) * KEYMAP_SIZE);
    keymap[0][0x1E] = 'a';
    keymap[0][0x30] = 'b';
    keymap[0][0x2E] = 'c';
    keymap[0][0x20] = 'd';
    keymap[0][0x12] = 'e';
    keymap[0][0x21] = 'f';
    keymap[0][0x22] = 'g';
    keymap[0][0x23] = 'h';
    keymap[0][0x17] = 'i';
    keymap[0][0x24] = 'j';
    keymap[0][0x25] = 'k';
    keymap[0][0x26] = 'l';
    keymap[0][0x32] = 'm';
    keymap[0][0x31] = 'n';
    keymap[0][0x18] = 'o';
    keymap[0][0x19] = 'p';
    keymap[0][0x10] = 'q';
    keymap[0][0x13] = 'r';
    keymap[0][0x1f] = 's';
    keymap[0][0x14] = 't';
    keymap[0][0x16] = 'u';
    keymap[0][0x2f] = 'v';
    keymap[0][0x11] = 'w';
    keymap[0][0x2d] = 'x';
    keymap[0][0x15] = 'y';
    keymap[0][0x2c] = 'z';
    keymap[0][0x1c] = '\n';
    keymap[0][0x39] = ' ';
    keymap[0][0x34] = '.';
    keymap[0][0x27] = ';';

    /* Shift key is pressed */
    keymap[1][0x1E] = 'A';
    keymap[1][0x30] = 'B';
    keymap[1][0x2E] = 'C';
    keymap[1][0x20] = 'D';
    keymap[1][0x12] = 'E';
    keymap[1][0x21] = 'F';
    keymap[1][0x22] = 'G';
    keymap[1][0x23] = 'H';
    keymap[1][0x17] = 'I';
    keymap[1][0x24] = 'J';
    keymap[1][0x25] = 'K';
    keymap[1][0x26] = 'L';
    keymap[1][0x32] = 'M';
    keymap[1][0x31] = 'N';
    keymap[1][0x18] = 'O';
    keymap[1][0x19] = 'P';
    keymap[1][0x10] = 'Q';
    keymap[1][0x13] = 'R';
    keymap[1][0x1f] = 'S';
    keymap[1][0x14] = 'T';
    keymap[1][0116] = 'U';
    keymap[1][0x2f] = 'V';
    keymap[1][0x11] = 'W';
    keymap[1][0x2d] = 'X';
    keymap[1][0x15] = 'Y';
    keymap[1][0x2c] = 'Z';
    keymap[1][0x1c] = '\n';
    keymap[1][0x39] = ' ';
    keymap[1][0x34] = '>';
    keymap[1][0x27] = ':';

    /* Register the keyboard handler */
    register_interrupt_handler(33, &keyboard_handler);
}