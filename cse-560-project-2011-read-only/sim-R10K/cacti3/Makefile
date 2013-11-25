#LIBS = -lm
#FLAGS = 

OEXT=o
LEXT=a

SRCS = time.c area.c io.c cacti_misc.c
HDR = def.h cacti_params.h cacti_misc.h
OBJS = time.$(OEXT) area.$(OEXT) io.$(OEXT) cacti_misc.$(OEXT)

all: libcacti.$(LEXT)
	@echo "my work is done here..."

libcacti.$(LEXT):	$(OBJS) $(HDR)
	$(RM) libcacti.$(LEXT)
	$(AR) $(AROPT)libcacti.$(LEXT) $(OBJS)
	$(RANLIB) libcacti.$(LEXT)

cacti: main.o libcacti.$(LEXT) $(OBJS) $(HDR)
	$(CC) -o cacti main.o libcacti.$(LEXT) -lm

.c.$(OEXT): $(HDR)
	$(CC) -g -Wall $(CFLAGS) -c $*.c

clean:
	  rm -f *.$(OEXT) libcacti.$(LEXT) *~ core cacti 


$(TEMPDIR)time.$(OEXT): def.h cacti_misc.h cacti_params.h
$(TEMPDIR)area.$(OEXT): def.h cacti_misc.h cacti_params.h
$(TEMPDIR)io.$(OEXT): def.h cacti_params.h
$(TEMPDIR)main.$(OEXT): def.h cacti_params.h
