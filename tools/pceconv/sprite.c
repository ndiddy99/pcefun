#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "qdbmp.h"
#include "sprite.h"

#define PALETTE_SIZE (16)
static uint16_t **palette_list;
static int palette_cursor;

typedef struct {
	int x;
	int y;
	uint8_t pal;	
	uint8_t *graphics;
} IMAGE_INFO;

static IMAGE_INFO *info_list;
static int info_cursor;

int sprite_convert(BMP *tile) {
	uint16_t palette_buffer[PALETTE_SIZE];
	uint8_t r;
	uint8_t g;
	uint8_t b;
	
	IMAGE_INFO *info = &info_list[info_cursor];
	info_cursor++;
	
	info->x = BMP_GetWidth(tile);
	info->y = BMP_GetHeight(tile);

	DBG_PRINTF("Width: %d Height: %d\n", info->x, info->y);
	if ((info->x % 16) != 0) {
		printf("Error: Sprite width not divisible by 16!\n");
		return 0;
	}
	else if ((info->y % 16) != 0) {
		printf("Error: Sprite height not divisible by 16!\n");
		return 0;
	}
	
	// read in the palette data
	uint16_t color;
	for (int i = 0; i < PALETTE_SIZE; i++) {
		BMP_GetPaletteColor(tile, i, &r, &g, &b);
		r >>= 5;
		g >>= 5;
		b >>= 5;
		color = (g << 6) | (r << 3) | b;
		palette_buffer[i] = color;
	}
	// check for a matching palette
	int palno = -1;
	for (int i = 0; i < palette_cursor; i++) {
		if (memcmp(palette_buffer, palette_list[i], PALETTE_SIZE * sizeof(uint16_t)) == 0) {
			palno = i;
			break;
		}
	}
	
	// add one if there isn't one
	if (palno == -1) {
		uint16_t *palette = palette_list[palette_cursor];
		memcpy(palette, palette_buffer, PALETTE_SIZE * sizeof(uint16_t));
		info->pal = palette_cursor;
		palette_cursor++;
	}
	else {
		info->pal = palno;
	}
	
	// read in the graphics data
	info->graphics = calloc(1, (info->x >> 1) * info->y);
	uint8_t *graphics_cur = info->graphics;
	uint8_t value;
	uint16_t plane1;
	uint16_t plane2;
	uint16_t plane3;
	uint16_t plane4;
	for (int y = 0; y < info->y; y += 16) {
		for (int x = 0; x < info->x; x += 16) {
			for (int yt = 0; yt < 16; yt++) {
				plane1 = 0;
				plane2 = 0;
				plane3 = 0;
				plane4 = 0;
				for (int xt = 0; xt < 16; xt++) {
					BMP_GetPixelIndex(tile, x + xt, y + yt, &value);
					if (value & 1) {
						plane1 |= 1;
					}
					if (value & 2) {
						plane2 |= 1;
					}
					if (value & 4) {
						plane3 |= 1;
					}
					if (value & 8) {
						plane4 |= 1;
					}
					plane1 <<= 1;
					plane2 <<= 1;
					plane3 <<= 1;
					plane4 <<= 1;
				}
				*graphics_cur = plane1 & 0xff;
				*(graphics_cur + 1) = plane1 >> 8;
				*(graphics_cur + 32) = plane2 & 0xff;
				*(graphics_cur + 33) = plane2 >> 8;
				*(graphics_cur + 64) = plane3 & 0xff;
				*(graphics_cur + 65) = plane3 >> 8;
				*(graphics_cur + 96) = plane4 & 0xff;
				*(graphics_cur + 97) = plane4 >> 8;
				graphics_cur += 2;
			}
			graphics_cur += 96;
		}
	}
	return 1;
}

#define FILENAME_BUFLEN (256)

