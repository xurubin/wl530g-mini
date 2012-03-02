/*
 *  user/de2ts-cal/de2ts-cal.c -- Touchscreen driver calibration
 *
 *	Copyright (C) 2003 Georges Menie
 *
 *  This is a DragonEngine board specific app.
 *
 *  Run it on this board, two arrows will be displayed, use the touchscreen
 *  to hit the arrows then two little square will be displayed to
 *  check the calibration. If one of the squares are not clicked right,
 *  the calibration start over. If both are ok, then use the parameters
 *  shown on the tty console to modify the microwindows driver:
 *  user/microwindows/src/drivers/mou_de2ts.c
 *
 *  If you are not using microwindows, see the fixed_setting() function
 *  below, use it to initialize the de2ts touchscreen driver
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License. See the file COPYING in the main directory of this archive for
 *  more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/de2ts.h>

/* set to 1 if you don't want to calibrate the touchscreen
 * but want to use some reasonnable default value
 */
#define FIXED_CALIBRATION 0

/* set to 0 if you don't want the calibration data to be
 * written in the eeprom
 */
#define EEPROM_STORED_CALDATA 1

unsigned char *screen_ptr;
int screen_width;
int screen_height;

inline void draw_pixel(int x, int y, int color)
{
	int mask = 1 << (7 - (x % 8));
	unsigned char *loc = screen_ptr + (y * screen_width / 8) + x / 8;

	if ((x < 0) || (x >= screen_width) || (y < 0) || (y >= screen_height))
		return;

	if (color)
		*loc |= mask;
	else
		*loc &= ~mask;
}

/* Abrash's take on the simplest Bresenham line-drawing algorithm. 
 *
 * This isn't especially efficient, as we aren't combining the bit-mask
 * logic and addresses with the line drawing code, never mind higher
 * level optimizations like run-length slicing, etc.
 *
 */

inline void draw_xish_line(int x, int y, int dx, int dy, int xdir,
						   int color)
{
	int dyX2;
	int dyX2mdxX2;
	int error;

	dyX2 = dy * 2;
	dyX2mdxX2 = dyX2 - (dx * 2);
	error = dyX2 - dx;

	draw_pixel(x, y, color);
	while (dx--) {
		if (error >= 0) {
			y++;
			error += dyX2mdxX2;
		} else {
			error += dyX2;
		}
		x += xdir;
		draw_pixel(x, y, color);
	}
}

inline void draw_yish_line(int x, int y, int dx, int dy, int xdir,
						   int color)
{
	int dxX2;
	int dxX2mdyX2;
	int error;

	dxX2 = dx * 2;
	dxX2mdyX2 = dxX2 - (dy * 2);
	error = dxX2 - dy;

	draw_pixel(x, y, color);
	while (dy--) {
		if (error >= 0) {
			x += xdir;
			error += dxX2mdyX2;
		} else {
			error += dxX2;
		}
		y++;
		draw_pixel(x, y, color);
	}
}

void draw_line(int x1, int y1, int x2, int y2, int color)
{
	int dx, dy;

	if (y1 > y2) {
		int t = y1;
		y1 = y2;
		y2 = t;
		t = x1;
		x1 = x2;
		x2 = t;
	}

	dx = x2 - x1;
	dy = y2 - y1;

	if (dx > 0) {
		if (dx > dy)
			draw_xish_line(x1, y1, dx, dy, 1, color);
		else
			draw_yish_line(x1, y1, dx, dy, 1, color);
	} else {
		dx = -dx;
		if (dx > dy)
			draw_xish_line(x1, y1, dx, dy, -1, color);
		else
			draw_yish_line(x1, y1, dx, dy, -1, color);
	}


}

void print_prm(struct de2ts_cal_params *prm)
{
	printf("version: %d\n", prm->version);
	printf("xoff: %d\n", prm->xoff);
	printf("xden: %d\n", prm->xden);
	printf("yoff: %d\n", prm->yoff);
	printf("yden: %d\n", prm->yden);
	printf("xrng: %d\n", prm->xrng);
	printf("yrng: %d\n", prm->yrng);
}

#if EEPROM_STORED_CALDATA

#define EEPROM_CAL_MAGIC 0xbebe2003

