/*
    Name: Dennis Fomichev
    Student ID: 2470131
    Email: fomichev@chapman.edu
    Course: CPSC 380-01
    Project: Programming Assignment 2 - Thread Synchronization

    This program implements a producer-consumer system with a buffer using POSIX 
    threads, semaphores, and a mutex. Producers will generate random 30-byte items 
    with checksums and insert them into a circular buffer, while consumers remove items 
    and recompute checksums to verify. The number of producers and consumers created, along with
    the amount of time the program runs for is inputted as arugments in the shell when running the 
    program.
*/

#include <stdint.h> // Needed for int sizes
#include <stdlib.h> // Needed for rand()
#include <unistd.h> // Needed for sleep
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include "buffer.h"

sem_t empty;
sem_t full;
pthread_mutex_t mutex;

BUFFER_ITEM buffer[BUFFER_SIZE];

int inserting = 0;
int removing = 0;

// Provided by assingment speicifcation in checksum.c
uint16_t checksum(char *addr, uint32_t count) {
    register uint32_t sum = 0;
    uint16_t *buf = (uint16_t *) addr;

    // Main summing loop
    while(count > 1)
    {
        sum = sum + *(buf)++;
        count = count - 2;
    }

    // Add left-over byte, if any
    if (count > 0)
        sum = sum + *addr;

    // Fold 32-bit sum to 16 bits
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return (~sum);
}

// Insert item into buffer, returns 0 if successful and -1 if error
int insert_item(BUFFER_ITEM item) {
    // Check if can't wait on semaphore or if we can't lock mutex, if any of these can't be done then return out of function
    if (sem_wait(&empty) != 0 || pthread_mutex_lock(&mutex) != 0) {
        return -1; 
    }

    // Finally insert the item into the buffer and change the inserting position index
    buffer[inserting] = item;
    inserting = (inserting + 1) % BUFFER_SIZE;

    // Check if can't unlock the mutex or if we can't post the semaphore, then show that there was an error
    if (pthread_mutex_unlock(&mutex) != 0 || sem_post(&full) != 0) {
        return -1; 
    }

    return 0;
}

// Removes an object from buffer placing it in item, returns 0 if successful and -1 if there's an error
int remove_item(BUFFER_ITEM *item) {
    // Check if can't wait on semaphore or if we can't lock mutex, if any of these can't be done then return out of function
    if (sem_wait(&full) != 0 || pthread_mutex_lock(&mutex) != 0) {
        return -1; 
    }

    // Remove the item from the buffer (and store it in pointer) and change the removing position index
    *item = buffer[removing];
    removing = (removing + 1) % BUFFER_SIZE;

    // Check if can't unlock the mutex or if we can't post the semaphore, then show that there was an error
    if (pthread_mutex_unlock(&mutex) != 0 || sem_post(&empty) != 0) {
        return -1; 
    }

    return 0;
}

// Manages the producer threads
void *producer(void *param) {
    BUFFER_ITEM item;

    while (1) { // Needed to use 1 instead of true because I forgot that C doesn't have booleans
        // Release the CPU randomly to simulate preemption *
        if (rand() % 100 < 40) { // ~40% chance
            sched_yield(); // Voluntarily yield CPU
        }

        // Generate the item
        for (int i = 0; i < 30; ++i) {
            item.data[i] = rand(); // Random bytes
        }

        item.cksum = checksum(item.data, 30);

        printf("Producer generated item with checksum %u.\n", item.cksum);

        if (insert_item(item)) { // Returns a value if there was an error inserting the item
            fprintf(stderr, "There was an error inserting the item.\n");
        }
    }
}

void *consumer(void *param) {
    BUFFER_ITEM item;

    while (1) { // Needed to use 1 instead of true because I forgot that C doesn't have booleans
        // Release the CPU randomly to simulate preemption *
        if (rand() % 100 < 40) { // ~40% chance
            sched_yield(); // Voluntarily yield CPU
        }

        if (remove_item(&item)) { // Returns a value if there was an error removing the item
            fprintf(stderr, "There was an error removing the item.\n");
        }

        uint16_t newChecksum = checksum(item.data, 30);

        printf("Consumer read item with checksum %u.\n", item.cksum);

        // Generated a new checksum based on the data and now see if it matches the old one
        if (newChecksum != item.cksum) {
            fprintf(stderr, "Checksum does not match, the expected checksum was %u but the new calculated one was %u.\n", item.cksum, newChecksum);
        }
    }
}

int main(int argc, char *argv[]) {
    // Check to make sure user actually submitted correct number of arguments
    if (argc != 4) {
        printf("Usage: %s <delay> <#producer threads> <#consumer threads>\n", argv[0]);

        return -1;
    }

    // 1. Parse the arguments
    int delay = atoi(argv[1]);
    int producerThreads = atoi(argv[2]);
    int consumerThreads = atoi(argv[3]);

    // 2. Initialize the buffer
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    // 3. Create producer threads
    pthread_t producers[producerThreads];
    for (int i = 0; i < producerThreads; i++) {
        pthread_create(&producers[i], NULL, producer, NULL);
    }

    // 4. Create consumer threads
    pthread_t consumers[consumerThreads];
    for (int i = 0; i < consumerThreads; i++) {
        pthread_create(&consumers[i], NULL, consumer, NULL);
    }

    // 5. Sleep for the user provided delay
    sleep(delay);

    // 6. Exit (cancel the threads, wait for them to be done, and also clean up the semaphores and mutex)
    for (int i = 0; i < producerThreads; i++) {
        pthread_cancel(producers[i]);
    }

    for (int i = 0; i < consumerThreads; i++) {
        pthread_cancel(consumers[i]);
    }

    for (int i = 0; i < producerThreads; i++) {
        pthread_join(producers[i], NULL);
    }

    for (int i = 0; i < consumerThreads; i++) {
        pthread_join(consumers[i], NULL);
    }

    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);
    
    return 0;
}