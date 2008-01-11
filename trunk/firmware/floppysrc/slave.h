#ifndef _SLAVE_H
#define _SLAVE_H

#include "integer.h"

#define SR_OPENERR	1
#define	SR_READERR	2
#define SR_FORMAT	3

#define CPU_REQUEST_READ 	0x10 	// Request to read one sector at oTRACK, oSECTOR, bit 0 is side (head #) 
#define CPU_REQUEST_WRITE	0x20 	// Request to write one sector at oTRACK, oSECTOR; bit 0 is side (head #) 
#define CPU_REQUEST_READADDR 0x30	// Request to read sector address (return 6 byte header) 
#define CPU_REQUEST_ACK		0x80	// clear status
#define CPU_REQUEST_FAIL	0xC0	// failure feedback for debug

#define CPU_STATUS_COMPLETE	0x01
#define CPU_STATUS_ERROR	0x02


uint8_t slave(const char *imagefile, uint8_t *buffer);

#endif