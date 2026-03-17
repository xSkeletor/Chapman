# Identifying Information
Name: Dennis Fomichev
Student ID: 2470131
Email: fomichev@chapman.edu
Course: CPSC 380-01
Project: Programming Assignment 4 - CPU Scheduling

# Source files
schedsim.cpp

# Known Errors
N/A

# References
man7.org — Linux manual page
linux.die.net - Another Linux manual page
cplusplus.com - C++ manual page, used for file reading
cppreference.com - Another C++ manual apge
I also used some AI sources to help me design the formatting/spacing of the Gantt Chart

# Building/running Instructions
To build using g++ in Linux: g++ schedsim.cpp -o schedsim
To run: ./schedsim [ -f | -s | -r | -p ]  [ -q <N> ] -i <FILE>
-f, --fcfs, Use FCFS scheduling (default policy)
-s, --sjf, Use SJF scheduling
-r, --rr, Use Round Robin scheduling
-p, --priority, Use Priority scheduling
-i, --input <FILE>, Input CSV filename
-q, --quantum <N>, Time quantum for RR (default 1)
-h, --help Show this help message

# Sample Output
./schedsim --fcfs -i workload.csv
=====FCFS SCHEDULING=====
Timeline (Gantt Chart):
0        5        8       16       22
|--------|--------|--------|--------|
|  P1    |  P2    |  P3    |  P4    |
|--------|--------|--------|--------|
PID   Arr   Burst  Start  Finish  Wait   Resp   Turn   
P1    0     5      0      5       0      0      5      
P2    1     3      5      8       4      4      7      
P3    2     8      8      16      6      6      14     
P4    3     6      16     22      13     13     19     
-------------------------------------
Avg Wait = 5.75
Avg Resp = 5.75
Avg Turn = 11.25
Throughput = 0.18 jobs/unit time
CPU Utilization = 100.00%