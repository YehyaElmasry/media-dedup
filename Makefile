SRCDIR=./src
INCDIR=./include
OBJDIR=./obj

CC=g++
CFLAGS=-std=c++17 -Wall -I$(INCDIR)

LIBS=-lm

_DEPS =
DEPS = $(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJ = main.o deduplicator.o hasher.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

dedup: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS) | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR):
	@echo "Creating folder $(OBJDIR)"
	mkdir -p $@

.PHONY: clean

clean:
	rm -f dedup
	rm -rf $(OBJDIR)