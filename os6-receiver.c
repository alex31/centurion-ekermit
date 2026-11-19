/* Receive one Kermit binary executable into an existing CENTOS type E file.
 * SYS000 must be CRT1 and SYS001 must be a temporary, preallocated E file. */
#include <stdio.h>
#include <os6.h>
#include <os6exf.h>
#include "cdefs.h"
#include "debug.h"
#include "kermit.h"
#include "os6-transport.h"

static struct k_data state;
static struct k_response response;
static UCHAR output_buffer[256];
static OS6_EXF_BLOCK file_block;
static unsigned int filled, sectors;
static int opened, closed, disk_error;

static int transmit(struct k_data *k, UCHAR *data, int length)
{
    return kr_write_packet(data, length);
}

static int open_file(struct k_data *k, UCHAR *name, int mode)
{
    if (mode != 2 || !name || !name[0] || opened || !k->ebqflg)
        return X_ERROR;
    if (os6_exf_begin(1)) return X_ERROR;
    file_block.length = OS6_EXF_SECTOR_SIZE;
    filled = sectors = 0;
    opened = 1;
    return X_OK;
}

static int write_file(struct k_data *k, UCHAR *bytes, int length)
{
    int i;
    for (i = 0; i < length; i++) {
        file_block.data[filled++] = bytes[i];
        if (filled == OS6_EXF_SECTOR_SIZE) {
            disk_error = os6_exf_write(&file_block);
            if (disk_error) return X_ERROR;
            filled = 0;
            sectors++;
        }
        if ((i & 63) == 0) os6_checkpoint();
    }
    return X_OK;
}

static int close_file(struct k_data *k, UCHAR disposition, int mode)
{
    if (mode != 2 || disposition == 'D' || !opened || closed || disk_error ||
        filled != 0 || sectors == 0) return X_ERROR;
    closed = 1;
    return X_OK;
}

int main(void)
{
    short slot;
    int length, status, result = 1;
    UCHAR *packet;
    if (kr_serial_open()) { puts("CRT1 OPEN FAILED"); return 1; }
    state.txd = transmit;
    state.openf = open_file;
    state.writef = write_file;
    state.closef = close_file;
    state.obuf = output_buffer;
    state.obuflen = sizeof(output_buffer);
    state.binary = BINARY;
    state.xfermode = 1;
    state.parity = PAR_EVEN;
    state.bct = 3;
    status = kermit(K_INIT, &state, 0, 0, "", &response);
    if (status != X_OK) goto done;
    puts("READY KERMIT CRT1");
    while (status != X_DONE) {
        packet = getrslot(&state, &slot);
        if (!packet || slot < 0) goto done;
        length = kr_read_packet(packet, P_PKTLEN + 8);
        if (length <= 0) goto done;
        status = kermit(K_RUN, &state, slot, length, "", &response);
        if (status == X_ERROR) goto done;
    }
    if (opened && closed && !disk_error) result = 0;
    if (!result) printf("RECEIVED %u SECTORS\n", sectors);
done:
    if (kr_serial_close()) result = 1;
    if (result) puts("TRANSFER FAILED; TEMPORARY FILE IS INCOMPLETE");
    return result;
}
