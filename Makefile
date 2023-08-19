CC=gcc
LD=gcc
LDFLAGS=
CFLAGS=

DEBUG=1

ifeq ($(DEBUG),1)
  CFLAGS += -g
endif

all: bin/responsive

bin/responsive: bin/main.o
	mkdir -p $(@D)
	$(LD) $< -o $@

bin/main.o: src/main.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

run: bin/responsive
	$<

debug:
	$(MAKE) clear
	$(MAKE) DEBUG=1

clean:
	rm -f bin/main.o bin/responsive
	[ -d bin ] && rmdir bin || true
