# Identifying Information
Name: Dennis Fomichev
Student ID: 2470131
Email: fomichev@chapman.edu
Course: CPSC 380-01
Project: Programming Assignment 3 - Reader-Writer Synchronization

# Source files
rw_log.h
rw_log.c
rw_main.c

# Known Errors
N/A

# References
man7.org — Linux manual page
linux.die.net - Another Linux manual page

# Building/running Instructions
To build using gcc in Linux: gcc rw_main.c rw_log.c -o rw_main
To run: ./rw_log --capacity <N> --readers <R> --writers <W> --writer-batch <B> --seconds <T> --rd-us <microseconds> --wr-us <microseconds> --dump

-c, --capacity <N>, Log capacity (default 1024)
-r, --readers <N>,  Number of reader threads (default 6)
-w, --writers <N>, Number of writer threads (default 4)
-b, --writer-batch <N>, Entries written per writer section (default 2)
-s, --seconds <N>, Total run time (default 10)
-R, --rd-us <usec>, Reader sleep between operations (default 2000)
-W, --wr-us <usec>, Writer sleep between operations (default 3000)
-d, --dump, Dump final log to log.csv
-h, --help, Show this help message

# Sample Output
./rw_main -c 1024 -r 5 -w 10 -b 1 -s 20 -R 500 -W 500 -d
capacity=1024 readers=5 writers=10 batch=1 seconds=20 rd_us=500 wr_us=500 dump=1
Average reader wait time: 370.326 ms
Average writer wait time: 132.007 ms
Average throughput: 16071.300 ops/sec