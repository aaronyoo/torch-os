#ifndef __PIC_H__
#define __PIC_H__

void init_pic();
void pic_send_eoi(unsigned char irq);

#endif