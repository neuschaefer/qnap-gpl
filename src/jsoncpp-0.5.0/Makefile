CC		= $(CROSS_COMPILE)gcc
CXX		= $(CROSS_COMPILE)g++
AR		= $(CROSS_COMPILE)ar
RANLIB	= $(CROSS_COMPILE)ranlib

INCLUDES = -Iinclude

FLAGS	= \
	-c \
	-w \
	-g \
	-O2 \
	-fPIC \
	-pthread

CC_SRCS = \
	src/lib_json/json_msgpack.cpp \
	src/lib_json/json_reader.cpp \
	src/lib_json/json_value.cpp \
	src/lib_json/json_writer.cpp

.PHONY: all
all: build jsoncpp install

build:
	$(shell mkdir -p build/include/json build/lib > /dev/null 2>&1)

src/%.o: $(@:.o=.cpp)
	$(CXX) $(FLAGS) $(INCLUDES) $(@:.o=.cpp) -o $@

build/lib/libjsoncpp.a: $(CC_SRCS:.cpp=.o)
	$(AR) cru $@ $(CC_SRCS:.cpp=.o)
	$(RANLIB) $@

.PHONY: jsoncpp
jsoncpp: build/lib/libjsoncpp.a

install:
	install -m 0644 include/json/*.h build/include/json

clean:
	$(shell find . -name *.o | xargs rm -f)
	$(shell find . -name *.a | xargs rm -f)
	-@rm -rf build 2>/dev/null || true
