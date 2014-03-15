CXXFLAGS =	-O2 -g -Wall -fmessage-length=0


LIBS = 		-L../QuickPlot -lQuickPlot -Wl,-rpath,'../QuickPlot'
INCLUDE =	-I../QuickPlot
TARGET =	IFF_dump

all:	$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).o

IFF_dump:
	$(CXX) -o IFF_dump IFF_dump.cpp $(INCLUDE) $(LIBS)
