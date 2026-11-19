#ifndef EKERMIT_OS6_TRANSPORT_H
#define EKERMIT_OS6_TRANSPORT_H

int kr_serial_open(void);
char *kr_serial_line(void);
void kr_serial_put(int c);
void kr_serial_send(int length);
int kr_serial_close(void);
int kr_read_packet(unsigned char *buffer, int capacity);
int kr_write_packet(const unsigned char *buffer, int length);

#define KR_REPLY_CAPACITY 96
extern unsigned char kr_reply_buffer[KR_REPLY_CAPACITY];

#endif
