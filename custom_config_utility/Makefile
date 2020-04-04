# Makefile template credits to Nebuleon and mth

CC          :=  /opt/gcw0-toolchain/usr/bin/mipsel-linux-gcc
#CC          :=  gcc

SYSROOT     := $(shell $(CC) --print-sysroot)

OBJS        := cfg.o SFont.o gcw.o menu_game_settings.o menu_control_settings.o menu_others.o
HEADERS     :=

INCLUDE     := -I. -I$(SYSROOT)/usr/include -I$(SYSROOT)/usr/include/SDL
DEFS        +=

CFLAGS       = -std=gnu99 -Wall -Wno-unused-variable -O2 $(DEFS) $(INCLUDE)
#CFLAGS       = -std=gnu99 -Wall -Wno-unused-variable -O0 -g $(DEFS) $(INCLUDE)
CFLAGS      += -DGCW
LDFLAGS     := -L$(SYSROOT)/usr/lib -lSDL -lpthread -lSDL_gfx -lSDL_image -lz 

.PHONY: all clean

all: rr_cfg

#rr_cfg: cfg.o SFont.o gcw.o menu_game_settings.o menu_control_settings.o menu_others.o
rr_cfg: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	chmod +x rr_cfg

clean:
	rm rr_cfg *.o

# The two below declarations ensure that editing a .c file recompiles only that
## file, but editing a .h file recompiles everything.
## Courtesy of Maarten ter Huurne.
#
# Each object file depends on its corresponding source file.
$(C_OBJS): %.o: %.c
#
# Object files all depend on all the headers.
$(OBJS): $(HEADERS)

