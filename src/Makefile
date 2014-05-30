
CXX = c++
CXXFLAGS = -g -DLINUX -D_GNULINUX -Wall -O3 -I. -fPIC -Ipng++-0.2.3/ -I/opt/boost/current/include
LDDFLAGS = -shared


SOURCES = $(wildcard $(SRCDIR)*.cpp)
OBJECTS = $(patsubst $(SRCDIR)%.cpp, %.o, $(SOURCES))
HEADERS = $(wildcard $(SRCDIR)*.h)

TARGET = libBaumer.so

default: $(TARGET)
	@echo built $(TARGET)


$(TARGET): $(OBJECTS) $(HEADERS) Makefile
	$(CXX) $(LDDFLAGS) -o $@ $(OBJECTS)

%.o: $(SRCDIR)%.cpp Makefile
	$(CXX) -c $< $(CXXFLAGS)


clean:
	@rm -f $(OBJECTS)
	@echo cleaned

realclean: clean
	@rm -f $(TARGET) *~
	@echo realcleaned

