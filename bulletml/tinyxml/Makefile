#****************************************************************************
#
# Makefil for TinyXml test.
# Lee Thomason
# www.grinninglizard.com
#
# This is a GNU make (gmake) makefile
#****************************************************************************

# DEBUG can be set to YES to include debugging info, or NO otherwise
DEBUG          := NO

# PROFILE can be set to YES to include profiling info, or NO otherwise
PROFILE        := NO

#****************************************************************************

#CC     := /usr/local/gp2xdev/bin/gp2x-gcc
#CXX    := /usr/local/gp2xdev/bin/gp2x-g++
#LD     := /usr/local/gp2xdev/bin/gp2x-g++
#AR     := /usr/local/gp2xdev/bin/gp2x-ar rc
#RANLIB := /usr/local/gp2xdev/bin/gp2x-ranlib
#CC = /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-gcc
#CXX = /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-g++
#LD = /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-g++
#AR = /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-ar rc
#RANLIB = /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-ranlib
CC       = /opt/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-gcc
CXX      = /opt/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-g++
LD       = /opt/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-g++
AR       = /opt/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-ar rc
RANLIB   = /opt/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-ranlib

#DEBUG_CFLAGS     := -Wall -Wno-unknown-pragmas -Wno-format -g -DDEBUG
#RELEASE_CFLAGS   := -Wall -Wno-unknown-pragmas -Wno-format -O2


#DEBUG_CFLAGS = -O3 -DNDEBUG -DGP2X -mcpu=arm920t -mtune=arm920t -msoft-float -ffast-math -DGP2X -fomit-frame-pointer
#RELEASE_CFLAGS = -O3 -DNDEBUG -DGP2X -mcpu=arm920t -mtune=arm920t -msoft-float -ffast-math -DGP2X -fomit-frame-pointer
#DEBUG_CFLAGS = -O2 -DNDEBUG -DGP2X -mcpu=arm920t -mtune=arm920t -msoft-float -ffast-math -DGP2X -fomit-frame-pointer \
#				  -fsingle-precision-constant 
#RELEASE_CFLAGS = -O2 -DNDEBUG -DGP2X -mcpu=arm920t -mtune=arm920t -msoft-float -ffast-math -DGP2X -fomit-frame-pointer \
#				  -fsingle-precision-constant 
#DEBUG_CFLAGS = -O2 -DNDEBUG -DGP2X -mcpu=arm920t -mtune=arm920t -msoft-float -DGP2X -fomit-frame-pointer \
#				  -fsingle-precision-constant 
#RELEASE_CFLAGS = -O2 -DNDEBUG -DGP2X -mcpu=arm920t -mtune=arm920t -msoft-float -DGP2X -fomit-frame-pointer \
#				  -fsingle-precision-constant 
DEBUG_CFLAGS = -Wall -Wno-unknown-pragmas -Wno-format -g -DDDEBUG -DGCW -fsingle-precision-constant -ffast-math
#RELEASE_CFLAGS = -Wall -Wno-unknown-pragmas -Wno-format -O2 -DGCW -fsingle-precision-constant -ffast-math 
#RELEASE_CFLAGS = -Wall -Wno-unknown-pragmas -Wno-format -O3 -DGCW -fsingle-precision-constant -mno-shared -flto
RELEASE_CFLAGS = -Wall -Wno-unknown-pragmas -Wno-format -O2 -DGCW -fsingle-precision-constant -mno-shared

LIBS		 :=

DEBUG_CXXFLAGS   := ${DEBUG_CFLAGS} 
RELEASE_CXXFLAGS := ${RELEASE_CFLAGS}

DEBUG_LDFLAGS    := -g
RELEASE_LDFLAGS  :=

ifeq (YES, ${DEBUG})
   CFLAGS       += ${DEBUG_CFLAGS}
   CXXFLAGS     += ${DEBUG_CXXFLAGS}
   LDFLAGS      += ${DEBUG_LDFLAGS}
else
   CFLAGS       += ${RELEASE_CFLAGS}
   CXXFLAGS     += ${RELEASE_CXXFLAGS}
   LDFLAGS      += ${RELEASE_LDFLAGS}
endif

ifeq (YES, ${PROFILE})
   CFLAGS   := ${CFLAGS} -pg
   CXXFLAGS := ${CXXFLAGS} -pg
   LDFLAGS  := ${LDFLAGS} -pg
endif

#****************************************************************************
# Preprocessor directives
#****************************************************************************

ifeq (YES, ${PROFILE})
  DEFS :=
else
  DEFS :=
endif

#****************************************************************************
# Include paths
#****************************************************************************

#INCS := -I/usr/include/g++-2 -I/usr/local/include
INCS :=


#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS   := ${CFLAGS}   ${DEFS}
CXXFLAGS := ${CXXFLAGS} ${DEFS}

#****************************************************************************
# Targets of the build
#****************************************************************************

OUTPUT := xmltest

all: ${OUTPUT}


#****************************************************************************
# Source files
#****************************************************************************

SRCS := tinyxml.cpp tinyxmlparser.cpp xmltest.cpp tinyxmlerror.cpp

# Add on the sources for libraries
SRCS := ${SRCS}

OBJS := $(addsuffix .o,$(basename ${SRCS}))

#****************************************************************************
# Output
#****************************************************************************

${OUTPUT}: ${OBJS}
	${LD} -o $@ ${LDFLAGS} ${OBJS} ${LIBS} ${EXTRA_LIBS}

#****************************************************************************
# common rules
#****************************************************************************

# Rules for compiling source files to object files
%.o : %.cpp
	${CXX} -c ${CXXFLAGS} ${INCS} $< -o $@

%.o : %.c
	${CC} -c ${CFLAGS} ${INCS} $< -o $@

clean:
	-rm -f core ${OBJS} ${OUTPUT}

depend:
	makedepend ${INCS} ${SRCS}

distdir:
