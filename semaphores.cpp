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
bool wait_at_semaphore(sem_t* semaphore, std::chrono::seconds timeout);


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
    std::chrono::seconds timeout(10); //set wait time at the semaphore to be a max of 10 seconds 
    
	for (int i = 0; i < number_of_producer_jobs; i++) {
		
		// Wait for an empty slot in the queue. If less than 10s, we move into the body of the if statement
		// we perform the semaphore 'down' in the boolean function itself
		if (wait_at_semaphore(&is_space_semaphore, timeout)){
			// Generate a random job duration between 1 and 10
			int job = rand() % 10 + 1;

			//sem_wait(&is_space_semaphore); //reduce space
			// Lock the queue before adding a job
			mutex_semaphore.lock();
			queue.add(job);
			queue.display();
			std::cerr << "Produced a job with duration " << job << std::endl;
			// now unlock the queue 
			mutex_semaphore.unlock();
			// perform 'up' on the not_empty_semaphore to indicate there is a job that can be consumed
			sem_post(&not_empty_semaphore); //add one to amount in queue

		}
		else {
			std::cerr << "Exiting producer as wait time exceeded 10s." << std::endl;
			break;
		}
		 
    }
}

void consumer(CircularQueue& queue) {
    std::chrono::seconds timeout(10); //set wait time at the semaphore to be a max of 10 seconds 
	while (true) {
		// perform a 'down' on the not_empty_semaphore
		sem_wait(&not_empty_semaphore); //reduce amount in queue

		//now lock the queue while we perform checks on it
		mutex_semaphore.lock();

		//check if the queue has elements. If it does we can 'consume'
        int job = queue.pop();
        std::cerr << "Consumed a job of duration " << job << std::endl;
			
        // we've taken a job so now we can free up the queue
		mutex_semaphore.unlock();
			
        // perform an 'up' on this is_space semaphore to signal we have freed up a slot
		sem_post(&is_space_semaphore);
			
        std::this_thread::sleep_for(std::chrono::seconds(job));
         
		// case if the queue is empty
		if (queue.isEmpty()){
			// sleep for 10s
            std::this_thread::sleep_for(std::chrono::seconds(10));
            // if the queue is still empty, exit
			if (queue.isEmpty()){
				std::cerr << "No jobs left. Exiting consumer." << std::endl;
				break;
			}
		}
    }
}

bool wait_at_semaphore(sem_t* semaphore, std::chrono::seconds timeout) {
    auto start = std::chrono::steady_clock::now();

    while (true) {
	    if (sem_trywait(semaphore) == 0) {
			return true; // Acquired the semaphore
		}
		
		auto end = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start);
		
		if (elapsed >= timeout) {
			return false;
		}
	}
}
