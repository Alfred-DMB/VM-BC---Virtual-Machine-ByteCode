CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS =
LIBS = -lreadline

SRCDIR = src
OBJDIR = obj

SRCS = $(SRCDIR)/main.c $(SRCDIR)/vm.c
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET = vm

.PHONY: all clean

all: $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)
