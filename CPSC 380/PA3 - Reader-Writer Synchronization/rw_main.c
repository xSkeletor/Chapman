/*
    Name: Dennis Fomichev
    Student ID: 2470131
    Email: fomichev@chapman.edu
    Course: CPSC 380-01
    Project: Programming Assignment 3 - Reader-Writer Synchronization

    This C source code file contains the main function and thread implementations for
    reader-writer synchronization via monitors. It sets up the configuration via command-line 
    arguments, initializes the shared log, creates reader and writer threads, and manages 
    their execution and termination. The program also supports graceful termination via signal
    handling and it then outputs performance metrics and optionally dumps the log to a CSV file.
*/

/* Add appropriate header files */
#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "rw_log.h"

// The only global variable needed
static volatile sig_atomic_t stop_flag = 0;

// Provided struct for configuration options in the command-line arguments
struct config {
    int capacity;
    int readers;
    int writers;
    int writer_batch;
    int seconds;
    int rd_us;
    int wr_us;
    int dump_csv;
};

// Used for writer threads
typedef struct {
    int id;
    int batch;
    int wr_us;

    int appendedCount;
    double writeTimeMS;
} writerMetrics;

// Used for reader threads
typedef struct {
    int id;
    int rd_us;

    int snapshotsCount;
    double readTimeMS;
} readerMetrics;

// Prints usage information (already provided in skeleton code)
static void print_usage(const char *progname) 
{
	fprintf(stderr,
        "Usage: %s [options]\n"
        "Options:\n"
        "-c,  --capacity <N>        Log capacity (default 1024)\n"
        "-r,  --readers <N>         Number of reader threads (default 6)\n"
        "-w,  --writers <N>         Number of writer threads (default 4)\n"
        "-b,  --writer-batch <N>    Entries written per writer section (default 2)\n"
        "-s,  --seconds <N>         Total run time (default 10)\n"
        "-R,  --rd-us <usec>        Reader sleep between operations (default 2000)\n"
        "-W,  --wr-us <usec>        Writer sleep between operations (default 3000)\n"
        "-d,  --dump                Dump final log to log.csv\n"
        "-h,  --help                Show this help message\n",
        progname);
}

