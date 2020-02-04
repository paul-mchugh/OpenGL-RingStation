CXX=g++
CXXFLAGS=-I. -g
DEPS = Util.h
LINKS = -lGL -lGLEW -lglfw
OBJ = glMain.o Util.o
EXECNAME = gl155

%.o: %.c $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(EXECNAME): $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LINKS)

.PHONY: clean

clean:
	rm *.o
