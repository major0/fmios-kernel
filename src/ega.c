#include <asm/io.h>

#define VIDEO_ADDR	0xb8000
static uint16_t *video_addr = (uint16_t *)VIDEO_ADDR;

#define VIDEO_COLS	80
#define VIDEO_ROWS	25
static uint8_t video_cols = VIDEO_COLS;
static uint8_t video_rows = VIDEO_ROWS;
static uint8_t cur_col = 0;
static uint8_t cur_row = VIDEO_ROWS - 1;

/**
 * Set the VGA Textmode Cursor Position
 */
#define VIDEO_CMD_REG	0x3d4
#define VIDEO_VAL_REG	0x3d5
#define CURSOR_LOC_HI	0x0e
#define CURSOR_LOC_LOW	0x0f
static void update_cursor()
{
	uint16_t pos;

	pos = (cur_row * video_cols) + cur_col;

	outb(VIDEO_CMD_REG, CURSOR_LOC_HI);
	outb(VIDEO_VAL_REG, (pos >> 8) & 0xFF);

	outb(VIDEO_CMD_REG, CURSOR_LOC_LOW);
	outb(VIDEO_VAL_REG, pos & 0xff);
}

/**
 * Scroll screen up one line
 */
static void video_scroll(void)
{
	uint16_t pos;

	/* First copy over row N with the contents of row N + 1 */
	for (pos = 0; pos < (video_rows - 1) * video_cols; pos++) {
		video_addr[pos] = video_addr[pos + video_cols];
	}

	/* Fill in the last ling with spaces */
	pos = (video_rows * video_cols) - video_cols;
	for (; pos < (video_rows * video_cols); pos++) {
		video_addr[pos] = (0x07<<8)|' ';
	}

	cur_row -= 1;
}

/**
 * Write character at current screen location
 */
static void __ega_putc(int c) {
	video_addr[(cur_row * video_cols) + cur_col] = (0x07 << 8) | (c);
}

/**
 * Place a character on next screen position
 */
int ega_putc(int c)
{
	if (!video_addr) {
		return 0;
	}

	switch (c) {
	case '\t':
		do {
			ega_putc(' ');
		} while (cur_col % 8);
		break;
	case '\r':
		cur_col = 0;
		break;
	case '\n': /* Assume \n\r */
		cur_row += 1;
		cur_col = 0;
		if (cur_row >= video_rows) {
			video_scroll();
		}
		break;
	case '\b':
		if (cur_col > 0) {
			cur_col -= 1;
			__ega_putc(' ');
		}
		break;
	default:
		__ega_putc(c);
		cur_col += 1;
		if (cur_col >= video_cols) {
			cur_col = 0;
			cur_row += 1;
			if (cur_row >= video_rows) {
				video_scroll();
			}
		}
	};
	update_cursor();

	return 1;
}

void ega_init(uint32_t addr, uint8_t cols, uint8_t rows)
{
	if (addr) {
		video_addr = (uint16_t *)addr;
	}

	if (cols && rows) {
		video_cols = cols;
		video_rows = rows;

		cur_col = 0;
		cur_row = rows - 1;
	}

	/* Use printk to display the data so that the message shows up on all
	 * outputs */
	printk("ega_init: addr=0x%x, cols=%d, rows=%d\n",
			video_addr, video_cols, video_rows);
}
