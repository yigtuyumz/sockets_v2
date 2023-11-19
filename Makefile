ifeq ($(shell uname), FreeBSD)
	CC = gcc13
else
	CC = gcc
endif

LIBDIR = ./lib
LDFLAGS = -L$(LIBDIR) -lutils -Wl,-rpath=$(LIBDIR)
CFLAGS = -std=c99
OUTDIR = ./out

RELEASE ?= no
ifeq ($(RELEASE),yes)
	CFLAGS := -Wall -Wextra -Werror
endif

.PHONY: a r clean s c


a: s c

clean:
	rm -rf $(OUTDIR)

r: clean a

s:
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) server.c -o $(OUTDIR)/s $(LDFLAGS)

c:
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) client.c -o $(OUTDIR)/c $(LDFLAGS)
