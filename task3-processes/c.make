YACC ?= bison
LEX = flex
YFLAGS += -d -t -v -b y

SRC = parse.c scan.c main.c execute.c
OBJ = $(SRC:.c=.o)

ifeq ($(shell which $(LEX)),)
$(error Your system does not have the "flex" tool installed. Please install the corresponding package.)
endif
ifeq ($(shell which $(YACC)),)
$(error Your system does not have the "bison" tool installed. Please install the corresponding package.)
endif

YFLAGS += -d -t -v -b y
LIBS = -lfl

all: shell

shell: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o scan.c parse.c y.tab.h y.output shell

.PHONY: clean
