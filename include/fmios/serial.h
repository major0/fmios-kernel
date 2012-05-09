#ifndef _SERIAL_H
#define _SERIAL_H

/* These are standard in 8250, might was well make them universal */
#define SERIAL_PARITY_EVEN	(1<<4 | 1<<3)
#define SERIAL_PARITY_ODD	(1<<3)
#define SERIAL_PARITY_NONE	(0x0)
#define SERIAL_STOP2		(1<<2)
#define SERIAL_STOP1		(0x0)
#define SERIAL_8BIT		0x3
#define SERIAL_7BIT		0x2
#define SERIAL_6BIT		0x1
#define SERIAL_5BIT		0x0

#endif /* _SERIAL_H */
