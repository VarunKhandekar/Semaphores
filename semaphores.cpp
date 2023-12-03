#include <iostream>
#include <thread>
#include <semaphore.h>
#include <mutex>
#include <vector>
#include <chrono>
#include <functional>
#include "CircularQueue.h"


const int MAX_QUEUE_SIZE = 100;
// create semaphores; global scope so every consumer and producer can see
std::mutex mutex_semaphore;
sem_t is_space_semaphore;
sem_t not_empty_semaphore;


void producer(int number_of_jobs, CircularQueue& queue);
void consumer(CircularQueue& queue);


int main(){
	int queue_size;
	int number_of_producer_jobs;
	int producer_number;
	int consumer_number;
	
	// Command line input
	std::cout << "Please enter the size of the queue you would like:" << std::endl;
	std::cin >> queue_size;
	std::cout << "Please enter the number of jobs per producer you would like:" << std::endl;
	std::cin >> number_of_producer_jobs;
	std::cout << "Please enter the number of producers you would like:" << std::endl;
	std::cin >> producer_number;
	std::cout << "Please enter the number of consumers you would like:" << std::endl;
	std::cin >> consumer_number;


    if (queue_size <= 0 || number_of_producer_jobs <= 0 || producer_number <= 0 || consumer_number <= 0) {
        std::cerr << "Invalid input. All arguments must be greater than 0." << std::endl;
        return 1;
    }

    if (queue_size > MAX_QUEUE_SIZE) {
        std::cerr << "Queue size exceeds maximum allowed size." << std::endl;
        return 1;
    }

	// input variables are ok, so now create our queue
	CircularQueue queue(queue_size);
	// initialise our semaphores where appropriate
	sem_init(&is_space_semaphore, 0, queue_size);
	sem_init(&not_empty_semaphore, 0, 0);
	//is_space_semaphore = queue_size;
	//not_empty_semaphore = 0;


	// create producer threads
    std::vector<std::thread> producer_threads;
	for (int i=0; i<producer_number; i++){
		// thread needs a function to be run in the thread and arguments to pass to that function
		producer_threads.emplace_back([&]() { producer(number_of_producer_jobs, std::ref(queue));});
	}

	// create consumer threads
    std::vector<std::thread> consumer_threads;
	for (int i=0; i<consumer_number; i++){
		// thread needs a function to be run in the thread and arguments to pass to that function
		consumer_threads.emplace_back([&]() { consumer(std::ref(queue));});
	}

	// start producer threads
	for (auto& t : producer_threads){
		t.join();
	}
	
	// start consumer threads
	for (auto& t : consumer_threads){
		t.join();
	}

	return 0;
}




void producer(int number_of_producer_jobs, CircularQueue& queue) {
    for (int i = 0; i < number_of_producer_jobs; i++) {
        // Generate a random job duration between 1 and 10
		int job = rand() % 10 + 1;
		
		// Wait for an empty slot in the queue
		sem_wait(&is_space_semaphore);
		//is_space_semaphore.acquire();  
		// Lock the queue before adding a job
		mutex_semaphore.lock();
        queue.add(job);
		queue.display();
        std::cerr << "Produced a job with duration " << job << std::endl;
		// now unlock the queue 
		mutex_semaphore.unlock();
        // perform 'up' on the not_empty_semaphore to indicate there is a job that can be consumed
		sem_post(&not_empty_semaphore);
		//not_empty_semaphore.release();
    }
}

void consumer(CircularQueue& queue) {
    while (true) {
        // perform a 'down' on the not_empty_semaphore
		sem_wait(&not_empty_semaphore);
		//not_empty_semaphore.acquire();
        // now lock the queue while we perform checks on it
		mutex_semaphore.lock();

        // check if the queue has elements. If it does we can 'consume'
		if (!queue.isEmpty()) {
            int job = queue.pop();
            std::cerr << "Consumed a job of duration " << job << std::endl;
            // we've taken a job so now we can free up the queue
			mutex_semaphore.unlock();
            // perform an 'up' on this is_space semaphore to signal we have freed up a slot
			sem_post(&is_space_semaphore);
			//is_space_semaphore.release();
            std::this_thread::sleep_for(std::chrono::seconds(job));
        } 
		// case if the queue is empty. We can unlock it so other threads can use it
		else {
            mutex_semaphore.unlock();
			// sleep for 10s
            std::this_thread::sleep_for(std::chrono::seconds(10));
            // if the queue is still empty, exit
			if (queue.isEmpty()) {
                std::cerr << "No jobs left. Exiting consumer." << std::endl;
                break;
            }
        }
    }
}

