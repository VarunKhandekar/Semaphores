#include "CircularQueue.h"


CircularQueue::CircularQueue(int _size){
	front = -1;
	rear = -1;
	size = _size;
	arr = new int[_size];
}


void CircularQueue::add(int value){
	if ((front == 0 && rear == size-1) || ((rear+1) % size == front)) { //second logic checks if rear is one behind the latest index for front
		return;
	}
	// Insert first element
	else if (front == -1) {
		front = rear = 0;
		arr[rear] = value; //add to back of queue
	}
	else if (rear == size-1 && front != 0) {
		rear = 0; //we circle back round -> set rear back to zero
		arr[rear] = value;
	}
	//standard case
	else {
		rear++;
		arr[rear] = value;
	}
}

int CircularQueue::pop(){
	if (front == -1) {
		std::cerr << "List is empty!" << std::endl;
		return 1;
	}
	 
	int data = arr[front];
	arr[front] = -1;
	if (front == rear){ //queue has now become empty
		front = -1;
		rear = -1;
	}
	else if (front == size-1){ //we have reached the end of the queue. Set the front back to 0
		front = 0;
	}
	else { //standard case
		front++;
	}
	return data;
}


void CircularQueue::display(){
	if (front == -1){
		std::cerr << "Empty" << std::endl;
		return;
	}
	
	if (rear >= front){
		for (int i = front; i <= rear; i++){
			std::printf("%d ",arr[i]);
		}
	}
	else {
		for (int i = front; i < size; i++){
			std::printf("%d ", arr[i]);
		}

		for (int i = 0; i <= rear; i++){
			std::printf("%d ", arr[i]);
		}
	}
}


bool CircularQueue::isFull(){
	if ((front == 0 && rear == size-1) || ((rear+1) % size == front)) { //second logic checks if rear is one behind the latest index for front
		return true;
	}
	return false;
}

bool CircularQueue::isEmpty(){
	if (front == -1){
		return true;
	}
	return false;
}