int sprite_process(char *dirname) {
	struct dirent *entry;
	DIR *dir_ptr;
	palette_cursor = 0;
	info_cursor = 0;
	char filename[FILENAME_BUFLEN];
	char spr_filename[FILENAME_BUFLEN];
	char pal_filename[FILENAME_BUFLEN];
	char ref_filename[FILENAME_BUFLEN];
	
	snprintf(spr_filename, FILENAME_BUFLEN, "%s.spr", dirname);
	snprintf(pal_filename, FILENAME_BUFLEN, "%s.pal", dirname);
	snprintf(ref_filename, FILENAME_BUFLEN, "%s.ref", dirname);
	
	// get number of files in the directory
	int num_files = 0;
	dir_ptr = opendir(dirname);
	if (dir_ptr == NULL) {
		printf("Error: directory %s doesn't exist.\n", dirname);
		return 0;
	}
	while ((entry = readdir(dir_ptr)) != NULL) {
		if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
			continue;
		}
		if (entry->d_type == DT_REG) {
			num_files++;
		}
	}
	closedir(dir_ptr);
	
	// allocate memory based on the number of files
	// the maximum number of palettes possible is one per frame
	palette_list = malloc(num_files * sizeof(uint32_t *));
	for (int i = 0; i < num_files; i++) {
		palette_list[i] = malloc(PALETTE_SIZE * sizeof(uint32_t));
	}
	info_list = malloc(num_files * sizeof(IMAGE_INFO));
	
	// read in all the files
	dir_ptr = opendir(dirname);
	if (dir_ptr == NULL) {
		printf("Error: directory %s doesn't exist.\n", dirname);
		return 0;
	}
	
	int processed = 0;
	while ((entry = readdir(dir_ptr)) != NULL) {
		if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
			continue;
		}
		if (entry->d_type == DT_REG) {
			DBG_PRINTF("%s/%s\n", dirname, entry->d_name);
			int retval;
			if ((retval = snprintf(filename, FILENAME_BUFLEN, "%s/%s", dirname, entry->d_name)) > FILENAME_BUFLEN) {
				printf("Error: overran filename buffer by %d bytes. Increase FILENAME_BUFLEN define in sprite.c.\n", retval);
				closedir(dir_ptr);
				return 0;
			}
			DBG_PRINTF("Reading file %s\n", filename);
			BMP *tiledata = BMP_ReadFile(filename);
			// BMP_CHECK_ERROR(stdout, 0);
			if ((BMP_GetDepth(tiledata) != 4) && (BMP_GetDepth(tiledata) != 8)) {
				printf("Error: file %s isn't indexed.\n", entry->d_name);
				BMP_Free(tiledata);
				closedir(dir_ptr);
				return 0;
			}
			if (!sprite_convert(tiledata)) {
				num_files = processed;
				goto err_exit;
			}
			processed++;
			BMP_Free(tiledata);
		}
	}
	closedir(dir_ptr);
	
	// write out the sprite data
	FILE *spr_file = fopen(spr_filename, "wb");
	if (!spr_file) {
		printf("Error: couldn't open %s for writing!\n", spr_filename);
		goto err_exit;
	}
	FILE *pal_file = fopen(pal_filename, "wb");
	if (!pal_file) {
		printf("Error: couldn't open %s for writing!\n", pal_filename);
		goto err_exit;
	}
	FILE *ref_file = fopen(ref_filename, "wb");
	if (!ref_file) {
		printf("Error: couldn't open %s for writing!\n", ref_filename);
		goto err_exit;
	}	
	// write all the palettes
	for (int i = 0; i < palette_cursor; i++) {
		fwrite(palette_list[i], sizeof(uint16_t), PALETTE_SIZE, pal_file);
	}
	// write IMAGE_INFO structs
	for (int i = 0; i < num_files; i++) {
		fwrite(info_list[i].graphics, sizeof(uint8_t), (info_list[i].x >> 1) * info_list[i].y, spr_file);
		fwrite(&info_list[i].pal, sizeof(uint8_t), 1, ref_file);
	}
	fclose(spr_file);
	fclose(pal_file);
	fclose(ref_file);
	// clean up memory
	for (int i = 0; i < num_files; i++) {
		free(palette_list[i]);
		free(info_list[i].graphics);
	}
	free(palette_list);
	free(info_list);
	
	return 1;
	
err_exit:
	for (int i = 0; i < num_files; i++) {
		free(palette_list[i]);
		free(info_list[i].graphics);
	}
	free(palette_list);
	free(info_list);
	return 0;
}
