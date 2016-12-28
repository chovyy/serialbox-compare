CPP = g++
CPPFLAGS = -std=c++11 -g 

SBDIR = /home/christian/lib/serialbox

INCLUDE = -I$(SBDIR)/include/STELLA/libjson -I$(SBDIR)/include/SerialBox 
LIBS = -L$(SBDIR)/lib -lSerialBox -lUtils -ljson -lsha256


all: compare dump

%: %.cpp shared.h
	$(CPP) $(CPPFLAGS) $(INCLUDE) -o $@ $< $(LIBS)

clean:
	rm -f compare dump	
 
 
 