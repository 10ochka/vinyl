
CC		= clang
CFLAGS	= -std=c23 -Wall -Wextra

SRCDIR = src
BUILDDIR = build

REPL_BINARY = $(BUILDDIR)/vinyl-repl


$(REPL_BINARY): $(SRCDIR)/main.c $(SRCDIR)/string.c $(SRCDIR)/common.c
	$(CC) $(CFLAGS) $? -o $@


clean:
	rm $(BUILDDIR)/*
