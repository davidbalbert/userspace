all: wc mmwc

wc: wc.c

mmwc: mmwc.c

clean:
	rm -f wc mmwc

.PHONY: all clean
