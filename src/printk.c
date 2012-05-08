#include <asm/io.h>

static void kputc(int c)
{
	ega_putc(c);
	serial_putc(c);
#if 0
	klog_putc(c);
#endif
}

/* Format a string and print it on the screen, just like the libc
   function printf. */
void printk (const char *format, ...)
{
	char **arg = (char **)&format;
	char buf[20];
	int c;

	arg++;

	while ((c = *format++) != 0) {
		if (c != '%')
			kputc(c);
		else {
			char *p;

			c = *format++;

			switch (c) {
			case 'd':
			case 'u':
			case 'x':
				itoa(buf, c, *((int *) arg++));
				p = buf;
				goto string;
				break;

			case 's':
				p = *arg++;
				if (!p) p = "(null)";
			string:
				while (*p)
					kputc(*p++);
				break;
			default:
				kputc(*((int *)arg++));
				break;
			}
		}
	}
}
