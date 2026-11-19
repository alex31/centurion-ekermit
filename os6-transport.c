#include "cdefs.h"
#include "debug.h"
#include "kermit.h"
#include "os6-transport.h"

unsigned char kr_reply_buffer[KR_REPLY_CAPACITY];

static int hex_digit(int c)
{
    c &= 127;
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* Check framing before E-Kermit uses the packet's length fields as offsets. */
static int valid_packet_length(const unsigned char *packet, int count)
{
    int length = packet[1] - 32;
    if (length < 0 || length > 94) return 0;
    if (length) return length >= 3 && count == length + 3;
#ifdef F_LP
    if (count < 11 || packet[4] < 32 || packet[4] > 126 ||
        packet[5] < 32 || packet[5] > 126) return 0;
    length = (packet[4] - 32) * 95 + packet[5] - 32;
    /* The caller checks the complete packet against the actual slot size. */
    return length >= 1 && count == length + 8;
#else
    return 0;
#endif
}

/* Carry Kermit packets in CRT lines, escaping bytes that the line editor
 * filters or that would be confused with the transport acknowledgement. */
int kr_read_packet(unsigned char *buffer, int capacity)
{
    static UCHAR packet[P_PKTLEN + 10];
    char *line;
    int i, hi, lo, length, position, value, last, count = 0;
    while (1) {
        line = kr_serial_line();
        last = (line[0] & 127) == 'L';
        if (!last && (line[0] & 127) != 'C') continue;
        hi = hex_digit(line[1]);
        lo = hex_digit(line[2]);
        if (hi < 0 || lo < 0) return -1;
        length = hi * 16 + lo;
        if (length < 1 || length > 71 || count + length > sizeof(packet)) return -1;
        position = 3;
        for (i = 0; i < length; i++) {
            if (position >= 75) return -1;
            value = line[position++] & 127;
            if (value == '`') {
                if (position >= 75) return -1;
                value = line[position++] & 127;
                if (value == 'A') value = SOH;
                else if (value == 'M') value = CR;
                else if (value == 'P') value = '+';
                else if (value == 'U') value = '_';
                else if (value == '`') value = '`';
                else return -1;
            } else if (value < 32 || value > 126 || value == '_' || value == '+') {
                return -1;
            }
            packet[count++] = value;
        }
        if (last) break;
        kr_serial_put('+');
    }
    if (count < 6 || packet[0] != SOH || packet[count - 1] != CR ||
        count - 2 > capacity || !valid_packet_length(packet, count))
        return -1;
    for (i = 1; i < count - 1; i++) buffer[i - 1] = packet[i];
    return count - 2;
}

int kr_write_packet(const unsigned char *buffer, int length)
{
    int i;
    if (length < 1 || length > KR_REPLY_CAPACITY) return X_ERROR;
    for (i = 0; i < length; i++) kr_reply_buffer[i] = buffer[i] | 0x80;
    kr_serial_send(length);
    return X_OK;
}
