
CXX = c++
CXXFLAGS = -g -DLINUX -D_GNULINUX -Wall -O3 -I../src -I/opt/boost/current/include \
	-L../src -Wl,-rpath,../src -lBaumer -lrt -L/opt/boost/current/lib -Wl,-rpath,/opt/boost/current/lib -lboost_thread -ljpeg -lpng -lGL -lglut -lpthread -lbgapi \
	-lopencv_core -lopencv_highgui -lopencv_features2d -lopencv_imgproc -lopencv_video


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

