/*
    Name: Dennis Fomichev
    Student ID: 2470131
    Email: fomichev@chapman.edu
    Course: CPSC 380-01
    Project: Programming Assignment 3 - Reader-Writer Synchronization

    This C source code file implements the various methods defined in the header
    rw_log file and overall defines how the read and write monitor works, along with
    the circular log buffer used to store the log entries. The monitor uses pthreads
    and POSIX shared memory to allow multiple processes to read and write to the log
    in a synchronized manner, following writer-preference reader-writer semantics.
*/

#include "rw_log.h"

static rwlog_state_t *S = NULL;

// Needed to get the current state of the monitor from rw_main.c
rwlog_state_t *rwlog_get_state(void) {
    return S;
}

// allocate/init monitor in POSIX shared memory
int rwlog_create(size_t capacity) {
    // Allocate memory to the monitor based on the amount of entries to be created (capacity)
    S = (rwlog_state_t *)malloc(sizeof(rwlog_state_t) + capacity * sizeof(rwlog_entry_t));
    
    // Quick sanity check to make sure that malloc didn't fail for some reason
    if (!S) {
        return -1;
    }

    // Default variables values
    S->capacity = capacity;
    S->readCount = 0;
    S->writeActive = 0;
    S->waitingWriters = 0;
    S->size = 0;
    S->writeIndex = 0;
    S->readIndex = 0;
    S->nextSeq = 0;

    // Initialize the mutex
    if (pthread_mutex_init(&S->mutex, NULL) != 0) {
        return -1;
    }

    // Initialize the write condition variable
    if (pthread_cond_init(&S->canWrite, NULL) != 0) {
        return -1;
    }

    // Initialize the read condition variable
    if (pthread_cond_init(&S->canRead, NULL) != 0) {
        return -1;
    }

    return 0;
}

// destroy sync, unmap & unlink shm
int rwlog_destroy(void) {
    if (!S) {
        return -1;
    }

    // Destroy the threads first
    pthread_cond_destroy(&S->canRead);
    pthread_cond_destroy(&S->canWrite);
    pthread_mutex_destroy(&S->mutex);

    // After threads are gone, clean up memory and reset the monitor
    free(S);
    S = NULL;

    return 0;
}

// enter read section (may block)
int rwlog_begin_read(void) {
    if (!S) {
        return -1;
    }

    if (pthread_mutex_lock(&S->mutex) != 0) {
        return -1;
    }

    // Wait while there is writing happening or writers waiting (writer priority)
    while (S->writeActive > 0 || S->waitingWriters > 0) {
        pthread_cond_wait(&S->canRead, &S->mutex);
    }

    // Increment read count since we're starting a read
    S->readCount++;

    pthread_mutex_unlock(&S->mutex);

    return 0;
}

// copy newest <= max_entries to caller buffer
ssize_t rwlog_snapshot(rwlog_entry_t *buf, size_t max_entries) {
    if (!S || !buf || max_entries == 0) {
        return -1;
    }

    if (pthread_mutex_lock(&S->mutex) != 0) {
        return -1;
    }

    // Check to make sure we can't copy more entries than exist
    if (S->size < max_entries) {
        max_entries = S->size;
    }

    // First find the index based on what is being read, then copy the new entry to the buffer at index
    for (size_t i = 0; i < max_entries; i++) {
        size_t index = (S->readIndex + i) % S->capacity;
        buf[i] = S->entries[index];
    }

    pthread_mutex_unlock(&S->mutex);

    return (ssize_t)max_entries;
}

// leave read section
int rwlog_end_read(void) {
    if (!S) {
        return -1;
    }

    if (pthread_mutex_lock(&S->mutex) != 0) {
        return -1;
    }

    // Decrement the read count since we're stopping read
    S->readCount--;

    // Start a writer if there are no readers but writing is still happening (starvation)
    if (S->readCount == 0 && S->waitingWriters > 0) {
        pthread_cond_signal(&S->canWrite);
    }

    pthread_mutex_unlock(&S->mutex);

    return 0;
}                                 

// enter write section (may block)
int rwlog_begin_write(void) {
    if (!S) {
        return -1;
    }

    if (pthread_mutex_lock(&S->mutex) != 0) {
        return -1;
    }

    // Increment the nnumber of waiting writers, since we're about to start
    S->waitingWriters++;

    // Make sure that no readers or writers are active
    while (S->writeActive > 0 || S->readCount > 0) {
        pthread_cond_wait(&S->canWrite, &S->mutex);
    }

    // No longer need the waiting writer since it's going to actively write now
    S->waitingWriters--;
    S->writeActive = 1;

    pthread_mutex_unlock(&S->mutex);

    return 0;
}

// append one entry (must be inside write section)
int rwlog_append(const rwlog_entry_t *e) {
    if (!S || !e) {
        return -1;
    }

    if (pthread_mutex_lock(&S->mutex) != 0) {
        return -1;
    }

    // Find the index which we have to write to
    size_t pos = S->writeIndex;

    // Copy the new entry into the log, setup its values
    S->entries[pos] = *e;
    S->entries[pos].seq = S->nextSeq++;
    S->entries[pos].tid = pthread_self();
    clock_gettime(CLOCK_REALTIME, &S->entries[pos].ts); // The "CLOCK_REALTIME" is giving me an error in VS Code, but it compiles and runs fine with gcc

    // Update the new write index of monitor
    S->writeIndex = (pos + 1) % S->capacity;

    // Increase the size of the monitor if it's not full yet
    if (S->size < S->capacity) {
        S->size++;
    } else { // Otherwise, start to overwrite oldest by changing read index
        S->readIndex = (S->readIndex + 1) % S->capacity;
    }

    pthread_mutex_unlock(&S->mutex);

    return 0;
}

// leave write section
int rwlog_end_write(void) {
    if (!S) {
        return -1;
    }

    if (pthread_mutex_lock(&S->mutex) != 0) {
        return -1;
    }

    // Ensure we are in a write section
    S->writeActive = 0;
    
    // Check if a writer needs to be started in order to prevent starvation
    if (S->waitingWriters > 0) {
        pthread_cond_signal(&S->canWrite);
    } else { // If we're done with writers, wake all readers
        pthread_cond_broadcast(&S->canRead);
    }

    pthread_mutex_unlock(&S->mutex);

    return 0;
}

/* Wake any threads blocked in the monitor (used on shutdown). Safe to call anytime. */
void rwlog_wake_all(void) {
    if (!S) {
        return;
    }

    // Lock the mutex, wake up all the threads dependent on conditions, then unlock mutex
    pthread_mutex_lock(&S->mutex);

    pthread_cond_broadcast(&S->canWrite);
    pthread_cond_broadcast(&S->canRead);

    pthread_mutex_unlock(&S->mutex);
}