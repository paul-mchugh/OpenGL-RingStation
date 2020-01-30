CXX=g++
CXXFLAGS=-I.
DEPS =
LINKS = -lGL -lGLEW -lglfw
OBJ = glMain.o
EXECNAME = gl155

%.o: %.c $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(EXECNAME): $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LINKS)

.PHONY: clean

clean:
	rm *.o
