# Identifying Information
Name: Dennis Fomichev
Student ID: 2470131
Email: fomichev@chapman.edu
Course: CPSC 380-01
Project: Programming Assignment 5 - Contiguous Memory Allocation

# Source files
allocator.c
sim.txt

# Known Errors
N/A

# References
man7.org — Linux manual page
linux.die.net - Another Linux manual page
I also used some AI sources to help me design the formatting of the memory map, specifically regarding the spacing of min/max values

# Building/running Instructions
To build using gcc in Linux: gcc allocator.c -o allocator
To launch the simulator: ./allocator <MAX MEMORY SIZE>
Then, once in the terminal, you can run the following commands:
RQ <process> <size> <F|B|W> - Request a contiguous block of memory using First Fit, Best Fit, or Worst Fit.
RL <process> - Release memory allocated to a process.
C - Compact all unused holes into one region.
STAT [-v] - Display a report of allocated and free regions.
X - Exit the program.
SIM <filename> - Run commands from an input file.

# Sample Output (using an example from the instructions and simulation file)
./allocator 1024
allocator>SIM sim.txt
Allocated memory:
Process P0: Start = 0 KB, End = 40 KB, Size = 40 KB
Process P2: Start = 120 KB, End = 200 KB, Size = 80 KB

Free memory:
Hole 1: Start = 40 KB, End = 120 KB, Size = 80 KB
Hole 2: Start = 200 KB, End = 1024 KB, Size = 824 KB

Summary:
Total allocated: 120 KB
Total free: 904 KB
Largest hole: 824 KB
External fragmentation: 8.85%
Average hole size: 452.0 KB

Memory Map:
[##....####........................................]
^0                                                ^1024