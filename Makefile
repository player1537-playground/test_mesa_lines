SHELL := bash

CFLAGS += -D_GNU_SOURCE
CFLAGS += -g

m_CFLAGS :=
m_LDLIBS := -lm

dl_CFLAGS :=
dl_LDLIBS := -ldl -lOSMesa -lGL

INC:=-I/opt/mesa/include -Isrc
LIB:=-L/opt/mesa/lib/x86_64-linux-gnu

.PHONY: all
all: build/render

.PHONY: clean
clean:

build:
	mkdir -p build

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) $(LIB) $(INC) -c -o $@ $<

build/%: build/%.o | build
	$(CC) $(CFLAGS) $(LIB) $(INC) -o $@ $^ $(LDLIBS) -Wl,-rpath=/opt/mesa/lib/x86_64-linux-gnu

build/render: LDLIBS += $(dl_LDLIBS) $(LIB) $(m_LDLIBS)
build/render: build/glad.o
