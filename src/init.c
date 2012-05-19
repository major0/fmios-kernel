#include <fmios/fmios.h>
#include <fmios/malloc.h>
#include <fmios/serial.h>
#include <fmios/video.h>
#include <fmios/io.h>
#include <multiboot.h>
#include <string.h>

/* Start looking for free pages at the end of the kernel */
extern const void __kernel_start;
extern const void _end;
unsigned long kernel_start;
unsigned long kernel_end;

extern char * cmdline_get_opt(char *cmdline, char *option);
extern struct pmap_table * init_malloc(void);

static void init_video(char *cmdline, struct video_config *config)
{
	char *param;

	param = cmdline_get_opt(cmdline, "video");

	/* cmdline param will be in the format of:
	 * [addr|ega|cga|off][,<height>x<width>[,bpp]]]
	 */
	if (param) {
		if (strncmp("off", param, 3) == 0) {
			return;
		}

		if (strncmp("ega", param, 3) == 0
		 || strncmp("cga", param, 3) == 0) {
			config->type = MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT;
			config->addr = 0xb8000;
			param += 3;
		} else {
			config->addr = strtol(param, &param, 0);
		}

		if (*param == ',') {
			config->height = strtol(++param, &param, 0);
			config->width = strtol(++param, &param, 0);
			if (*param == ',') {
				config->depth = strtol(++param, &param, 0);
			}
		}
	}

	/* FIXME This should be filled in with other video types later */
	switch (config->type) {
	case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
		ega_init(config->addr, config->width, config->height);
		break;
	}
}

static void init_serial(char *cmdline)
{
	uint32_t iobase = 0;
	uint32_t baud = 0;
	uint16_t divisor = 0;
	uint8_t flags = 0;
	char *param;

	param = cmdline_get_opt(cmdline, "serial");

	/* cmdline param will be in the format of:
	 * [iobase][,baud][,flags][,divisor]
	 */
	if (!param) {
		return;
	}

	if (strncmp("off", param, 3) == 0) {
		return;
	}

	iobase = strtol(param, &param, 0);

	if (*param == ',') {
		baud = strtol(++param, &param, 0);
	}

	if (*param == ',') {
		++param;
		switch (*param) {
		case '8':
			flags = SERIAL_8BIT;
			param++;
			break;
		case '7':
			flags = SERIAL_7BIT;
			param++;
			break;
		case '6':
			flags = SERIAL_6BIT;
			param++;
			break;
		case '5':
			flags = SERIAL_5BIT;
			param++;
			break;
		}

		if (flags) {
			switch (*param) {
			case 'e':
			case 'E':
				flags |= SERIAL_PARITY_EVEN;
				param++;
				break;
			case 'o':
			case 'O':
				flags |= SERIAL_PARITY_ODD;
				param++;
				break;
			case 'n':
			case 'N':
				flags |= SERIAL_PARITY_NONE;
				param++;
				break;
			}

			switch (*param) {
			case '2':
				flags |= SERIAL_STOP2;
				param++;
				break;
			case '1':
				flags |= SERIAL_STOP1;
				param++;
				break;
			}
		}
	}

	if (*param == ',') {
		divisor = strtol(param, &param, 0);
	}

	serial_init(iobase, baud, flags, divisor);
}

/* These routines are platform specific and must be defined to boot */
static int __init_paging(struct pmap_table *pmap)
{
	struct pmap_entry *entry = pmap->entry;
	int index;

	printk("Page Mapping:\n");

	/* Display the memory map we currently have to aid in board bringups */
	for (index = 0; index < pmap->count; index++) {
		if (entry[index].type != MULTIBOOT_MEMORY_AVAILABLE) {
			continue;
		}

		switch (entry[index].flags) {
		case MEMORY_PMAP_KERNEL:
			printk("  kernel: ");
			break;
		case MEMORY_PMAP_LOADER:
			printk("  loader: ");
			break;
		case MEMORY_PMAP_MODULE:
			printk("  module: ");
			break;
		default:
			printk("  unused: ");
			break;
		}

		printk("start=0x%x, end=0x%x\n", entry[index].start,
				entry[index].end);
	}

	printk("No platform paging initialization defined\n");
	return 0;
}
weak_symbol(__init_paging, init_paging);

/** Start of OS independant initialization
 * @magic Multiboot magic number
 * @addr Address of Multiboot Information Structure
 * Parse the Multiboot Information and initialize the system */
int fmios_init(unsigned long magic, unsigned long addr)
{
	struct pmap_table *pmap;
	char *cmdline = "";

	kernel_start = (unsigned long)&__kernel_start;
	kernel_end = (unsigned long)&_end;

	if (!mb_init(addr, magic)) {
		return 1;
	}

	cmdline = mb_mbi_cmdline();

	init_serial(cmdline);

	/* Grab the curent video configuration from within multiboot. */
	if (mb_fb_type()) {
		struct video_config fb;

		memset(&fb, 0, sizeof(struct video_config));

		fb.addr = mb_fb_addr();
		fb.pitch = mb_fb_pitch();
		fb.width = mb_fb_width();
		fb.height = mb_fb_height();
		fb.depth = mb_fb_depth();
		fb.type = mb_fb_type();

		if (fb.type) {
			init_video(cmdline, &fb);
		}
	}

	printk("%s v%s\n", PACKAGE_NAME, PACKAGE_VERSION);

	/* Initialize the memory allocator subsystem.  The allocator is not
	 * usable until after paging is enabled, but we can not initialize
	 * paging until we have the initial bit-buckets for malloc setup and
	 * have a map of existing memory */
	pmap = init_malloc();
	if (!pmap) {
		printk("error initializing memory\n");
		return 1;
	}

	if (!init_paging(pmap)) {
		printk("error initializing paging\n");
		return 1;
	}
	printk("Paging enabled.\n");

	/* At this point we return to to boot.S/entry.S to clear the stack and
	 * to allow any extra platform specific code to be fired off.  From
	 * there the platform specific code needs to enter the scheduler */
	return 0;
}
