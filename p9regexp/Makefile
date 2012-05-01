RANLIB=true

CC=gcc
CFLAGS+=-Wall -Wno-missing-braces -Wno-parentheses -Wno-switch -fPIC -O2 -g -c -I.
O=o
AR=ar
ARFLAGS=rvc
NAN=nan64.$O

PREFIX=/usr/local

TARG=p9regexp

OFILES=p9regexp.$O

CLEANFILES=p9regexp

all: $(TARG)

$(LIB): $(OFILES)
	$(AR) $(ARFLAGS) $(LIB) $(OFILES)
	$(RANLIB) $(LIB)

.c.$O:
	$(CC) $(CFLAGS) -I$(PREFIX)/include $*.c

%.$O: %.c
	$(CC) $(CFLAGS) -I$(PREFIX)/include $*.c

$(TARG): $(OFILES)
	$(CC) -o $(TARG) $(OFILES) -L$(PREFIX)/lib -lregexp9 -lbio -lfmt -lutf

clean:
	rm -f $(OFILES) $(LIB) $(CLEANFILES)
