
CC		= clang
CFLAGS	= -std=c23 -Wall
CLIBS   = -lreadline -lncurses -lxxhash

SRCDIR = src

REPL_BINARY = vinyl-repl


$(REPL_BINARY): src/*.c
	$(CC) $(CFLAGS) $(CLIBS) $^ -o $@
