OBJECTS := main.o
INCLUDES := ../include/*.cpp

execute : $(OBJECTS)
	g++ -o $@ $^ $(INCLUDES)
	rm -f *.o

*.o: %.cpp
	g++ -o $@ -c $^ -I 