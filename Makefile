CFLAGS = -g -Wall -MMD -std=gnu99 -pthread -fPIC -pie
LDFLAGS=

HOST_CFLAGS = -g -Wall -MMD -std=gnu1x
HOSTCC = gcc
CC     = gcc
CCLD   = gcc
RM     = rm -rf
AR     = ar

MAKEFLAGS+="-R --no-print-directory"

ifndef V
	QUIET_CC   = @ echo '    ' CC $@;
	QUIET_LD   = @ echo '    ' LD $@;
	QUIET_AR   = @ echo '    ' AR $@;
	QUIET_CCLD = @ echo '    ' CCLD $@;
	QUIET_HOSTCC=@ echo '    ' HOSTCC $@;
endif

LIBNAME = libethcan
LIBOBJ  = ethcan.o

all : dynamic static

.PHONY: dynamic static
dynamic : $(LIBNAME).so
static  : $(LIBNAME).a
$(LIBNAME).so : $(LIBOBJ)
$(LIBNAME).a  : $(LIBOBJ)

.SECONDARY:

%.o : %.c
	$(QUIET_CC)$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean :
	$(RM) *.o *.so *.a

%.so  :
	$(QUIET_CCLD)$(CCLD) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^

%.a   :
	$(QUIET_AR)$(AR) rcsD $@ $^

%.elf :
	$(QUIET_LD)$(CCLD) $(CFLAGS) $(LDFLAGS) -o $@ $^

bpgen : bpgen.c
	$(QUIET_HOSTCC)$(HOSTCC) $(HOST_CFLAGS) -o $@ $<

-include $(wildcard *.d)
