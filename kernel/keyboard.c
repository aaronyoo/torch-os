#include <keyboard.h>
#include <isr.h>
#include <stdint.h>
#include <io.h>
#include <logger.h>
#include <tty.h>
#include <string.h>

// https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html

#define KEYMAP_SIZE 256

char keymap[KEYMAP_SIZE];

// void sc(uint8_t scan_code) {
//         /* Convert the scan code into a character */
//     if (keymap[scan_code] == 0) {
//         logf("[PANIC] Unhandled keypress: %x", scan_code);
//         while(1); // TODO: change to panic
//     }

//     /* It's safe to convert to a character */
//     char c = keymap[scan_code];

//     // TODO: add to the character buffer
//     // But for now, let's just print to the screen
//     terminal_putchar(c);
// }

void process_scan_code(uint8_t scan_code) {
    // /* Convert the scan code into a character */
    // if (keymap[scan_code] == 0) {
    //     logf("[PANIC] Unhandled keypress: %x", scan_code);
    //     while(1); // TODO: change to panic
    // }

    // /* It's safe to convert to a character */
    // char c = keymap[scan_code];

    // // TODO: add to the character buffer
    // // But for now, let's just print to the screen
    // terminal_putchar(c);
}

void keyboard_handler(context_t* context) {
    /* Read in the keypress. Note that this reading step is required
       before the PIC will generate more keyboard interrupts */
    // uint8_t scan_code = inb(0x60);
    uint8_t scan_code = 0;
    // process_scan_code(scan_code);
}

void init_keyboard() {
    memset(keymap, 0, sizeof(char) * KEYMAP_SIZE);
    keymap[0x1E] = 'A';
    keymap[0x30] = 'B';
    keymap[0x2E] = 'C';
    keymap[0x20] = 'D';
    keymap[0x12] = 'E';
    
    keymap[0x1C] = '\n';

    /* Register the keyboard handler */
    register_interrupt_handler(33, 0);
}

// void init_keyboard1() {
//     // memset(keymap, 0, sizeof(char) * KEYMAP_SIZE);
//     // keymap[0x1E] = 'A';
//     // keymap[0x30] = 'B';
//     // keymap[0x2E] = 'C';
//     keymap[0x20] = 'D';
//     // keymap[0x12] = 'E';
    
//     // keymap[0x1C] = '\n';

//     /* Register the keyboard handler */
//     // register_interrupt_handler(33, 0);
// }

// void init_keyboard2() {
//     memset(keymap, 0, sizeof(char) * KEYMAP_SIZE);
//     keymap[0x1E] = 'A';
//     keymap[0x30] = 'B';
//     keymap[0x2E] = 'C';
//     keymap[0x20] = 'D';
//     keymap[0x12] = 'E';
    
//     keymap[0x1C] = '\n';

//     /* Register the keyboard handler */
//     register_interrupt_handler(33, 0);
// }