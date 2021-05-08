CFLAGS += -std=gnu11 -g
EXES    = vuln

all:  $(EXES)
clean:
	-rm -f $(EXES)

vuln: vuln.c
# don't treat all and clean as file targets
.PHONY: all clean