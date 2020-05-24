CC		= $(CROSS_COMPILE)gcc
CXX		= $(CROSS_COMPILE)g++
AR		= $(CROSS_COMPILE)ar
RANLIB	= $(CROSS_COMPILE)ranlib

INCLUDES = \
	-Isrc \
	-I$(SQLITE3_HEADER)

FLAGS = \
	-c \
	-w \
	-g \
	-O2 \
	-fPIC \
	-pthread

CC_SRCS = \
	src/Column.cpp \
	src/Database.cpp \
	src/Statement.cpp \
	src/Transaction.cpp

.PHONY: all
all: build sqlitecpp install

build:
	$(shell mkdir -p build/include/sqlitecpp build/lib > /dev/null 2>&1)

src/%.o: $(@:.o=.cpp)
	$(CC) $(FLAGS) $(INCLUDES) $(@:.o=.cpp) -o $@

build/lib/libsqlitecpp.a: $(CC_SRCS:.cpp=.o)
	$(AR) cru $@ $(CC_SRCS:.cpp=.o)
	$(RANLIB) $@

.PHONY: sqlitecpp
sqlitecpp: build/lib/libsqlitecpp.a

install:
	install -m 0644 src/*.h build/include/sqlitecpp
#	install -m 0644 sqlite3/sqlite3.h build/include/sqlitecpp

clean:
	$(shell find . -name *.o | xargs rm -f)
	$(shell find . -name *.a | xargs rm -f)
	-@rm -rf build 2>/dev/null || true
