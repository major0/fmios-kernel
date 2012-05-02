#include <io.h>

/*
 * term_ibm.c 
 *	IBM Console interface routines for kernel debugger
 *
 * This code contributed by G.T.Nicol.
 */

#define CONSOLE_NONE	0
#define CONSOLE_TERM	1
#define CONSOLE_SERIAL	2

#define COLOR			/* Else MGA text locations */
#define TERM_MAX_COL  80		/* Maximum number of columns */
#define TERM_MAX_ROW  25		/* Maximum number of rows */
#define TERM_RAM_SIZE (80*25)	/* Size of screen, in short's */

/*
 * Pick base of video RAM and control ports depending on which
 * type of interface we're using.
 */
#ifdef COLOR
#define GDC_REG_PORT 0x3d4
#define GDC_VAL_PORT 0x3d5
#define TVRAM 0xb8000    
#else
#define GDC_REG_PORT 0x3b4
#define GDC_VAL_PORT 0x3b5
#define TVRAM 0xb0000    
#endif

/*
 * White character attribute--works for MGA and CGA
 */
#define WHITE 0x07

/*
 * 1 for COM1, 0 for COM2 (bleh)
 */
#define COM (1)

/*
 * I/O address of registers
 */
#define IOBASE (0x2F0 + COM*0x100)	/* Base of registers */
#define LINEREG (IOBASE+0xB)	/* Format of RS-232 data */
#define LOWBAUD (IOBASE+0x8)	/* low/high parts of baud rate */
#define HIBAUD (IOBASE+0x9)
#define LINESTAT (IOBASE+0xD)	/* Status of line */
#define DATA (IOBASE+0x8)	/* Read/write data here */
#define INTREG (IOBASE+0x9)	/* Interrupt control */
#define INTID (IOBASE+0xA)	/* Why "interrupted" */
#define MODEM (IOBASE+0xC)	/* Modem lines */

static unsigned short *term_tvram = (unsigned short *) TVRAM;
static unsigned char term_current_col = 0;
static unsigned char term_current_row = 24; /* start debugger at bottom */
static unsigned int console = CONSOLE_TERM;

/*
 * term_set_cursor_pos()
 *	Set cursor position based on current absolute screen offset
 */
static void
term_set_cursor_pos(void)
{
	unsigned short gdc_pos;

	gdc_pos = (term_current_row * TERM_MAX_COL) + term_current_col;
	outb(GDC_REG_PORT, 0xe);
	outb(GDC_VAL_PORT, (gdc_pos >> 8) & 0xFF);
	outb(GDC_REG_PORT, 0xf);
	outb(GDC_VAL_PORT, gdc_pos & 0xff);
}

/*
 * term_scroll_up()
 *	Scroll screen up one line
 */
static void
term_scroll_up(void)
{
	unsigned int loop;

	for (loop = 0; loop < (TERM_MAX_ROW - 1) * TERM_MAX_COL; loop++) {
		term_tvram[loop] = term_tvram[loop + TERM_MAX_COL];
	}
	for (loop = (TERM_RAM_SIZE)-TERM_MAX_COL; loop < TERM_RAM_SIZE; loop++) {
		term_tvram[loop] = (WHITE<<8)|' ';
	}
}

/*
 * PUT()
 *	Write character at current screen location
 */
#define PUT(c) (term_tvram[(term_current_row * TERM_MAX_COL) + \
	term_current_col] = (WHITE << 8) | (c))

/*
 * cons_putc()
 *	Place a character on next screen position
 */
static void
term_cons_putc(int c)
{
	switch (c) {
	case '\t':
		do {
			term_cons_putc(' ');
		} while (term_current_col % 8);
		break;
	case '\r':
		term_current_col = 0;
		break;
	case '\n':
		term_current_row += 1;
		if (term_current_row >= TERM_MAX_ROW) {
			term_scroll_up();
			term_current_row -= 1;
		}
		break;
	case '\b':
		if (term_current_col > 0) {
			term_current_col -= 1;
			PUT(' ');
		}
		break;
	default:
		PUT(c);
		term_current_col += 1;
		if (term_current_col >= TERM_MAX_COL) {
			term_current_col = 0;
			term_current_row += 1;
			if (term_current_row >= TERM_MAX_ROW) {
				term_scroll_up();
				term_current_row -= 1;
			}
		}
	};
	term_set_cursor_pos();
}

static void term_cons_init()
{
	char *str = "Initializing console\n\r\0";

	while (*str) {
		term_cons_putc(*str++);
	}
}


/*
 * init_cons()
 *	Initialize to 9600 baud on com port
 */
static void
serial_cons_init(void)
{
	char *str = "Initializing console\n\r\0";

	outb(LINEREG, 0x80);	/* 9600 baud */
	outb(HIBAUD, 0);
	outb(LOWBAUD, 0x0C);
	outb(LINEREG, 3);		/* 8 bits, one stop */

	while (*str) {
		term_cons_putc(*str++);
	}
}

/*
 * rs232_putc()
 *	Busy-wait and then send a character
 */
static void
serial_cons_putc(int c)
{
	while ((inb(LINESTAT) & 0x20) == 0)
		;
	outb(DATA, c & 0x7F);
}


/*
 * init_cons()
 *	Hook for any early setup
 */
void
init_cons(void)
{
	switch (console) {
		case CONSOLE_TERM:
			term_cons_init();
			break;
		case CONSOLE_SERIAL:
			serial_cons_init();
			break;
		default:
			break;
	}
}


void
cons_putc(int c)
{
	switch (console) {
		case CONSOLE_TERM:
			term_cons_putc(c);
			break;
		case CONSOLE_SERIAL:
			serial_cons_putc(c);
			break;
		default:
			break;
	}
}


/*
 * putchar()
 *	Write a character to the debugger port
 *
 * Brain damage as my serial terminal doesn't wrap columns.
 */
static unsigned int col;
void
putchar(int c)
{
	if (c == '\n') {
		col = 0;
		cons_putc('\r');
		cons_putc('\n');
	} else {
		if (++col >= 78) {
			cons_putc('\r'); cons_putc('\n');
			col = 1;
		}
		cons_putc(c);
	}
}

