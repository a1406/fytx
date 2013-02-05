.PHONY : all clean 

LIBPATH = -L../../boost_1_47_0/stage/lib  -L../util_lib  -L../net_lib
BOOST_LIB = -lboost_system -lboost_thread -lboost_filesystem -lboost_date_time

CFLAGS = -g -pg -O0 -I../net_lib/Include -I../util_lib -I../../boost_1_47_0/ -I../game_def -I/usr/include/mongo
LDFLAGS = $(LIBPATH) $(BOOST_LIB) -lmysqlclient  -ldl -lm 
LDFLAGS += -lrt -Wl,-E  -pg 
SHARED = -fPIC --shared


#SRC_DIR = $(shell find . -type d)
#SRCS = $(foreach TMP_SRC_DIRS, $(SRC_DIR), $(wildcard $(TMP_SRC_DIRS)/*.cpp))
#OBJS = $(SRCS:.cpp = .o)

OBJS = $(shell find . -type f -name "*.cpp" | sed 's/^\.\///g' | sed  's/.cpp/.o/g')

%.o : %.cpp
	g++ -c -o $@  $< $(CFLAGS)

