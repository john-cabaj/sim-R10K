#
# Makefile - simulator suite make file
#
# This file is a part of the SimpleScalar tool suite written by
# Todd M. Austin as a part of the Multiscalar Research Project.
#  
# The tool suite is currently maintained by Doug Burger and Todd M. Austin.
# 
# Copyright (C) 1994, 1995, 1996, 1997, 1998 by Todd M. Austin
#
# This source file is distributed "as is" in the hope that it will be
# useful.  It is distributed with no warranty, and no author or
# distributor accepts any responsibility for the consequences of its
# use. 
#
# Everyone is granted permission to copy, modify and redistribute
# this source file under the following conditions:
#
#    This tool set is distributed for non-commercial use only. 
#    Please contact the maintainer for restrictions applying to 
#    commercial use of these tools.
#
#    Permission is granted to anyone to make or distribute copies
#    of this source code, either as received or modified, in any
#    medium, provided that all copyright notices, permission and
#    nonwarranty notices are preserved, and that the distributor
#    grants the recipient permission for further redistribution as
#    permitted by this document.
#
#    Permission is granted to distribute this file in compiled
#    or executable form under the same conditions that apply for
#    source code, provided that either:
#
#    A. it is accompanied by the corresponding machine-readable
#       source code,
#    B. it is accompanied by a written offer, with no time limit,
#       to give anyone a machine-readable copy of the corresponding
#       source code in return for reimbursement of the cost of
#       distribution.  This written offer must permit verbatim
#       duplication by anyone, or
#    C. it is distributed by someone who received only the
#       executable form, and is accompanied by a copy of the
#       written offer of source code that they received concurrently.
#
# In other words, you are welcome to use, share and improve this
# source file.  You are forbidden to forbid anyone else to use, share
# and improve what you give them.
#
# INTERNET: dburger@cs.wisc.edu
# US Mail:  1210 W. Dayton Street, Madison, WI 53706
#
# $Id: Makefile,v 1.9 1998/09/03 22:26:58 taustin Exp taustin $
#
# $Log: Makefile,v $
# Revision 1.9  1998/09/03 22:26:58  taustin
# Alpha OSF1 host support added
# PA-RISC HPUX host support added
# SPARC SunOS host support added
# RS/6000 AIX Unix host support added
# now cleans both tests-alpha and tests-pisa, independent of config
#
# Revision 1.8  1998/08/31 17:05:49  taustin
# added support for MS VC++ NMAKE builds (works with v5.0 SR3 or later)
#
# Revision 1.7  1998/08/27 07:40:03  taustin
# reorganized Makefile: it now works with MS VC++ NMAKE, and many host
#    configurations are supplied in the header; added target configuration
#    support; converted "sim-tests" target to use "-redir:sim" and
#    "-redir:prog" options, this eliminates the need for the "redir"
#    scripts
#
# Revision 1.6  1997/04/16  22:08:40  taustin
# added standalone loader support
#
# Revision 1.5  1997/03/11  01:04:13  taustin
# updated copyright
# CC target now supported
# RANLIB target now supported
# MAKE target now supported
# CFLAGS reorganized
# MFLAGS and MLIBS to improve portability
#
# Revision 1.1  1996/12/05  18:56:09  taustin
# Initial revision
#
# 

##################################################################
#
# Modify the following definitions to suit your build environment,
# NOTE: most platforms should not require any changes
#
##################################################################

#
# Define below C compiler and flags, machine-specific flags and libraries,
# build tools and file extensions, these are specific to a host environment,
# pre-tested environments follow...
#

