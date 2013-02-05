include ../common.mk

TARGET = login_server
#LDFLAGS += -L../bin  -lldperfect_util  -lldperfect_net -lmongoclient
LDFLAGS += `curl-config  --libs`

$(TARGET): $(OBJS)
	g++  -o $@ $^  -L../bin  -lldperfect_util  -lldperfect_net -lmongoclient $(LDFLAGS)

clean:
	find . -type f -name "*.o" | xargs rm -f
	rm -f  $(TARGET)