static struct {
	int magic;
	struct de2ts_cal_params prm;
	unsigned short cksum;
} eep_stored_prm;

int eeprom_write_cal_prm(struct de2ts_cal_params *prm)
{
	int eep_fd, count;
	unsigned short cks;

	eep_stored_prm.magic = EEPROM_CAL_MAGIC;
	eep_stored_prm.cksum = 0;
	eep_stored_prm.prm = *prm;

	for (count = cks = 0; count < sizeof eep_stored_prm/2; ++count) {
		cks += *(((unsigned short *) &eep_stored_prm) + count);
	}
	eep_stored_prm.cksum = -cks;

	eep_fd = open("/dev/eeprom", O_RDWR);
	if (eep_fd < 0) {
		perror("Error opening eeprom");
		return -1;
	}

	/* calibration data are stored at the end of the eeprom */
	if (lseek(eep_fd, -sizeof eep_stored_prm, SEEK_END) < 0) {
		perror("Error seeking eeprom");
		close(eep_fd);
		return -1;
	}

	count = write(eep_fd, &eep_stored_prm, sizeof eep_stored_prm);
	if (count != sizeof eep_stored_prm) {
		perror("Error writing eeprom");
		close(eep_fd);
		return -1;
	}

	close(eep_fd);
	return 0;
}

#endif

void next_up_event(int fd, struct de2ts_event *ev)
{
	while (read(fd, ev, sizeof(struct de2ts_event)) > 0) {
		if (ev->event == EV_PEN_UP)
			return;
	}
}

void calibration(int fd, struct de2ts_cal_params *prm)
{
	int x0, x1, y0, y1;
	struct de2ts_event ev;

	printf("Calibration...\ntouch the screen at the arrows\n");

	/* clear the screen */
	memset(screen_ptr, -1, screen_height * screen_width / 8);

	while (1) {

		/* set the parameter to full range */
		prm->version = 0;
		if (ioctl(fd, DE2TS_CAL_PARAMS_SET, prm) < 0) {
			perror("Unable to set touchscreen information");
			exit(1);
		}

		draw_line(0, screen_height - 1, 20, screen_height - 1, 0);
		draw_line(0, screen_height - 1, 0, screen_height - 21, 0);
		draw_line(0, screen_height - 1, 20, screen_height - 21, 0);

		next_up_event(fd, &ev);
		x0 = ev.x;
		y0 = ev.y;

		draw_line(0, screen_height - 1, 20, screen_height - 1, 1);
		draw_line(0, screen_height - 1, 0, screen_height - 21, 1);
		draw_line(0, screen_height - 1, 20, screen_height - 21, 1);

		draw_line(screen_width - 1, 0, screen_width - 21, 0, 0);
		draw_line(screen_width - 1, 0, screen_width - 21, 20, 0);
		draw_line(screen_width - 1, 0, screen_width - 1, 20, 0);

		next_up_event(fd, &ev);
		x1 = ev.x;
		y1 = ev.y;

		draw_line(screen_width - 1, 0, screen_width - 21, 0, 1);
		draw_line(screen_width - 1, 0, screen_width - 21, 20, 1);
		draw_line(screen_width - 1, 0, screen_width - 1, 20, 1);

		printf("x: %d - %d\n", x0, x1);
		printf("y: %d - %d\n", y0, y1);

		/* compute the new parameters */
		prm->version = DE2TS_VERSION;
		prm->xoff = x0;
		prm->xden = (x1 - x0);
		prm->yoff = y1;
		prm->yden = (y0 - y1);
		prm->xrng = screen_width;
		prm->yrng = screen_height;

		printf("touchscreen new parameters:\n");
		print_prm(prm);

		if (ioctl(fd, DE2TS_CAL_PARAMS_SET, prm) < 0) {
			perror("Unable to set touchscreen information");
			exit(1);
		}

		/* check calibration */
		x0 = screen_width * 2 / 3;
		y0 = screen_height * 2 / 3;

		draw_line(x0, y0, x0 + 8, y0, 0);
		draw_line(x0 + 8, y0, x0 + 8, y0 + 8, 0);
		draw_line(x0 + 8, y0 + 8, x0, y0 + 8, 0);
		draw_line(x0, y0 + 8, x0, y0, 0);

		next_up_event(fd, &ev);

		draw_line(x0, y0, x0 + 8, y0, 1);
		draw_line(x0 + 8, y0, x0 + 8, y0 + 8, 1);
		draw_line(x0 + 8, y0 + 8, x0, y0 + 8, 1);
		draw_line(x0, y0 + 8, x0, y0, 1);

		if (ev.x >= x0 && ev.x <= x0 + 8) {
			if (ev.y >= y0 && ev.y <= y0 + 8) {

				x0 = screen_width * 2 / 3;
				y0 = screen_height * 1 / 3;

				draw_line(x0, y0, x0 + 8, y0, 0);
				draw_line(x0 + 8, y0, x0 + 8, y0 + 8, 0);
				draw_line(x0 + 8, y0 + 8, x0, y0 + 8, 0);
				draw_line(x0, y0 + 8, x0, y0, 0);

				next_up_event(fd, &ev);

				draw_line(x0, y0, x0 + 8, y0, 1);
				draw_line(x0 + 8, y0, x0 + 8, y0 + 8, 1);
				draw_line(x0 + 8, y0 + 8, x0, y0 + 8, 1);
				draw_line(x0, y0 + 8, x0, y0, 1);

				if (ev.x >= x0 && ev.x <= x0 + 8) {
					if (ev.y >= y0 && ev.y <= y0 + 8) {
						break;
					}
				}
			}
		}
	}
}