##
## vanilla Unix, GCC build
##
## NOTE: the SimpleScalar simulators must be compiled with an ANSI C
## compatible compiler.
##
## tested hosts:
##
##	Slackware Linux version 2.0.33, GNU GCC version 2.7.2.2
##	FreeBSD version 3.0-current, GNU egcs version 2.91.50
##	Alpha OSF1 version 4.0, GNU GCC version 2.7.2
##	PA-RISC HPUX version B.10.01, GNU GCC version 2.7-96q3
##	SPARC SunOS version 5.5.1, GNU egcs-2.90.29
##	RS/6000 AIX Unix version 4, GNU GCC version cygnus-2.7-96q4
##	Windows NT version 4.0, Cygnus CygWin/32 beta 19
##
#CC = gcc
#OFLAGS = -O0 -g -Wall
#MFLAGS = `./sysprobe -flags`
#MLIBS  = `./sysprobe -libs` -lm
#ENDIAN = `./sysprobe -s`
#MAKE = make
#AR = ar qcv
#AROPT =
#RANLIB = ranlib
#RM = rm -f
#RMDIR = rm -f
#LN = ln -s
#LNDIR = ln -s
#DIFF = diff
#OEXT = o
#LEXT = a
#EEXT =
#CS = ;
#X=/

##
## Solaris 2.6, GNU GCC version 2.7.2.3
##
CC = /s/gcc-2.7.2.3/bin/gcc
CC = /usr/local/bin/gcc
CC = /usr/bin/gcc
OFLAGS = -g -Wall -finline-functions  
OFLAGS = -g -fno-inline -m32
GCC_LINK_FLAGS = -Wl,--wrap,__ctype_b
DEXT = .debug
DEXT = 
MFLAGS = `./sysprobe -flags`
ZLIB = /filespace/people/c/cabaj/sim-R10K-lib/libz.so.1.2.8
EXOLIB = libexo.a
MLIBS  = `./sysprobe -libs` -lm 
MLIBS  = -lm 
ENDIAN = `./sysprobe -s`
MAKE = make
AR = ar qcv
AROPT =
RANLIB = ranlib
RM = rm -f
RMDIR = rm -f
LN = ln -s
LNDIR = ln -s
DIFF = diff
OEXT = o
LEXT = a
AEXT = alpha
CS = ;
X=/

##
## Alpha OSF1 version 4.0, DEC C compiler version V5.2-036
##
#CC = cc -std
#OFLAGS = -O0 -g -w
#MFLAGS = `./sysprobe -flags`
#MLIBS  = `./sysprobe -libs` -lm
#ENDIAN = `./sysprobe -s`
#MAKE = make
#AR = ar qcv
#AROPT =
#RANLIB = ranlib
#RM = rm -f
#RMDIR = rm -f
#LN = ln -s
#LNDIR = ln -s
#DIFF = diff
#OEXT = o
#LEXT = a
#EEXT =
#CS = ;
#X=/

##
## PA-RISC HPUX version B.10.01, c89 HP C compiler version A.10.31.02
##
#CC = c89 +e -D__CC_C89
#OFLAGS = -g
#MFLAGS = `./sysprobe -flags`
#MLIBS  = `./sysprobe -libs` -lmq
#ENDIAN = `./sysprobe -s`
#MAKE = make
#AR = ar qcv
#AROPT =
#RANLIB = ranlib
#RM = rm -f
#RMDIR = rm -f
#LN = ln -s
#LNDIR = ln -s
#DIFF = diff
#OEXT = o
#LEXT = a
#EEXT =
#CS = ;
#X=/

##
## SPARC SunOS version 5.5.1, Sun WorkShop C Compiler (acc) version 4.2
##
#CC = /opt/SUNWspro/SC4.2/bin/acc
#OFLAGS = -O0 -g
#MFLAGS = `./sysprobe -flags`
#MLIBS  = `./sysprobe -libs` -lm
#ENDIAN = `./sysprobe -s`
#MAKE = make
#AR = ar qcv
#AROPT =
#RANLIB = ranlib
#RM = rm -f
#RMDIR = rm -f
#LN = ln -s
#LNDIR = ln -s
#DIFF = diff
#OEXT = o
#LEXT = a
#EEXT =
#CS = ;
#X=/

##
## RS/6000 AIX Unix version 4, xlc compiler build
##
#CC = xlc -D__CC_XLC
#OFLAGS = -g
#MFLAGS = `./sysprobe -flags`
#MLIBS  = `./sysprobe -libs` -lm
#ENDIAN = `./sysprobe -s`
#MAKE = make
#AR = ar qcv
#AROPT =
#RANLIB = ranlib
#RM = rm -f
#RMDIR = rm -f
#LN = ln -s
#LNDIR = ln -s
#DIFF = diff
#OEXT = o
#LEXT = a
#EEXT =
#CS = ;
#X=/

