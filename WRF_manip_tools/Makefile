CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

TARGET =	utils IFF_dump IFF_copy WRF_dump

all:	$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).o libutils.so

utils:
	g++ -fPIC -shared utils.cpp -o libutils.so 
	
IFF_dump:
	$(CXX) -o IFF_dump IFF_dump.cpp -I. -I../QuickPlot -L. -lutils -Wl,-rpath,'.' -L../QuickPlot -lQuickPlot -Wl,-rpath,'../QuickPlot'

IFF_copy:
	$(CXX) -o IFF_copy IFF_copy.cpp -I. -L. -lutils -Wl,-rpath,'.'
	
WRF_dump:
	$(CXX) -o WRF_dump WRF_dump.cpp -I. -I../QuickPlot -L. -lutils -Wl,-rpath,'.' -L../QuickPlot -lQuickPlot -Wl,-rpath,'../QuickPlot' -lnetcdf -lnetcdf_c++