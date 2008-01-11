#include "slave.h"
#include "specialio.h"
#include "tff.h"
#include "fddimage.h"
#include "integer.h"
#include "timer.h"

#include "serial.h"

FDDImage fddimage;

#define DELAY_RELOAD 4096

// thrall forever
uint8_t slave(const char *imagefile, uint8_t *buffer) {
	FIL	file1;
	uint8_t leds = 0x01;
	uint16_t delay = 1;
	uint8_t last_request = 0377;

	uint8_t result;

	if (f_open(&file1, imagefile, FA_READ) != FR_OK) return SR_OPENERR;

	fdd_load(&file1, &fddimage, buffer);
	fdd_seek(&fddimage, 0, 4, 1);
	if (fdd_readsector(&fddimage) != FR_OK) return SR_READERR;

	if (buffer[0] != '\000' || buffer[1] == '\000') return SR_FORMAT;	// directory starts with a 0
	
	// tests passed, clear to slave forever
	
	for (;;) {
		if (MASTER_COMMAND != last_request) {
			last_request = MASTER_COMMAND;
			
			ser_nl();

			switch (MASTER_COMMAND & 0xf0) {
			case CPU_REQUEST_READ:
				ser_putc('R');

				SLAVE_STATUS = 0;
				fdd_seek(&fddimage, MASTER_COMMAND & 0x01, MASTER_TRACK, MASTER_SECTOR);

				print_hex(fddimage.cur_side);
				print_hex(fddimage.cur_sector);
				print_hex(fddimage.cur_track);

				if ((result = fdd_readsector(&fddimage)) != FR_OK) SLAVE_STATUS = CPU_STATUS_ERROR;

				ser_nl();
				print_hex(result);
				ser_nl();
				
				print_buff(fddimage.buffer);

				SLAVE_STATUS |= CPU_STATUS_COMPLETE;
				// touche!

				break;
			case CPU_REQUEST_READADDR:
				ser_putc('Q');
				// fill the beginning of buffer with position info
				// 6 bytes: track, side, sector, sectorsize code, crc1, crc2
				SLAVE_STATUS = 0;
				if (fdd_readadr(&fddimage) != FR_OK) SLAVE_STATUS |= CPU_STATUS_ERROR;
				SLAVE_STATUS |= CPU_STATUS_COMPLETE;
				// touche!
				break;
			case CPU_REQUEST_WRITE:
				ser_putc('W');
				SLAVE_STATUS = CPU_STATUS_COMPLETE | CPU_STATUS_ERROR;
				break;
			case CPU_REQUEST_ACK:
				ser_putc('A');
				SLAVE_STATUS = 0;
				break;
			case CPU_REQUEST_FAIL:
			default:
				ser_putc('<');
				print_hex(MASTER_COMMAND);
				ser_putc('>');
				break;
			}
		} else {
			if (--delay == 0) {
				delay = DELAY_RELOAD;
				GREEN_LEDS = leds;
				leds <<= 1;
				if (leds == 0) leds = 0x01;
			}
		}
	}
}