##
## WinNT, MS VC++ build
##
## NOTE: requires MS VC++ version 5.0 + service pack 3 or later
## NOTE1: before configuring the simulator, delete the symbolic link "tests/"
##
#CC = cl /Za /nologo
#OFLAGS = /W3 /Zi
#MFLAGS = -DBYTES_LITTLE_ENDIAN -DWORDS_LITTLE_ENDIAN -DFAST_SRL -DFAST_SRA
#MLIBS  =
#ENDIAN = little
#MAKE = nmake /nologo
#AR = lib
#AROPT = -out:
#RANLIB = dir
#RM = del/f/q
#RMDIR = del/s/f/q
#LN = copy
#LNDIR = xcopy/s/e/i
#DIFF = dir
#OEXT = obj
#LEXT = lib
#EEXT = .exe
#CS = &&
#X=\\\\

#
# Compilation-specific feature flags
#
# -DDEBUG	- turns on debugging features
# -DBFD_LOADER	- use libbfd.a to load programs (also required BINUTILS_INC
#		  and BINUTILS_LIB to be defined, see below)
# -DGZIP_PATH	- specifies path to GZIP executable, only needed if SYSPROBE
#		  cannot locate binary
# -DSLOW_SHIFTS	- emulate all shift operations, only used for testing as
#		  sysprobe will auto-detect if host can use fast shifts
#
FFLAGS = -DDEBUG

#
# Point the Makefile to your Simplescalar-based bunutils, these definitions
# should indicate where the include and library directories reside.
# NOTE: these definitions are only required if BFD_LOADER is defined.
#
#BINUTILS_INC = -I../include
#BINUTILS_LIB = -L../lib

#
#


##################################################################
#
# YOU SHOULD NOT NEED TO MODIFY ANYTHING BELOW THIS COMMENT
#
##################################################################

#
# complete flags
#
CFLAGS = $(MFLAGS) $(FFLAGS) $(OFLAGS) $(BINUTILS_INC) $(BINUTILS_LIB)

#
# all the sources
#
SRCS =	main.c sim-func.c sim-cache.c sim-R10K.c sim-R10K-reg.c sim-R10K-power.c sim-DLX.c \
	memory.c cache.c bpred.c adisambig.c \
	resource.c endian.c options.c predec.c \
	eio.c stats.c endian.c misc.c \
	loader.c syscall.c ctype-fix.c

HDRS =	syscall.h memory.h sim.h loader.h cache.h bpred.h \
	resource.h adisambig.h endian.h \
	eio.h version.h endian.h misc.h stats.h \
	machine.h machine.def ecoff.h

#
# common objects
#
FUNC_OBJS = main.$(OEXT) syscall.$(OEXT) memory.$(OEXT) \
	loader.$(OEXT) endian.$(OEXT) stats.$(OEXT) \
	options.$(OEXT) eio.$(OEXT) predec.$(OEXT)  \
	fastfwd.$(OEXT) misc.$(OEXT) machine.$(OEXT) ctype-fix.$(OEXT)

CB_OBJS = cache.$(OEXT) bpred.$(OEXT) resource.$(OEXT)

TIMING_OBJS = adisambig.$(OEXT)

#
# uncommon objects
#

#
# programs to build
#
PROGS = sim-func sim-func.condor sim-cache sim-cache.condor sim-cache-power sim-cache-power.condor sim-DLX sim-DLX.condor sim-R10K sim-R10K.condor sim-R10K-reg sim-R10K-power sim-R10K-power.condor
# strip out the condor targets for now...
PROGS = sim-func sim-cache sim-cache-power sim-DLX sim-R10K sim-R10K-reg sim-R10K-power

#
# all targets, NOTE: library ordering is important...
#
all: $(PROGS)
	@echo "my work is done here..."

sysprobe:	sysprobe.c
	$(CC) $(FFLAGS) -o sysprobe sysprobe.c
	@echo endian probe results: $(ENDIAN)
	@echo probe flags: $(MFLAGS)
	@echo probe libs: $(MLIBS)

