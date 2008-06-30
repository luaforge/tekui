
/*
**	UTF-8 encoder / decoder
**	Written by Timm S. Mueller
**	Placed in the Public Domain
**
**	References:
**	http://www.cl.cam.ac.uk/~mgk25/unicode.html
**	http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "display_dfb_mod.h"

/*****************************************************************************/

/*
**	Encode unicode char (31bit) to UTF-8 (up to 6 chars)
**	Reserve AT LEAST 6 bytes free space in the destination buffer
*/

LOCAL unsigned char *encodeutf8(unsigned char *buf, int c)
{
	if (c < 128)
	{
		*buf++ = c;
	}
	else if (c < 2048)
	{
		*buf++ = 0xc0 + (c >> 6);
		*buf++ = 0x80 + (c & 0x3f);
	}
	else if (c < 65536)
	{
		*buf++ = 0xe0 + (c >> 12);
		*buf++ = 0x80 + ((c & 0xfff) >> 6);
		*buf++ = 0x80 + (c & 0x3f);
	}
	else if (c < 2097152)
	{
		*buf++ = 0xf0 + (c >> 18);
		*buf++ = 0x80 + ((c & 0x3ffff) >> 12);
		*buf++ = 0x80 + ((c & 0xfff) >> 6);
		*buf++ = 0x80 + (c & 0x3f);
	}
	else if (c < 67108864)
	{
		*buf++ = 0xf8 + (c >> 24);
		*buf++ = 0x80 + ((c & 0xffffff) >> 18);
		*buf++ = 0x80 + ((c & 0x3ffff) >> 12);
		*buf++ = 0x80 + ((c & 0xfff) >> 6);
		*buf++ = 0x80 + (c & 0x3f);
	}
	else
	{
		*buf++ = 0xfc + (c >> 30);
		*buf++ = 0x80 + ((c & 0x3fffffff) >> 24);
		*buf++ = 0x80 + ((c & 0xffffff) >> 18);
		*buf++ = 0x80 + ((c & 0x3ffff) >> 12);
		*buf++ = 0x80 + ((c & 0xfff) >> 6);
		*buf++ = 0x80 + (c & 0x3f);
	}
	return buf;
}

/*****************************************************************************/

/*
**	UTF-8 reader
**	Note: You must supply length, the reader treats 0 as a valid char!
**
**		struct readstringdata rs;
**		struct utf8reader rd;
**		int c;
**
**		rs.src = utf-8-string
**		rs.srclen = length_in_bytes
**
**		rd.readchar = readstring;
**		rd.accu = 0;
**		rd.numa = 0;
**		rd.bufc = -1;
**		rd.udata = &rs;
**
**		while ((c = readutf8(&rd)) >= 0)
**		{
**			// unicode charcode (31bit) in c
**		}
*/

#if 0

struct utf8reader
{
	/* character reader callback: */
	int (*readchar)(struct utf8reader *);
	/* reader state: */
	int accu, numa, min, bufc;
	/* userdata to reader */
	void *udata;
};

struct readstringdata
{
	/* src string: */
	const unsigned char *src;
	/* src string length: */
	size_t srclen;
};

int readstring(struct utf8reader *rd)
{
	struct readstringdata *ud = rd->udata;
	if (ud->srclen == 0)
		return -1;
	ud->srclen--;
	return *ud->src++;
}

#endif

LOCAL int readutf8(struct utf8reader *rd)
{
	int c;
	for (;;)
	{
		if (rd->bufc >= 0)
		{
			c = rd->bufc;
			rd->bufc = -1;
		}
		else
			c = rd->readchar(rd);
		if (c < 0)
			return c;

		if (c == 254 || c == 255)
			break;

		if (c < 128)
		{
			if (rd->numa > 0)
			{
				rd->bufc = c;
				break;
			}
			return c;
		}
		else if (c < 192)
		{
			if (rd->numa == 0)
				break;
			rd->accu <<= 6;
			rd->accu += c - 128;
			rd->numa--;
			if (rd->numa == 0)
			{
				if (rd->accu == 0 || rd->accu < rd->min ||
					(rd->accu >= 55296 && rd->accu <= 57343))
					break;
				c = rd->accu;
				rd->accu = 0;
				return c;
			}
		}
		else
		{
			if (rd->numa > 0)
			{
				rd->bufc = c;
				break;
			}

			if (c < 224)
			{
				rd->min = 128;
				rd->accu = c - 192;
				rd->numa = 1;
			}
			else if (c < 240)
			{
				rd->min = 2048;
				rd->accu = c - 224;
				rd->numa = 2;
			}
			else if (c < 248)
			{
				rd->min = 65536;
				rd->accu = c - 240;
				rd->numa = 3;
			}
			else if (c < 252)
			{
				rd->min = 2097152;
				rd->accu = c - 248;
				rd->numa = 4;
			}
			else
			{
				rd->min = 67108864;
				rd->accu = c - 252;
				rd->numa = 5;
			}
		}
	}
	/* bad char */
	rd->accu = 0;
	rd->numa = 0;
	return 65533;
}

#if defined(TEST)
/*****************************************************************************/
/*
**	Test usage:
**	# wget http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt
**	# ./utf8test < UTF-8-test.txt > out.txt
*/

struct readfiledata
{
	FILE *file;
};


int readfile(struct utf8reader *rd)
{
	struct readfiledata *ud = rd->udata;
	return fgetc(ud->file);
}


int main(int argc, char **argv)
{
	struct readfiledata rs;
	struct utf8reader rd;
	int c, ln = 1;
	char outbuf[6], *bufend;

	rs.file = stdin;

	rd.readchar = readfile;
	rd.accu = 0;
	rd.numa = 0;
	rd.bufc = -1;
	rd.udata = &rs;

	while ((c = readutf8(&rd)) >= 0)
	{
		/* unicode charcode (31bit) in c */
		if (c == 65533)
			fprintf(stderr, "Bad UTF-8 encoding / char in line %d\n", ln);
		bufend = encodeutf8(outbuf, c);
		fwrite(outbuf, bufend - outbuf, 1, stdout);
		if (c == 10)
			ln++;
	}

	return 1;
}
#endif
