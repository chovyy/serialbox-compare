CPP = g++
CPPFLAGS = -std=c++11 -g 

SBDIR = $(HOME)/lib/serialbox

INCLUDE = -I$(SBDIR)/include/STELLA/libjson -I$(SBDIR)/include/Serialbox 
LIBS = -L$(SBDIR)/lib -lSerialbox -lUtils -ljson -lsha256


all: sbcompare sbdump

%: %.cpp sbshared.h
	$(CPP) $(CPPFLAGS) $(INCLUDE) -o $@ $< $(LIBS)

clean:
	rm -f sbcompare sbdump	
 
 
 