sim-func:	sysprobe sim-func.$(OEXT) $(FUNC_OBJS) $(EXOLIB)
	$(CC) -o sim-func $(CFLAGS) $(GCC_LINK_FLAGS) sim-func.$(OEXT) $(FUNC_OBJS) $(EXOLIB) $(ZLIB) $(MLIBS)

sim-func.condor:	sysprobe sim-func.$(OEXT) $(FUNC_OBJS) $(EXOLIB)
	condor_compile $(CC) -o sim-func.condor $(GCC_LINK_FLAGS) $(CFLAGS) sim-func.$(OEXT) $(FUNC_OBJS) $(EXOLIB) $(ZLIB) $(MLIBS)

sim-cache:	sysprobe sim-cache.$(OEXT) $(FUNC_OBJS) cache.$(OEXT) $(EXOLIB)
	$(CC) -o sim-cache $(GCC_LINK_FLAGS) $(CFLAGS) sim-cache.$(OEXT) cache.$(OEXT) $(FUNC_OBJS) $(EXOLIB) $(ZLIB) $(MLIBS)

sim-cache.condor:	sysprobe sim-cache.$(OEXT) $(FUNC_OBJS) cache.$(OEXT) $(EXOLIB)
	condor_compile $(CC) -o sim-cache.condor $(GCC_LINK_FLAGS) $(CFLAGS) sim-cache.$(OEXT) cache.$(OEXT) $(FUNC_OBJS) $(EXOLIB) $(ZLIB) $(MLIBS)

sim-cache-power:	sysprobe sim-cache-power.$(OEXT) $(FUNC_OBJS) cache.$(OEXT) power.$(OEXT) cacti3/libcacti.$(LEXT) $(EXOLIB)
	$(CC) -o sim-cache-power $(GCC_LINK_FLAGS) $(CFLAGS) sim-cache-power.$(OEXT) cache.$(OEXT) power.$(OEXT) $(FUNC_OBJS) cacti3/libcacti.$(LEXT) $(EXOLIB) $(ZLIB) $(MLIBS)

sim-cache-power.condor:	sysprobe sim-cache-power.$(OEXT) $(FUNC_OBJS) cache.$(OEXT) power.$(OEXT) $(EXOLIB) cacti3/libcacti.$(LEXT)
	condor_compile $(CC) -o sim-cache-power.condor $(GCC_LINK_FLAGS) $(CFLAGS) sim-cache-power.$(OEXT) cache.$(OEXT) power.$(OEXT) $(FUNC_OBJS) cacti3/libcacti.$(LEXT) $(EXOLIB) $(ZLIB) $(MLIBS)


sim-DLX:	sysprobe sim-DLX.$(OEXT) $(FUNC_OBJS) $(CB_OBJS) $(EXOLIB)
	$(CC) -o sim-DLX $(GCC_LINK_FLAGS) $(CFLAGS) sim-DLX.$(OEXT) $(CB_OBJS) $(FUNC_OBJS) $(EXOLIB) $(ZLIB) $(MLIBS)

sim-DLX.condor:	sysprobe sim-DLX.$(OEXT) $(FUNC_OBJS) $(CB_OBJS) $(EXOLIB)
	condor_compile $(CC) -o sim-DLX.condor $(GCC_LINK_FLAGS) $(CFLAGS) sim-DLX.$(OEXT) $(CB_OBJS) $(FUNC_OBJS) $(EXOLIB) $(ZLIB) $(MLIBS)

sim-R10K:	sysprobe sim-R10K.$(OEXT) $(FUNC_OBJS) cache.$(OEXT) bpred.$(OEXT) resource.$(OEXT) adisambig.$(OEXT) $(EXOLIB)
	$(CC) -o sim-R10K $(GCC_LINK_FLAGS) $(CFLAGS) sim-R10K.$(OEXT) $(FUNC_OBJS) cache.$(OEXT) bpred.$(OEXT) resource.$(OEXT) adisambig.$(OEXT) $(EXOLIB) $(ZLIB) $(MLIBS)

