PROGS = wc mmwc pwd ls cat sh
CFLAGS = -g

all: $(PROGS)

clean:
	rm -f $(PROGS)

.PHONY: all clean
