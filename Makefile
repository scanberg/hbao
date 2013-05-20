Sources		= main.cpp Camera.cpp Shader.cpp Framebuffer2D.cpp Log.cpp Geometry.cpp ObjLoader.cpp Tokenizer.cpp

CFlags 		= -c -Wall -g -O3
LDFlags 	= 
ObjectDir 	= obj/
SourceDir 	= src/
Executable  = Program
BinDir 		= 

SYS := $(shell gcc -dumpmachine)
ifneq (, $(findstring linux, $(SYS)))
	LDFlags += -lglfw -lGLEW
else ifneq (, $(findstring mingw, $(SYS)))
	LDFlags += -lglfw -lglew32 -lopengl32
else ifneq (, $(findstring darwin, $(SYS)))
	LDFlags += -framework Cocoa -framework OpenGL -lglfw -lGLEW
endif

CC = g++
RM = rm

Objects 	= $(Sources:.cpp=.o)
CSources 	= $(addprefix $(SourceDir),$(Sources))
CObjects 	= $(addprefix $(ObjectDir),$(Objects))
CExecutable = $(addprefix $(BinDir),$(Executable))

all: $(CSources) $(CExecutable)

$(CExecutable): $(CObjects)
	$(CC) $(CObjects) $(LDFlags) -o $@

$(ObjectDir)%.o: $(SourceDir)%.cpp
	$(CC) $(CFlags) $< -o $@

clean:
	$(RM) $(CObjects)