sim-R10K.condor:	sysprobe sim-R10K.$(OEXT) $(FUNC_OBJS) cache.$(OEXT) bpred.$(OEXT) resource.$(OEXT) adisambig.$(OEXT) $(EXOLIB)
	condor_compile $(CC) -o sim-R10K.condor $(GCC_LINK_FLAGS) $(CFLAGS) sim-R10K.$(OEXT) $(FUNC_OBJS) cache.$(OEXT) bpred.$(OEXT) resource.$(OEXT) adisambig.$(OEXT) $(EXOLIB) $(ZLIB) $(MLIBS)

sim-R10K-reg:	sysprobe sim-R10K-reg.$(OEXT) $(FUNC_OBJS) cache.$(OEXT) bpred.$(OEXT) resource.$(OEXT) adisambig.$(OEXT) $(EXOLIB)
	$(CC) -o sim-R10K-reg $(GCC_LINK_FLAGS) $(CFLAGS) sim-R10K-reg.$(OEXT) $(FUNC_OBJS) cache.$(OEXT) bpred.$(OEXT) resource.$(OEXT) adisambig.$(OEXT) $(EXOLIB) $(ZLIB) $(MLIBS)

sim-R10K-power:	sysprobe sim-R10K-power.$(OEXT) $(FUNC_OBJS) cache.$(OEXT) bpred.$(OEXT) resource.$(OEXT) adisambig.$(OEXT) power.$(OEXT) cacti3/libcacti.$(LEXT) $(EXOLIB)
	$(CC) -o sim-R10K-power $(GCC_LINK_FLAGS) $(CFLAGS) sim-R10K-power.$(OEXT) $(FUNC_OBJS) cache.$(OEXT) bpred.$(OEXT) resource.$(OEXT) adisambig.$(OEXT) power.$(OEXT) cacti3/libcacti.$(LEXT) $(EXOLIB) $(ZLIB) $(MLIBS)

sim-R10K-power.condor:	sysprobe sim-R10K-power.$(OEXT) $(FUNC_OBJS) cache.$(OEXT) bpred.$(OEXT) resource.$(OEXT) adisambig.$(OEXT) power.$(OEXT) cacti3/libcacti.$(LEXT) $(EXOLIB)
	condor_compile $(CC) -o sim-R10K-power.condor $(GCC_LINK_FLAGS) $(CFLAGS) sim-R10K-power.$(OEXT) $(FUNC_OBJS) cache.$(OEXT) bpred.$(OEXT) resource.$(OEXT) adisambig.$(OEXT) power.$(OEXT) cacti3/libcacti.$(LEXT) $(EXOLIB) $(ZLIB) $(MLIBS)

cacti3/libcacti.$(LEXT): cacti3/area.c cacti3/io.c cacti3/main.c cacti3/time.c\
                         cacti3/areadef.h cacti3/cacti_params.h cacti3/def.h
	cd cacti3 $(CS) \
	$(MAKE) "MAKE=$(MAKE)" "CC=$(CC)" "AR=$(AR)" "AROPT=$(AROPT)" "RANLIB=$(RANLIB)" "CFLAGS=$(MFLAGS) $(FFLAGS) $(OFLAGS)" "OEXT=$(OEXT)" "LEXT=$(LEXT)" "EEXT=$(EEXT)" "X=$(X)" "RM=$(RM)" libcacti.$(LEXT)

cacti3/cacti: cacti3/libcacti.$(LEXT) cacti3/main.c
	cd cacti3 $(CS) \
	$(MAKE) "MAKE=$(MAKE)" "CC=$(CC)" "AR=$(AR)" "AROPT=$(AROPT)" "RANLIB=$(RANLIB)" "CFLAGS=$(MFLAGS) $(FFLAGS) $(OFLAGS)" "OEXT=$(OEXT)" "LEXT=$(LEXT)" "EEXT=$(EEXT)" "X=$(X)" "RM=$(RM)" cacti

.c.$(OEXT):
	$(CC) $(CFLAGS) -c $*.c

filelist:
	@echo $(SRCS) $(HDRS) Makefile

