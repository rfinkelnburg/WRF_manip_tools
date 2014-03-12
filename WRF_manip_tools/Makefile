CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

LIBS =

TARGET =	IFF_dump

all:	$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).o

IFF_dump: IFF_dump.o
	$(CXX) -o IFF_dump IFF_dump.cpp $(LIBS)