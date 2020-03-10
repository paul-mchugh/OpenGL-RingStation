CXX = g++
CXXFLAGS = -I. -g -I./include
DEPS = Util.h Generator.h Model.h World.h Camera.h
LINKS = -lGL -lGLEW -lglfw -lsoil2 -L./lib
OBJ = glMain.o Util.o Generator.o Model.o World.o Camera.o
EXECNAME = gl155

$(EXECNAME): $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LINKS)

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm *.o
