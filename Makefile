CC = gcc
# CFLAGS = -Wall -Wextra -Werror -std=gnu11 -march=native -flto -funroll-loops -MMD -Iinclude
CFLAGS = -Wall -Wextra -Werror -std=gnu11 -MMD -Iinclude -g
CFFLAGS = 
LDFLAGS = 

SRCDIR = src
INCLUDEDIR = include
BUILDDIR = build
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SOURCES))

EXECUTABLE = $(BINDIR)/e8086

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(CFFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJECTS:.o=.d)

clean:
	rm -rf $(BUILDDIR) $(BINDIR)

.PHONY: all clean 
