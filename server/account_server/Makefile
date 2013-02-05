include ../common.mk

TARGET = share_get share_share2 share_share share_list varify question question_list login register guest bind pay_apple
#LDFLAGS += -L../bin  -lldperfect_util  -lldperfect_net -lmongoclient
CFLAGS += -I"../inc/mysql++-3.1.0/include/" -I"../inc/MySQL Server 5.1/include"


all: $(TARGET)

share_get: share_get.o
	g++  -o $@ $^    -lldperfect_util  -lldperfect_net -lmysqlclient  $(LDFLAGS)

share_share2: share_share2.o
	g++  -o $@ $^    -lldperfect_util  -lldperfect_net -lmysqlclient  $(LDFLAGS) `curl-config  --libs` 

share_share: share_share.o
	g++  -o $@ $^    -lldperfect_util  -lldperfect_net -lmysqlclient  $(LDFLAGS) `curl-config  --libs` 

share_list: share_list.o
	g++  -o $@ $^    -lldperfect_util  -lldperfect_net -lmysqlclient  $(LDFLAGS)

varify: varify.o
	g++  -o $@ $^    -lldperfect_util  -lldperfect_net -lmysqlclient  $(LDFLAGS)

question_list: question_list.o
	g++  -o $@ $^    -lldperfect_util  -lldperfect_net -lmysqlclient  $(LDFLAGS)

question: question.o
	g++  -o $@ $^    -lldperfect_util  -lldperfect_net -lmysqlclient  $(LDFLAGS)

login: login.o
	g++  -o $@ $^    -lldperfect_util  -lldperfect_net -lmysqlclient  $(LDFLAGS) `curl-config  --libs` 

register: register.o
	g++  -o $@ $^    -lldperfect_util  -lldperfect_net -lmysqlclient $(LDFLAGS)

guest: guest.o
	g++  -o $@ $^    -lldperfect_util  -lldperfect_net -lmysqlclient $(LDFLAGS)

bind: bind.o
	g++  -o $@ $^    -lldperfect_util  -lldperfect_net -lmysqlclient $(LDFLAGS)

pay_apple: pay_apple.o
	g++  -o $@ $^    -lldperfect_util  -lldperfect_net -lmysqlclient $(LDFLAGS) `curl-config  --libs` 

clean:
	find . -type f -name "*.o" | xargs rm -f
	rm -f  $(TARGET)