diffs:
	-rcsdiff RCS/*

clean:
	-$(RM) *.o *.obj core *~ Makefile.bak sysprobe $(PROGS)

unpure:
	rm -f sim.pure *pure*.o sim.pure.pure_hardlink sim.pure.pure_linkinfo

depend:
	makedepend.local -n -x $(BINUTILS_INC) $(SRCS)


# DO NOT DELETE THIS LINE -- make depend depends on it.

sim-func.$(OEXT): host.h misc.h machine.h machine.def memory.h
sim-func.$(OEXT): options.h stats.h loader.h syscall.h sim.h
sim-cache.$(OEXT): host.h misc.h machine.h machine.def memory.h
sim-cache.$(OEXT): options.h stats.h loader.h syscall.h sim.h cache.h
sim-DLX.$(OEXT): host.h misc.h machine.h machine.def memory.h
sim-DLX.$(OEXT): options.h stats.h loader.h syscall.h sim.h cache.h bpred.h
sim-R10K.$(OEXT): host.h misc.h machine.h machine.def memory.h
sim-R10K.$(OEXT): options.h stats.h cache.h loader.h syscall.h
sim-R10K.$(OEXT): bpred.h resource.h sim.h
sim-R10K-reg.$(OEXT): host.h misc.h machine.h machine.def memory.h
sim-R10K-reg.$(OEXT): options.h stats.h cache.h loader.h syscall.h
sim-R10K-reg.$(OEXT): bpred.h resource.h sim.h

sim-cache-power.$(OEXT): host.h misc.h machine.h machine.def memory.h
sim-cache-power.$(OEXT): options.h stats.h loader.h syscall.h sim.h cache.h power.h 
sim-cache-power.$(OEXT): cacti3/cacti_params.h cacti3/cacti_misc.h
sim-R10K-power.$(OEXT): host.h misc.h machine.h machine.def memory.h
sim-R10K-power.$(OEXT): options.h stats.h cache.h loader.h syscall.h
sim-R10K-power.$(OEXT): bpred.h resource.h sim.h power.h 
sim-R10K-power.$(OEXT): cacti3/cacti_misc.h cacti3/cacti_params.h

power.$(OEXT): machine.h options.h power.h cacti3/cacti_params.h

main.$(OEXT): host.h misc.h machine.h machine.def endian.h version.h 
main.$(OEXT): memory.h options.h stats.h loader.h sim.h
memory.$(OEXT): host.h misc.h machine.h machine.def options.h stats.h 
memory.$(OEXT): memory.h
cache.$(OEXT): host.h misc.h machine.h machine.def cache.h memory.h options.h
cache.$(OEXT): stats.h 
bpred.$(OEXT): host.h misc.h machine.h machine.def bpred.h stats.h 
bpred.$(OEXT): options.h
resource.$(OEXT): host.h misc.h resource.h
endian.$(OEXT): endian.h loader.h host.h misc.h machine.h machine.def 
endian.$(OEXT): memory.h options.h stats.h
options.$(OEXT): host.h misc.h options.h
eio.$(OEXT): host.h misc.h machine.h machine.def memory.h options.h
eio.$(OEXT): stats.h loader.h libexo.h host.h misc.h machine.h
eio.$(OEXT): syscall.h sim.h endian.h eio.h
endian.$(OEXT): endian.h loader.h host.h misc.h machine.h machine.def 
endian.$(OEXT): memory.h options.h stats.h 
misc.$(OEXT): host.h misc.h machine.h machine.def
loader.$(OEXT): host.h misc.h machine.h machine.def endian.h memory.h
loader.$(OEXT): options.h stats.h sim.h eio.h loader.h
loader.$(OEXT): ecoff.h
syscall.$(OEXT): host.h misc.h machine.h machine.def memory.h
syscall.$(OEXT): options.h stats.h loader.h sim.h endian.h eio.h
syscall.$(OEXT): syscall.h
machine.$(OEXT): host.h machine.h misc.h machine.def 
predec.$(OEXT): host.h machine.h misc.h machine.def memory.h loader.h predec.h
fastfwd.$(OEXT): host.h machine.h misc.h machine.def memory.h loader.h predec.h fastfwd.h

