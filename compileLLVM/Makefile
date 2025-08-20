TARGET = a.out
# ASMS = test.s
OBJS = test.s.o stdkano.c.o

CC = clang
LLC = llc
LLFLAGS = --relocation-model=pic
CPPFLAGS += -Wall -g

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -rf *.o *.s


$(TARGET): $(OBJS)
	$(CC) $(CPPFLAGS) -o $@ $(OBJS)

%.c.o: %.c
	$(CC) -c $< -o $@

%.s.o: %.s
	$(CC) -c $< -o $@

%.s: %.ll
	$(LLC) $(LLFLAGS) $< -o $@
