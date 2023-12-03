#ifndef CIRCULARQUEUE_H
#define CIRCULARQUEUE_H

#include <iostream>

class CircularQueue {
    
	private:	
		int rear;
		int front;
		int size;
		int *arr;

	public:
		CircularQueue(int s);
	 
		void add(int value);
		int pop();
		void display();
		bool isFull();
		bool isEmpty();
};
#endif
