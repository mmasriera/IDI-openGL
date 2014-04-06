
LIBS = -lglut -lGL -lGLU
CC = g++
OBJS = model.o main.o

#############################

all: bloc3

bloc3: $(OBJS)
	$(CC) -o main $(OBJS) $(LIBS)

main: main.cpp
	$(CC) -c main.cpp $(LIBS)

model: model.cpp
	$(CC) -c model.cpp

clean:
	rm main main.o model.o 
