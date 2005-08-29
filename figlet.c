/*
 * figlet.c -- figlet support for vtclock.
 *
 * caveats:
 * - ignores kerning entirely, not good with slanted figlet fonts.
 * - invokes figlet once for each character.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "vtclock.h"
#include "font.h"
#include "figlet.h"

#define CHUNK_SIZE 2048

struct char_dims { 
	int width;
	int height;
	int left_x;
	int right_x;
};

char *
get_command_output (const char *cmd)
{
	FILE *pipe;
	char *out;
	size_t out_size;
	size_t out_len;
	size_t buf_len;
	char buf[CHUNK_SIZE];
	pipe = popen(cmd, "r");
	if (pipe == NULL)
		return NULL;
	out = NULL;
	out_len = 0;
	while (fgets(buf, CHUNK_SIZE, pipe)) {
		buf_len = strlen(buf);
		if (out == NULL) {
			out = (char *)malloc(CHUNK_SIZE);
			out_size = CHUNK_SIZE;
			*out = '\0';
		}
		else if (out_len + buf_len + 1 >= out_size) {
			out = (char *)realloc(out, out_size += CHUNK_SIZE);
		}
		strcpy(out + out_len, buf); /* strcat optimized */
		out_len += buf_len;
	}
	pclose(pipe);
	return out;
}

struct char_dims
get_character_dimensions (const char *figchar)
{
	/* assume spaces, no tabs */
	struct char_dims dims;
	int x;
	const char *p;

	x = 0;
	dims.width = 0;
	dims.height = 0;
	dims.left_x = -1;
	dims.right_x = -1;

	for (p = figchar; *p; ++p) {
		if (*p == '\r' || *p == '\n') {
			++dims.height;
			if (dims.width < x) dims.width = x;
			x = 0;
		} else if (isspace(*p)) {
			++x;
		} else {
			if (dims.left_x == -1 || dims.left_x > x)
				dims.left_x = x;
			if (dims.right_x == -1 || dims.right_x < x)
				dims.right_x = x;
			++x;
		}
	}
	if (x) {
		++dims.height;
		if (dims.width < x) dims.width = x;
	}
	++dims.width;
	return dims;
}

char *
trim_glyph (char *glyph)
{
	struct char_dims dims;
	int chop_left;
	int chop_right;
	int x;
	char *new_glyph_p;
	char *glyph_p;

	char *new_glyph = (char *)malloc(strlen(glyph) + 1);
	*new_glyph = '\0';

	dims = get_character_dimensions(glyph);
	chop_left = dims.left_x;
	chop_right = dims.width - dims.right_x - 1;

	x = 0;
	for (new_glyph_p = new_glyph, glyph_p = glyph; *glyph_p; ++glyph_p) {
		if (*glyph_p == '\r' || *glyph_p == '\n') {
			*new_glyph_p++ = *glyph_p;
			x = 0;
		}
		else {
			if (x >= dims.left_x && x <= dims.right_x) {
				*new_glyph_p++ = *glyph_p;
			}
			++x;
		}
	}
	*new_glyph_p = '\0';

	strcpy(glyph, new_glyph);
	free(new_glyph);
	return glyph;
}

char *
recenter_glyph (char *glyph, int width)
{
	struct char_dims dims;
	int shift;
	int pos;
	int i;
	
	dims = get_character_dimensions(glyph);
	shift = (width - dims.width + 1) / 2;

	if (shift == 0)
		return glyph;
	
	glyph = realloc(glyph, strlen(glyph) + shift * dims.height + 1);

	pos = strlen(glyph) - 1;
	while (1) {
		/* move backward to beginning of string or character 
		   immediately following \r or \n, whichever comes first. */
		for (; pos && glyph[pos-1] != '\n' 
			     && glyph[pos-1] != '\r'; --pos);
		memmove(glyph + pos + shift, 
			glyph + pos, 
			strlen(glyph + pos) + 1);
		for (i = 0; i < shift; ++i)
			glyph[pos + i] = ' ';
		if (pos < 1) break;
		--pos;
		/* skip past \r and \n characters. */
		for (; pos >= 0 && (glyph[pos] == '\n' 
				    || glyph[pos] == '\r'); --pos);
		if (pos < 0) break;
	}
	
	return glyph;
}

vtclock_font *
generate_figlet_font (figlet_options *options) 
{
	int debug;
	vtclock_font *font;
	int i, j;
	char *glyph;
	size_t glyph_size = 0;
	char *cp;
	FILE *pipe;
	int char_lines;
	int char_width;
	char figlet_line[CHUNK_SIZE];
	struct char_dims dims;
	char *figlet_cmd;
	size_t figlet_cmd_len;

	debug = (getenv("VTCLOCK_DEBUG") != NULL);
	
	font = (vtclock_font *)malloc(sizeof(vtclock_font));
	font->digit_height = 0;
	font->digit_width = 0;
	font->colon_width = 0;
	for (i = 0; i < 10; ++i) {
		font->digits[i] = NULL;
	}
	font->colon = NULL;
	
	figlet_cmd_len = strlen("figlet -W 'x'");
	if (options && options->font_name && strlen(options->font_name)) {
		figlet_cmd_len += strlen(" -f ") + strlen(options->font_name);
	}
	figlet_cmd = (char *)malloc(figlet_cmd_len + 1);
	
	for (i = 0; i <= 10; ++i) { /* 0-9 = digits, 10 = colon */
		sprintf(figlet_cmd, "figlet -W '%c'", 
			(i == 10) ? ':' : (i + '0'));
		if (options && options->font_name 
		    && strlen(options->font_name)) {
			sprintf(figlet_cmd + strlen(figlet_cmd),
				" -f %s", options->font_name);
		}

		glyph = get_command_output(figlet_cmd);
		if (glyph == NULL) {
			goto err;
		}
		trim_glyph(glyph);
		dims = get_character_dimensions(glyph);
		if (font->digit_height < dims.height)
			font->digit_height = dims.height;
		if (i == 10) {
			font->colon = glyph;
			if (font->colon_width < dims.width)
				font->colon_width = dims.width;
		} else {
			font->digits[i] = glyph;
			if (font->digit_width < dims.width)
				font->digit_width = dims.width;
		}
	}

	for (i = 0; i < 10; ++i) {
		font->digits[i] = recenter_glyph(font->digits[i],
						 font->digit_width);
	}

	/* extra space */
	++font->colon_width;
	++font->digit_width;

	free(figlet_cmd);
	return font;
 err:
	for (j = 0; j < 10; ++j) {
		if (font->digits[i]) free(font->digits[i]);
	}
	if (font->colon)
		free(font->colon);
	if (font)
		free(font);
	if (figlet_cmd)
		free(figlet_cmd);
	return NULL;
}

vtclock_config *
generate_figlet_config (figlet_options *options)
{
	vtclock_font *font;
	vtclock_config *config;
	font = generate_figlet_font(options);
	if (font == NULL)
		return NULL;
	config = (vtclock_config *)malloc(sizeof(vtclock_config));
	config->hour   = font;
	config->minute = font;
	config->second = font;
	config->colon1 = font;
	config->colon2 = font;
	return config;
}

