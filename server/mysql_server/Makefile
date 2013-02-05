include ../common.mk

TARGET = mysql_server
#LDFLAGS += -L../bin  -lldperfect_util  -lldperfect_net -lmongoclient
CFLAGS += -I"../inc/mysql++-3.1.0/include/" -I"../inc/MySQL Server 5.1/include"


$(TARGET): $(OBJS)
	g++  -o $@ $^   -lldperfect_util  -lldperfect_net -lmysqlpp $(LDFLAGS)

clean:
	find . -type f -name "*.o" | xargs rm -f
	rm -f  $(TARGET)

