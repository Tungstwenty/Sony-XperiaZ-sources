/*
 * Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * Author: Lars Svensson <lars1.svensson@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern int e2fsck_main(int argc, char **argv);
extern int tune2fs_main(int argc, char **argv);
extern int mke2fs_main(int argc, char **argv);

char *program_name = "e2fstool";

int main(int argc, char **argv) {
	int i;
	char *tmp, *cmd = argv[0];

	if ((argc > 1) && (argv[1][0] == '@')) {
		cmd = argv[1] + 1;
		argc--;
		argv++;
	} else if ((tmp = strrchr(argv[0], '/')) != '\0') {
		cmd = tmp + 1;
	}

	if (strstr(cmd, "e2fsck")) {
		program_name = "e2fsck_static";
		return e2fsck_main(argc, argv);
	}
	if (strstr(cmd, "tune2fs")) {
		program_name = "tune2fs_static";
		return tune2fs_main(argc, argv);
	}
	if (strstr(cmd, "mke2fs")) {
		program_name = "mke2fs";
		return mke2fs_main(argc, argv);
	}

	fprintf(stderr, "Usage: %s @<tool>\n", argv[0]);
	fprintf(stderr, "Supported tools: e2fsck tune2fs mke2fs\n");

	exit(EXIT_FAILURE);
}