void init_screen(void)
{
	struct fb_fix_screeninfo fscreeninfo;
	struct fb_var_screeninfo screeninfo;
	int fd;

	if ((fd = open("/dev/fb0", O_RDWR)) < 0) {
		perror("Unable to open framebuffer device");
		exit(1);
	}

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fscreeninfo) < 0) {
		perror("Unable to retrieve framebuffer information");
		exit(1);
	}

	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo) < 0) {
		perror("Unable to retrieve framebuffer information");
		exit(1);
	}

	fprintf(stderr, "scren: %dx%dx%d\n", screeninfo.xres_virtual,
			screeninfo.yres_virtual, screeninfo.bits_per_pixel);
	screen_width = screeninfo.xres_virtual;
	screen_height = screeninfo.yres_virtual;

	if ((screen_ptr =
		 mmap(0, screen_height * screen_width / 8, PROT_READ | PROT_WRITE,
			  0, fd, 0)) == MAP_FAILED) {
		perror("Unable to mmap framebuffer");
		exit(1);
	}
}

struct de2ts_event ev_prev;
	
int main(int argc, char *argv[])
{
	int fd;
	struct de2ts_event ev;
	struct de2ts_cal_params prm;

	init_screen();

	if ((fd = open("/dev/ts", O_RDONLY)) < 0) {
		perror("Unable to open touchscreen device");
		exit(1);
	}

#if FIXED_CALIBRATION

	prm.version = DE2TS_VERSION;
	prm.xoff = 500;
	prm.xden = 2840;
	prm.yoff = 3484;
	prm.yden = -3129;
	prm.xrng = 320;
	prm.yrng = 240;

	if (ioctl(fd, DE2TS_CAL_PARAMS_SET, &prm) < 0) {
		perror("Unable to set touchscreen information");
		exit(1);
	}
#else

	calibration(fd, &prm);

#endif

#if EEPROM_STORED_CALDATA

	/* now store the data to the eeprom */
	eeprom_write_cal_prm(&prm);

#endif

	while (read(fd, &ev, sizeof ev) > 0) {
//		char *evst;

		switch (ev.event) {
		case EV_PEN_DOWN:
//			evst = "DOWN";
			break;
		case EV_PEN_MOVE:
//			evst = "MOVE";
			draw_line(ev_prev.x, ev_prev.y, ev.x, ev.y, 0);
			break;
		case EV_PEN_UP:
//			evst = "UP  ";
			if (ev.x > 300 && ev.y > 220) {
				/* clear the screen */
				memset(screen_ptr, -1, screen_height * screen_width / 8);
			} else {
				draw_line(ev_prev.x, ev_prev.y, ev.x, ev.y, 0);
			}
			break;
		default:
//			evst = "????";
			printf("bad event code!\n");
			break;
		}

//		printf("%s: %d %d\n", evst, ev.x, ev.y);

		ev_prev = ev;
	}

	return 0;
}
