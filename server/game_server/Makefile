include ../common.mk

TARGET = game_server
#LDFLAGS += -L../bin  -lldperfect_util  -lldperfect_net -lmongoclient

$(TARGET): $(OBJS)
	g++  -o $@ $^    -lldperfect_util  -lldperfect_net -lmongoclient $(LDFLAGS)

clean:
	find . -type f -name "*.o" | xargs rm -f
	rm -f  $(TARGET)

