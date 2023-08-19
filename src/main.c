/**
 * This file is part of responsive.
 * Copyright (C) 2023 Ahmed Al-Shoqanebi
 *
 * responsive is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>

char* read_whole_file(FILE *);
bool strprefixed(const char *str, const char *prefix);
int count_char_occur(const char *str, const char ch);
bool is_drive_rotational(const char *dev);

#define OUT_OF_MEMORY() {write(fileno(stderr), "Out of memory!\n", 15); _exit(1);}
#define IN_RANGE(x, a, b) (x >= a && x <= b)

typedef enum {
	DRIVE_TYPE_NONE = -1,
	DRIVE_TYPE_IDE = 0,
	DRIVE_TYPE_SATA_HDD,
	DRIVE_TYPE_SATA_SSD,
	DRIVE_TYPE_PCIe_NVME,
	DRIVE_TYPE_SCSI_HDD,
	DRIVE_TYPE_SCSI_SSD,
} DriveType;

char *drive_type_to_str(DriveType);

int main() {
	FILE *mounts_fd = fopen("/proc/mounts", "r");
	if (mounts_fd == NULL) {
		perror("Couldn't open /proc/mounts");
		return 1;
	}

	ssize_t read = 0;
	size_t len = 0;
	char *l = NULL;
	DriveType root_drive_type = DRIVE_TYPE_NONE;
	while ((read = getline(&l, &len, mounts_fd)) != -1) {
		char *line = strdup(l);
		char *source  = strtok(line, " ");
		char *target  = strtok(NULL, " ");
		char *fstype  = strtok(NULL, " ");
		char *options = strtok(NULL, " ");
		char *freq    = strtok(NULL, " ");
		char *passno  = strtok(NULL, "\n");

		// printf("Line: %s", l);
		// printf("Source: %s\n", source);
		// printf("Target: %s\n", target);
		// printf("FS Type: %s\n", fstype);
		// printf("Options: %s\n", options);
		// printf("Freq: %s\n", freq);
		// printf("Passno: %s\n", passno);
		if (strcmp(target, "/") != 0) {
			continue;
		}

		if (strprefixed(source, "/dev/nvme")) {
			// Make sure the path consists of 2 segments
			int path_segments = count_char_occur(source, '/');
			if (path_segments != 2) {
				fprintf(stderr, "Fatal: Unexpected device name in devfs (/dev): %s\n", source);
				return 1;
			}

			char *filename = basename(source);
			size_t filename_len = strlen(filename);
		}
		else if (strprefixed(source, "/dev/sd")) {
			int path_segments = count_char_occur(source, '/');
			if (path_segments != 2) {
				fprintf(stderr, "Fatal: Unexpected device name in devfs (/dev): %s\n", source);
				return 1;
			}

			char *filename = basename(source);
			size_t filename_len = strlen(filename);
			if (IN_RANGE(filename[filename_len - 1], '0', '9')) {
				filename_len -= 1;
			}
			const char *path_prefix = "/sys/block/";
			const char *path_suffix = "/queue/rotational";
			char *sys_block_rotational_path = calloc(1,
				strlen(path_prefix)
				+ filename_len
				+ strlen(path_suffix)
				+ 1
			);
			if (sys_block_rotational_path == NULL) {
				OUT_OF_MEMORY();
			}

			char *stop = sys_block_rotational_path;
			stop = stpcpy(stop, path_prefix);
			stop = stpncpy(stop, filename, filename_len);
			stop = stpcpy(stop, path_suffix);
			FILE *sys_block_rotational = fopen(sys_block_rotational_path, "r");
			if (sys_block_rotational_path == NULL) {
				char *error_message = NULL;
				asprintf(&error_message, "Error opening: %s", sys_block_rotational_path);
				perror(error_message);
				return 1;
			}

			char rotational;
			fread(&rotational, 1, 1, sys_block_rotational);
			if (rotational == '0') {
				root_drive_type = DRIVE_TYPE_SATA_SSD;
			}
			else if (rotational == '1') {
				root_drive_type = DRIVE_TYPE_SATA_HDD;
			}

			break;
		}
		// For fucks sake why do you have your root on an IDE drive?
		// It's 2023 grandpa, nobody uses IDE.
		else if (strprefixed(source, "/dev/hd")) {
			int path_segments = count_char_occur(source, '/');
			if (path_segments != 2) {
				fprintf(stderr, "Fatal: Unexpected device name in devfs (/dev): %s\n", source);
				return 1;
			}

			char *filename = basename(source);
			size_t filename_len = strlen(filename);
		}

		// bool isMapper = strprefixed(source, "/dev/mapper");
		fprintf(stderr, "Unrecognized drive type: %s\n", source);
		return 1;
		break;
	}
	printf("Drive type: %s\n", drive_type_to_str(root_drive_type));

	// FILE *swappiness_fd = fopen("/proc/sys/vm/swappiness", "r");
	// if (swappiness_fd == NULL) {
	// 	perror("Couldn't open /proc/sys/vm/swappiness");
	// 	return 1;
	// }

	return 0;
}

char* read_whole_file(FILE *file) {
	fseek(file, 0, SEEK_END);
	ssize_t mounts_size = ftell(file);
	rewind(file);
	char *mounts = malloc(mounts_size);
	if (mounts == NULL) {
		OUT_OF_MEMORY();
	}

	fread(mounts, mounts_size, 1, file);
	return mounts;
}

bool strprefixed(const char *str, const char *prefix) {
	return strncmp(str, prefix, strlen(prefix)) == 0;
}

int count_char_occur(const char *str, const char ch) {
	size_t i = 0;

	while (*str != 0) {
		if (*str == ch) i++;
		str++;
	}

	return i;
}

bool is_drive_rotational(const char *dev) {
	return true;
}

char *drive_type_to_str(DriveType drive_type) {
	switch(drive_type) {
		case DRIVE_TYPE_NONE:
			return "DRIVE_TYPE_NONE";
		case DRIVE_TYPE_SATA_HDD:
			return "DRIVE_TYPE_SATA_HDD";
		case DRIVE_TYPE_SATA_SSD:
			return "DRIVE_TYPE_SATA_SSD";
		case DRIVE_TYPE_IDE:
			return "DRIVE_TYPE_IDE";
		case DRIVE_TYPE_SCSI_HDD:
			return "DRIVE_TYPE_SCSI_HDD";
		case DRIVE_TYPE_SCSI_SSD:
			return "DRIVE_TYPE_SCSI_SSD";
		case DRIVE_TYPE_PCIe_NVME:
			return "DRIVE_TYPE_PCIe_NVME";
		default:
			return "ERROR_UNKNOWN";
	}
}
