semaphores: semaphores.cpp CircularQueue.o
	g++ -g -Wall semaphores.cpp CircularQueue.o -o semaphores

CircularQueue.o: CircularQueue.h CircularQueue.cpp
	g++ -g -Wall -c CircularQueue.cpp

q8: q8.cpp
	g++ -g -Wall q8.cpp -o q8
