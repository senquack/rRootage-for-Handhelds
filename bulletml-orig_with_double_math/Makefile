#CC = /usr/local/gp2xdev/bin/gp2x-g++
CC = /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-g++
INCLUDES = -I.
LIBS = tinyxml/tinyxml.o tinyxml/tinyxmlparser.o tinyxml/tinyxmlerror.o
#CXXFLAGS = -g -W -Wall -ansi -pedantic
#CXXFLAGS = -pg -g -W -Wall -ansi -pedantic
#CXXFLAGS = -O2 -W -Wall -ansi -pedantic
#CXXFLAGS = -O3 -DNDEBUG -DGP2X -mcpu=arm920t -mtune=arm920t -msoft-float -ffast-math -DGP2X -fomit-frame-pointer
CXXFLAGS = -DLINUX -O2 -DNDEBUG -Wall -DGP2X -msoft-float -mcpu=arm920t -mtune=arm920 -ffast-math -fomit-frame-pointer \
				  -fsingle-precision-constant 

#CFLAGS = -O2
#CFLAGS = -O3 -DNDEBUG -DGP2X -mcpu=arm920t -mtune=arm920t -msoft-float -ffast-math -DGP2X -fomit-frame-pointer
CFLAGS = -DLINUX -O2 -DNDEBUG -Wall -DGP2X -msoft-float -mcpu=arm920t -mtune=arm920 -ffast-math -fomit-frame-pointer \
				  -fsingle-precision-constant 

OBJS = bulletmlparser-tinyxml.o bulletmlparser.o bulletmltree.o calc.o formula-variables.o bulletmlrunner.o bulletmlrunnerimpl.o

all: libbulletml.a

libbulletml.a: $(OBJS)
	$(MAKE) -C tinyxml
	$(AR) -r libbulletml.a *.o tinyxml/tiny*.o

clean:
	rm -f *.o *.a
	$(MAKE) -C tinyxml clean

calc.cpp: calc.yy
	bison -y   calc.yy && mv y.tab.c calc.cpp

$(OBJS): %.o: %.cpp
	$(CC) -c $(CXXFLAGS) $(INCLUDES) $<

doxy:
	doxygen

distdir:

