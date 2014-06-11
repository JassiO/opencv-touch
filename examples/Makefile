
CXX = c++
CXXFLAGS = -g -DLINUX -D_GNULINUX -Wall -O3 -I../src -I/opt/boost/boost_1_55_0/include \
	-L../src -Wl,-rpath,../src -lBaumer -lrt -L/opt/boost/boost_1_55_0/lib -Wl,-rpath,/opt/boost/boost_1_55_0/lib -lboost_thread -ljpeg -lpng -lGL -lglut -lpthread -lbgapi \
	-lopencv_core -lopencv_highgui -lopencv_features2d -lopencv_imgproc -lopencv_video -lboost_system


SOURCES = $(wildcard *.cpp)
OBJECTS = $(patsubst %.cpp, %.o, $(SOURCES))
TARGETS = $(patsubst %.cpp, %, $(SOURCES))


default:
	cd ../src && make
	make examples

examples: $(TARGETS)
	@echo built $(TARGETS)


%: %.cpp Makefile
	$(CXX) $< $(CXXFLAGS) -o $@

clean:
	cd ../src && make clean
	@rm -f $(TARGETS)
	@echo cleaned

realclean: clean
	cd ../src && make realclean
	@rm -f *~
	@echo realcleaned

