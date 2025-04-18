/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef __CONFIG_XPLAYER

#include <stdlib.h>
#include <string.h>
#include "fs/fatfs/ff.h"
#include "cdx_fs.h"

#define DBG(fmt, ...)   printf("[Cedarx File DBG] %s line %d, " fmt, __func__, __LINE__, ##__VA_ARGS__);

FILE *cdx_fopen(const char *filename, const char *mode)
{
	unsigned char fmode = 0;

	//DBG("filename - %s.\n", filename);
	FIL * file = malloc(sizeof(FIL));
	if (file == NULL)
		goto out;

	if (strstr(mode, "a"))
		fmode |= FA_OPEN_APPEND;
	if (strstr(mode, "+"))
		fmode |= FA_READ | FA_WRITE;
	if (strstr(mode, "w"))
		fmode |= FA_WRITE | FA_CREATE_ALWAYS;
	if (strstr(mode, "r"))
		fmode |= FA_READ | FA_OPEN_EXISTING;

	FRESULT res = f_open(file, filename, fmode);
	if (res != FR_OK) {
		DBG("open file\"%s\" failed: %d\n", filename, res);
		free(file);
		file = NULL;
	}

out:
	return (FILE *)file;
}

int cdx_fread(void *ptr, int size, int nmemb, FILE *stream)
{
	unsigned int ret;
	FRESULT res;

	if (stream == NULL)
		return -1;

	res = f_read((FIL *)stream, ptr, size * nmemb, &ret);
	if (res != FR_OK) {
		DBG("read file failed: %d\n", res);
		return (int)-res;
	}

	return (int)(ret / size);
}

int cdx_fwrite(const void *ptr, int size, int nmemb, FILE *stream)
{
	unsigned int ret;
	FRESULT res;

	if (stream == NULL)
		return -1;

	res = f_write((FIL *)stream, ptr, size * nmemb, &ret);
	if (res != FR_OK) {
		DBG("read file failed: %d\n", res);
		return (int)-res;
	}

	return (int)(ret / size);
}

int cdx_fseek(FILE *stream, long long offset, int whence)
{
	FRESULT res;
	long long foffset;

	if (stream == NULL)
		return -1;

	if (whence == SEEK_SET)
		foffset = offset;
	else if (whence == SEEK_CUR)
		foffset = f_tell((FIL *)stream) + offset;
	else if (whence == SEEK_END)
		foffset = f_size((FIL *)stream) + offset;
	else
		return -1;

	res = f_lseek((FIL *)stream, foffset);
	if (res != FR_OK)
		DBG("read file failed: %d\n", res);

	return (int)-res;
}

long long cdx_ftell(FILE *stream)
{
	if (stream == NULL)
		return -1;

	return f_tell((FIL *)stream);
}

int cdx_fclose(FILE *stream)
{
	FRESULT res;

	if (stream == NULL)
		return -1;

	res = f_close((FIL *)stream);
	if (res == FR_OK)
		free(stream);

	return (int)-res;
}

int cdx_unlink(const char *filename)
{
	FRESULT res;

	res = f_unlink(filename);
	return (int)-res;
}

#endif
