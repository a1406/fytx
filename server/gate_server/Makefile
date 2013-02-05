include ../common.mk

TARGET = gate_server
#LDFLAGS += -L../bin  -lldperfect_util  -lldperfect_net 

$(TARGET): $(OBJS)
	g++  -o $@ $^   -lldperfect_util  -lldperfect_net  $(LDFLAGS)

clean:
	find . -type f -name "*.o" | xargs rm -f
	rm -f  $(TARGET)

