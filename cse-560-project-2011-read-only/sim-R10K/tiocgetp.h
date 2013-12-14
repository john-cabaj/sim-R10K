/* This file contains the subset of libbsd required to implement the
 * TIOCGETP BSD-style ioctl.  The file contains parts of the files sgtty.c
 * and sgtty.h included with the package linux86_0.13.0.orig.tar.gz
 * The original file headers are retained.  -- butts@cs.wisc.edu */

/* sgtty.h */

/* This file contains defintions to help make linux termios look like
   bsd sgtty. */

/* Copyright (c) 1992 Ross Biro

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <termios.h>

#define TIOCGETP	0x5481

struct sgttyb
{
    unsigned short sg_flags;
    char sg_ispeed;
    char sg_ospeed;
    char sg_erase;
    char sg_kill;
    struct termios t;
    int check;
};

/* sgtty.c - emulate BSD sgtty stuff with termios - ross biro, rick sladkey */

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

struct mask
{
   unsigned short mask;
   unsigned short res;
};

struct sf 
{
   struct mask iflag;
   struct mask oflag;
   struct mask cflag;
   struct mask lflag;
};

static struct sf trans[] =
{
   /* order is important here. */
   /* iflag oflag cflag lflag */
   /* this needs to be fixed. */
   {{0,0}, {OPOST,0}, {0,0}, {ISIG,0}},			/* O_RAW */
   {{0,0}, {0,0}, {0,0}, {XCASE,XCASE}},		/* O_LCASE */
   {{ICRNL,ICRNL}, {ONLCR, ONLCR}, {0,0}, {0,0}},	/* O_CRMOD */
   {{0,0}, {0,0}, {0,0}, {ECHO,ECHO}},			/* O_ECHO */
   {{0,0}, {0,0}, {PARENB|PARODD,PARENB|PARODD}, {0,0}},/* O_ODDP */
   {{0,0}, {0,0}, {PARENB|PARODD,PARENB}, {0,0}},	/* O_EVENP */
   {{0,0}, {0,0}, {0,0}, {ICANON,0}},			/* O_CBREAK */
};

#define _BSD_VDISABLE	255

int
bsd_ioctl (fd,option,sg)
int fd;
int option;
struct sgttyb *sg;
{
   struct termios t;
   int err;
   int i;

   if(option != TIOCGETP) return EINVAL;
   err = ioctl (fd, TCGETS, &t);
   if (err < 0) return (err);
   sg->sg_ispeed = cfgetispeed (&t);
   sg->sg_ospeed = cfgetospeed (&t);
   sg->sg_erase = _POSIX_VDISABLE ? _BSD_VDISABLE : t.c_cc[VERASE];
   sg->sg_kill = _POSIX_VDISABLE ? _BSD_VDISABLE : t.c_cc[VKILL];
   sg->sg_flags = 0;
   for (i = 0; i < sizeof (trans) / sizeof (struct sf); i++)
     {
	if ((t.c_iflag & trans[i].iflag.mask) == trans[i].iflag.res &&
	    (t.c_oflag & trans[i].oflag.mask) == trans[i].oflag.res &&
	    (t.c_cflag & trans[i].cflag.mask) == trans[i].cflag.res &&
	    (t.c_lflag & trans[i].lflag.mask) == trans[i].lflag.res)
	  {
	     sg->sg_flags |= 1 << i;
	  }
     }
   return (0);
}

