
GL_LIBS = `pkg-config --static --libs glfw3` -lGLEW -framework OpenGL
EXT =
CPPFLAGS = `pkg-config --cflags glfw3`


CC = g++
EXE = assign3_part2
OBJS = model-view.o Sphere.o Shader.o Viewer.o

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(GL_LIBS)

model-view.o: model-view.cpp InputState.h
	$(CC) $(CPPFLAGS) -c model-view.cpp

Shader.o : Shader.cpp Shader.hpp
	$(CC) $(CPPFLAGS) -c Shader.cpp

Viewer.o: Viewer.h Viewer.cpp InputState.h
	$(CC) $(CPPFLAGS) -c Viewer.cpp

Sphere.o: Sphere.hpp Sphere.cpp
	$(CC) $(CPPFLAGS) -c Sphere.cpp

clean:
	rm -f *.o $(EXE)$(EXT)