// Parses command-line arguments and fills the config struct, but also sets defaults if no values provided
static void parse_args(int argc, char **argv, struct config *cfg)
{
	// Set defaults
    cfg->capacity     = 1024;
    cfg->readers      = 6;
    cfg->writers      = 4;
    cfg->writer_batch = 2;
    cfg->seconds      = 10;
    cfg->rd_us        = 2000;
    cfg->wr_us        = 3000;
    cfg->dump_csv     = 0;
	
	/* define the long_opts options */
    static struct option long_options[] = {
        {"capacity", required_argument, 0, 'c'},
        {"readers", required_argument, 0, 'r'},
        {"writers", required_argument, 0, 'w'},
        {"writer-batch", required_argument, 0, 'b'},
        {"seconds", required_argument, 0, 's'},
        {"rd-us", required_argument, 0, 'R'},
        {"wr-us", required_argument, 0, 'W'},
        {"dump", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int option;
	
	/* Parse each of the arguments using getopt_long() function */
    while ((option = getopt_long(argc, argv, "c:r:w:b:s:R:W:dh", long_options, NULL)) != -1) {
        switch (option) {
            case 'c':
                cfg->capacity = atoi(optarg);
                break;
            case 'r':
                cfg->readers = atoi(optarg);
                break;
            case 'w':
                cfg->writers = atoi(optarg);
                break;
            case 'b':
                cfg->writer_batch = atoi(optarg);
                break;
            case 's':
                cfg->seconds = atoi(optarg);
                break;
            case 'R':
                cfg->rd_us = atoi(optarg);
                break;
            case 'W':
                cfg->wr_us = atoi(optarg);
                break;
            case 'd':
                cfg->dump_csv = 1;
                break;
            case 'h': // Just show the usage of the program and then exit since nothing needs to be done
                print_usage(argv[0]);
                exit(0);
            case '?': // These last 2 cases are from the in-class example
                // getopt_long already printed an error message
                break;
            default:
                abort();
        }
    }
}

// Signal handler for SIGINT which is called when the user presses Ctrl+C
static void SigInt(int sig) {
    (void)sig;

    stop_flag = 1;

    rwlog_wake_all();
}

// Timer thread function to set the stop_flag after a certain number of seconds
static void *TimerThread(void *arg) {
    sleep(*(int *)arg); // Convert it to a pointer to int and then dereference it to get the actual int value

    stop_flag = 1;

    rwlog_wake_all();
}

// Called by each writer thread
static void *WriterThread(void *arg) {
    writerMetrics *metrics = (writerMetrics *)arg;
    struct timespec start, end;

    while (!stop_flag) {
        // 1. Record start time
        clock_gettime(CLOCK_MONOTONIC, &start);

        // 1. (cont) Stop if we can't even write in the first place
        if (rwlog_begin_write() != 0) {
            break;
        }

        // 2. Record end time and calculate difference
        clock_gettime(CLOCK_MONOTONIC, &end);
        metrics->writeTimeMS += (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1e6;
        
        // 3. Write entries based on the batch size provided in metrics
        for (int i = 0; i < metrics->batch; i++) {
            rwlog_entry_t entry;

            // Output text to the msg field of the entry
            snprintf(entry.msg, RWLOG_MSG_MAX, "Writer %d entry %d", metrics->id, metrics->appendedCount + i);

            // If appending fails, break out of the loop early
            if (rwlog_append(&entry) != 0) {
                break;
            }
        }

        // 3. (cont) Update appended count based on the batch size
        metrics->appendedCount += metrics->batch;

        // 4. Release write lock
        rwlog_end_write();

        // 5. Sleep for wr_us microseconds (usleep is used for microseconds apparently)
        usleep(metrics->wr_us);
    }
}

// Called by each reader thread
static void *ReaderThread(void *arg) {
    rwlog_entry_t entries[rwlog_get_state()->capacity];
    readerMetrics *metrics = (readerMetrics *)arg;
    struct timespec start, end;

    while (!stop_flag) {
        // 1. Record start time
        clock_gettime(CLOCK_MONOTONIC, &start);

        // 1. (cont) Stop if we can't even read in the first place
        if (rwlog_begin_read() != 0) {
            break;
        }

        // 2. Take a snapshot
        ssize_t n = rwlog_snapshot(entries, rwlog_get_state()->capacity);

        // 3. Monotoncinity check, print something if it's wrong
        for (ssize_t i = 1; i < n; i++) {
            if (entries[i].seq < entries[i-1].seq) {
                fprintf(stderr, "Reader %d: Non-monotonic snapshot detected at entry %ld (seq %lu < %lu)\n", metrics->id, i, entries[i].seq, entries[i-1].seq);
            }
        }

        // 4. Release read lock
        rwlog_end_read();

        // 5. Record end time and calculate difference
        clock_gettime(CLOCK_MONOTONIC, &end);
        metrics->readTimeMS += (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1e6;
        metrics->snapshotsCount++;

        // 6. Sleep for rd_us microseconds (usleep is used for microseconds apparently)
        usleep(metrics->rd_us);
    }
}

// Entry point of this huge program, handles initialization, thread creation, and final reporting
int main(int argc, char **argv) 
{
    struct config cfg;
    parse_args(argc, argv, &cfg);

    printf("capacity=%d readers=%d writers=%d batch=%d seconds=%d rd_us=%d wr_us=%d dump=%d\n",
           cfg.capacity, cfg.readers, cfg.writers, cfg.writer_batch,
           cfg.seconds, cfg.rd_us, cfg.wr_us, cfg.dump_csv);

    /* your remaining initialization here... */
	
	/* Initialize the shm-backed monitor */
    if (rwlog_create(cfg.capacity) != 0) {
        fprintf(stderr, "There was an error creating the r/w log.\n");
        exit(1);
    }
 
    /* Install SIGINT and start wall-clock timer thread */
    signal(SIGINT, SigInt);

    pthread_t timerThread;
    pthread_create(&timerThread, NULL, TimerThread, &cfg.seconds);
	
	/* Create the writer threads */
	pthread_t * writerThreads = calloc(cfg.writers, sizeof(pthread_t));
	writerMetrics * wMetrics = calloc(cfg.writers, sizeof(writerMetrics));

    for (int i = 0; i < cfg.writers; i++) {
        wMetrics[i].id = i;
        wMetrics[i].batch = cfg.writer_batch;
        wMetrics[i].wr_us = cfg.wr_us;
        wMetrics[i].writeTimeMS = 0.0;
        wMetrics[i].appendedCount = 0;

        pthread_create(&writerThreads[i], NULL, WriterThread, &wMetrics[i]);
    }
	 
	/* Create the reader threads */
    pthread_t *readerThreads = calloc(cfg.readers, sizeof(pthread_t));
	readerMetrics *rMetrics = calloc(cfg.readers, sizeof(readerMetrics));

    for (int i = 0; i < cfg.readers; i++) {
        rMetrics[i].id = i;
        rMetrics[i].rd_us = cfg.rd_us;
        rMetrics[i].readTimeMS = 0.0;
        rMetrics[i].snapshotsCount = 0;

        pthread_create(&readerThreads[i], NULL, ReaderThread, &rMetrics[i]);
    }
	 
	/* Join reader/writer threads and timer thread */
    for (int i = 0; i < cfg.readers; i++) {
        pthread_join(readerThreads[i], NULL);
    }

    for (int i = 0; i < cfg.writers; i++) {
        pthread_join(writerThreads[i], NULL);
    }

    pthread_join(timerThread, NULL);
	 
	/* Optional: dump the final log to CSV for inspection/grading. */
    if (cfg.dump_csv) {
        FILE *file = fopen("log.csv", "w");

        if (file) {
            // Setup the CSV header
            fprintf(file, "seq,tid,ts_sec,ts_nsec,msg\n");

            // Buffer to hold all entries
            rwlog_entry_t ent[cfg.capacity];

            // Take a snapshot of the entire log
            rwlog_begin_read();

            // Get number of entries actually copied
            ssize_t n = rwlog_snapshot(ent, cfg.capacity);

            // Release read lock
            rwlog_end_read();

            // Write each entry to the CSV file
            for (ssize_t i = 0; i < n; i++) {
                fprintf(file, "%lu,%lu,%ld,%ld,%s\n", ent[i].seq, (unsigned long)ent[i].tid, ent[i].ts.tv_sec, ent[i].ts.tv_nsec, ent[i].msg);
            }

            // Close the file
            fclose(file);
        }
    }

	/* Compute averages only (avg reader wait, avg writer wait, avg throughput) */
    double averageReader = 0.0;
    double averageWriter = 0.0;
    int totalAppended = 0;
    int totalSnapshots = 0;

    for (int i = 0; i < cfg.readers; i++) {
        averageReader += rMetrics[i].readTimeMS;
        totalSnapshots += rMetrics[i].snapshotsCount;
    }

    for (int i = 0; i < cfg.writers; i++) {
        averageWriter += wMetrics[i].writeTimeMS;
        totalAppended += wMetrics[i].appendedCount;
    }

    printf("Average reader wait time: %.3f ms\n", averageReader / cfg.readers);
    printf("Average writer wait time: %.3f ms\n", averageWriter / cfg.writers);
    printf("Average throughput: %.3f ops/sec\n", (double)(totalAppended) / (double)cfg.seconds);

	/* Cleanup heap and monitor resources */
    free(readerThreads);
    free(writerThreads);

    free(rMetrics);
    free(wMetrics);
    
    rwlog_destroy();
	  
	return 0;  
}