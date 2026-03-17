# Identifying Information
Name: Dennis Fomichev
Student ID: 2470131
Email: fomichev@chapman.edu
Course: CPSC 380-01
Project: Programming Assignment 2 - Thread Synchronization

# Source files
prodcon.c
buffer.h

# Known Errors
N/A

# References
man7.org — Linux manual page

# Building/running Instructions
To build using gcc in Linux: gcc prodcon.c buffer.h -o prodcon
To run: ./prodcon <delay> <#producer threads> <#consumer threads>

# Sample Output (to make it short for this file, I used 0.1 delay and 1 producer/consumer)
./prodcon 0.1 1 1
Producer generated item with checksum 51088.
Producer generated item with checksum 20977.
Consumer read item with checksum 51088.
Consumer read item with checksum 20977.
Producer generated item with checksum 62607.
Producer generated item with checksum 38528.
Producer generated item with checksum 6159.
Consumer read item with checksum 62607.