CC=clang
CFLAGS=-fPIC -std=c11 -O2 -Wall \
        -Wno-deprecated-declarations \
        -Wno-incompatible-pointer-types-discards-qualifiers \
        -Wno-unused-parameter -pthread \
        -D_GNU_SOURCE -D_XOPEN_SOURCE=700

OPENSSL_CFLAGS := $(shell pkg-config --cflags openssl 2>/dev/null)
OPENSSL_LIBS   := $(shell pkg-config --libs openssl 2>/dev/null)
ifeq ($(strip $(OPENSSL_LIBS)),)
OPENSSL_ROOT ?= /opt/homebrew/Cellar/openssl@3/3.5.0
OPENSSL_CFLAGS := -I$(OPENSSL_ROOT)/include
OPENSSL_LIBS   := -L$(OPENSSL_ROOT)/lib -lssl -lcrypto
endif

INCLUDES=-Iinclude $(OPENSSL_CFLAGS)
LDFLAGS=$(OPENSSL_LIBS) -shared -pthread

SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)
TARGET=libmerkle.so

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
