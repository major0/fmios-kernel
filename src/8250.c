#include <asm/io.h>

/* 8250 registers */
#define DIVISOR_LOW_REG		0x00	/* When DLAB set */
#define DIVISOR_HI_REG		0x01	/* When DLAB set */
#define TX_HOLD_REG		0x00	/* outb() */
#define	RX_BUFF_REG		0x00	/* inb() */
#define INTR_ENABLE_REG		0x01
#define INTR_STATUS_REG		0x02
#define LINE_CTRL_REG		0x03
#define MODEM_CTRL_REG		0x04
#define LINE_STATUS_REG		0x05
#define MODEM_STATUS_REG	0x06

/* Interrupt Control Register bits */
#define INTR_CTRL_MODEM_STATUS	(1<<3)
#define INTR_CTRL_LINE_STATUS	(1<<2)
#define INTR_CTRL_TX_READY	(1<<1)
#define INTR_CTRL_RX_DATA	(1<<0)

/* Line Control Register bits */
#define LINE_CTRL_DLAB		(1<<7)
#define LINE_CTRL_BREAK		(1<<6)
#define LINE_CTRL_FORCE_PARITY	(1<<5)
#define LINE_CTRL_EVEN		(1<<4)
#define LINE_CTRL_PARITY	(1<<3)
#define LINE_CTRL_STOP2		(1<<2)	/* Clear for 1 stop bit */
#define LINE_CTRL_8BIT		0x3
#define LINE_CTRL_7BIT		0x2
#define LINE_CTRL_6BIT		0x1
#define LINE_CTRL_5BIT		0x0

/* 8250 Modem Control bits */
#define MODEM_CTRL_LOOP		(1<<4)
#define MDOEM_CTRL_INTR1	(1<<3)
#define MODEM_CTRL_INTR2	(1<<2)	/* Not Connected? */
#define MODEM_CTRL_RTS		(1<<1)
#define MODEM_CTRL_DTR		(1<<0)

/* 8250 Line Status bits */
#define LINE_STATUS_TSR		(1<<6)
#define LINE_STATUS_TX_READY	(1<<5)
#define LINE_STATUS_BREAK	(1<<4)
#define LINE_STATUS_FRAME_ERR	(1<<3)
#define LINE_STATUS_PARITY_ERR	(1<<2)
#define LINE_STATUS_OVERRUN	(1<<1)
#define LINE_STATIS_RX_DATA	(1<<0)

/* 8250 Modem Status bits */
#define MODEM_STATUS_RLSD	(1<<7)	/* RX Line Signal Detect */
#define MODEM_STATUS_RI		(1<<6)	/* Ring Indicator */
#define MODEM_STATUS_DSR	(1<<5)
#define MODEM_STATUS_CTS	(1<<4)
#define MODEM_STATUS_CLDS	(1<<3)	/* Change Line Signal Detect */
#define MODEM_STATUS_TRI	(1<<2)	/* Trailing Edge RI */
#define MODEM_STATUS_CDSR	(1<<1)	/* Change in DSR */
#define MODEM_STATUS_CCTS	(1<<0)	/* Change in CTS */

#define DEFAULT_IOBASE		0x3f8	/* COM1 */
#define DEFAULT_CLOCK		1843200	/* 1.8Mhz */
#define	DEFAULT_BAUD		9600
static uint32_t serial_iobase = DEFAULT_IOBASE;
static uint64_t serial_clock = DEFAULT_CLOCK;
static uint32_t serial_baud = DEFAULT_BAUD;
static uint16_t serial_div = DEFAULT_CLOCK / DEFAULT_BAUD / 16;
static uint8_t  serial_flags = LINE_CTRL_8BIT; /* 8n1 */

static void __serial_init(void) {
	/* Compute the divisor */
	serial_div = serial_clock / serial_baud / 16;

	/* set the divisor */
	outb(serial_iobase + LINE_CTRL_REG, LINE_CTRL_DLAB);
	outb(serial_iobase + DIVISOR_LOW_REG, serial_div & 0xff);
	outb(serial_iobase + DIVISOR_HI_REG, (serial_div >> 8) & 0xff);
	outb(serial_iobase + LINE_CTRL_REG, 0x0); /* clear DLAB */

	/* Set bits, parity, stop */
	outb((serial_iobase + LINE_CTRL_REG), serial_flags);

	/* Clear Interrupts */
	outb(serial_iobase + INTR_ENABLE_REG, 0x0);
	outb(serial_iobase + MODEM_CTRL_REG, 0x0);

	/* Print using printk() so the information can show up on alternate
	 * output devices */
	printk("8250_init: iobase=0x%x, baud=%d, flags=0x%x, div=0x%x\n",
			serial_iobase, serial_baud,
			serial_flags, serial_div);
}

/**
 * Write the character to the serial port
 */
int serial_putc(int c)
{
	uint32_t count = 0;
	uint8_t status = 0;

	if (!serial_iobase) {
		return 0;
	}

	/* Wait for TX_READY bit to be set */
	/* FIXME we should have a sleep/delay in here */
	while (count++ < 1000) {
		status = inb(serial_iobase + LINE_STATUS_REG);

		if (status & LINE_STATUS_TX_READY) {
			break;
		}
	}

	if (!(status & LINE_STATUS_TX_READY)) {
		/* ETIMEOUT? */
		return -1;
	}

	outb(serial_iobase + TX_HOLD_REG, c & 0xFF);
	return 1;
}

void serial_init(uint32_t iobase, uint32_t baud, uint8_t flags,
		uint16_t divisor)
{
	if (iobase) {
		serial_iobase = iobase;
	}

	if (flags) {
		serial_flags = flags;
	}

	/* Compute everything every way we can */
	if (divisor && baud) {
		serial_clock = divisor * baud * 16;
	} else if (baud) {
		serial_div = serial_clock / baud / 16;
	} else if (divisor) {
		serial_baud = serial_clock / divisor / 16;
	}

	__serial_init();
}
