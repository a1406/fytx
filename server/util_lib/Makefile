include ../common.mk

TARGET = libldperfect_util.so
CFLAGS += $(SHARED)

$(TARGET): $(OBJS)
	g++  $(SHARED) -o $@ $^ 

clean:
	find . -type f -name "*.o" | xargs rm -f
	rm -f  $(TARGET)

