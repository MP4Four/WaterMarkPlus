# MinGW-w64 build (works for cross-compiling on Linux or natively on Windows with mingw32-make)
#   make                 -> 64-bit
#   make PREFIX=i686-w64-mingw32-   -> 32-bit
PREFIX  ?= x86_64-w64-mingw32-
CC      := $(PREFIX)gcc
WINDRES := $(PREFIX)windres
CFLAGS  := -std=gnu11 -Wall -Wextra -Os -municode -DUNICODE -D_UNICODE
LDFLAGS := -municode -mwindows -s -static
LIBS    := -lgdiplus -lcomdlg32 -lcomctl32 -lshell32 -lgdi32 -lole32 -luser32 -lkernel32

all: IDWatermark.exe

app.res.o: app.rc app.manifest app.ico
	$(WINDRES) -O coff -i app.rc -o $@

IDWatermark.exe: main.c app.res.o
	$(CC) $(CFLAGS) main.c app.res.o -o $@ $(LDFLAGS) $(LIBS)

clean:
	rm -f IDWatermark.exe app.res.o

.PHONY: all clean
