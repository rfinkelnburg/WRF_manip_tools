CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

TARGET =	libutils libiff libwrf IFF_dump IFF_copy WRF_dump WRF_copy WRF2IFF

all:	$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).o libutils.so libiff.so libwrf.so

libutils:
	g++ -fPIC -shared libutils.cpp -o libutils.so -lm

libiff:
	g++ -fPIC -shared libiff.cpp -o libiff.so -lutils -Wl,-rpath,'/usr/local/lib' -lQuickPlot -Wl,-rpath,'/usr/local/lib'

libwrf:
	g++ -fPIC -shared libwrf.cpp -o libwrf.so -lutils -Wl,-rpath,'/usr/local/lib' -lQuickPlot -Wl,-rpath,'/usr/local/lib' -lnetcdf
	
IFF_dump:
	$(CXX) -o IFF_dump IFF_dump.cpp -lutils -Wl,-rpath,'/usr/local/lib' -liff -Wl,-rpath,'/usr/local/lib' -lQuickPlot -Wl,-rpath,'/usr/local/lib'

IFF_copy:
	$(CXX) -o IFF_copy IFF_copy.cpp -lutils -Wl,-rpath,'/usr/local/lib' -liff -Wl,-rpath,'/usr/local/lib'
	
WRF_dump:
	$(CXX) -o WRF_dump WRF_dump.cpp -lwrf -Wl,-rpath,'/usr/local/lib'
	
WRF_copy:
	$(CXX) -o WRF_copy WRF_copy.cpp -lwrf -Wl,-rpath,'/usr/local/lib'

WRF2IFF:
	$(CXX) -o WRF2IFF WRF2IFF.cpp -lutils -Wl,-rpath,'/usr/local/lib' -liff -Wl,-rpath,'/usr/local/lib' -lwrf -Wl,-rpath,'/usr/local/lib'