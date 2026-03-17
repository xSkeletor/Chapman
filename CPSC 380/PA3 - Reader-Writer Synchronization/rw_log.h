/*
    Name: Dennis Fomichev
    Student ID: 2470131
    Email: fomichev@chapman.edu
    Course: CPSC 380-01
    Project: Programming Assignment 3 - Reader-Writer Synchronization

    This C header file is responsible for storing the definitions of the
    methods used within the read/write monitor, used in the rw_log.c source 
    code file. Most of is the same as the one given, except I added more libraries
    needed by the code, added a struct used to keep track of the current status of
    the monitor (and getter function), and a tweak to define this as a header file 
    so that I can use a separate C file to implement the various functions and methods.
*/

#ifndef RW_LOG_H
#define RW_LOG_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h> // Needed for CLOCK_REALTIME
#include <pthread.h>
#include <sys/types.h> // Needed for ssize_t, was getting an error when I didn't have this in the given header file

/* Maximum message payload per log entry (writer fills this) */
#define RWLOG_MSG_MAX 64

/* One log record stored in the circular buffer.
 * NOTE: seq/tid/ts are assigned by the monitor during append(). */
typedef struct {
    uint64_t      seq;                // global, monotonically increasing
    pthread_t     tid;                // writing thread's pthread id
    struct timespec ts;               // timestamp (CLOCK_REALTIME)
    char          msg[RWLOG_MSG_MAX]; // writer-supplied short message
} rwlog_entry_t;

/* === Monitor API (pthreads + POSIX shared memory) ===
 * The monitor implements writer-preference RW semantics:
 *  - New readers are blocked if a writer is active OR any writer is waiting.
 *  - Exactly one writer holds the write section at a time.
 *  - No busy waiting: all waits use pthread_cond_wait in a while-loop. */
int     rwlog_create(size_t capacity);                             // allocate/init monitor in POSIX shared memory
int     rwlog_destroy(void);                                       // destroy sync, unmap & unlink shm

int     rwlog_begin_read(void);                                    // enter read section (may block)
ssize_t rwlog_snapshot(rwlog_entry_t *buf, size_t max_entries);    // copy newest <= max_entries to caller buffer
int     rwlog_end_read(void);                                      // leave read section

int     rwlog_begin_write(void);                                   // enter write section (may block)
int     rwlog_append(const rwlog_entry_t *e);                      // append one entry (must be inside write section)
int     rwlog_end_write(void);                                     // leave write section

/* Wake any threads blocked in the monitor (used on shutdown). Safe to call anytime. */
void    rwlog_wake_all(void);

typedef struct { // Used in the monitors shared data
    // For synchronization management
    pthread_mutex_t mutex;
    pthread_cond_t canRead;
    pthread_cond_t canWrite;

    int readCount; // # of readers currently inside
    int writeActive; // 0/1 (whether a writer holds the mutex)
    int waitingWriters; // Writers queued for the write section

    // For the circular log
    size_t capacity;
    size_t size; // Number of valid entries, should be <= the capacity above
    size_t writeIndex; // Next index to write into 
    size_t readIndex; // Next index to read from 
    uint64_t nextSeq; // Next sequence to assign 

    rwlog_entry_t entries[]; // Flexible array (capacity at allocation)
} rwlog_state_t;

// Needed to get the current state of the monitor from rw_main.c
rwlog_state_t *rwlog_get_state(void);

#endif