PROGS = wc mmwc pwd ls cat

all: $(PROGS)

clean:
	rm -f $(PROGS)

.PHONY: all clean
