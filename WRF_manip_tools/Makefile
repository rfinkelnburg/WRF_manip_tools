CXXFLAGS =	-O2 -g -Wall -fmessage-length=0


LIBS = 		-L. -lutils -Wl,-rpath,'.' -L../QuickPlot -lQuickPlot -Wl,-rpath,'../QuickPlot'
INCLUDE =	-I. -I../QuickPlot
TARGET =	utils IFF_dump IFF_copy

all:	$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).o libutils.so

utils:
	g++ -fPIC -shared utils.cpp -o libutils.so 
	
IFF_dump:
	$(CXX) -o IFF_dump IFF_dump.cpp $(INCLUDE) $(LIBS) 

IFF_copy:
	$(CXX) -o IFF_copy IFF_copy.cpp -I. -L. -lutils -Wl,-rpath,'.'