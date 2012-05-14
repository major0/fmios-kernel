#include <fmios/fmios.h>
#include <string.h>

char *cmdline_get_opt(char *cmdline, char *option)
{
	int len;
	char *param;

	if (!option || !cmdline) {
		return NULL;
	}

	if (!strlen(option) || !strlen(cmdline)) {
		return NULL;
	}

	len = strlen(option);

	while (*cmdline != '\0') {

		if (*cmdline == ' ') {
			cmdline++;
			continue;
		}

		if (strncmp(option, cmdline, len) == 0) {
			cmdline += len;
			param = cmdline;

			if (*param == '=') {
				return ++param;
			}
		}

		while (*cmdline != '\0' && *cmdline != ' ') {
			cmdline++;
		}
	}

	return NULL;
}


