/* serial.h - Serial definitions */
/* Copyright (C) 2012 Mark Ferrell
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ANY
 * DEVELOPER OR DISTRIBUTOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _FMIOS_SERIAL_H
#define _FMIOS_SERIAL_H

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

#ifndef __ASSEMBLY__

int serial_putc(int c);
void serial_init(uint32_t iobase, uint32_t baud, uint8_t flags, uint16_t divisor);

#endif

#endif /* _FMIOS_SERIAL_H */
