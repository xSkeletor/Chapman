# Identifying Information
Name: Dennis Fomichev
Student ID: 2470131
Email: fomichev@chapman.edu
Course: CPSC 380-01
Project: Programming Assignment 6 - Virtual Memory Manager

# Source files
vmmgr.c

# Known Errors
N/A

# References
man7.org — Linux manual page
linux.die.net - Another Linux manual page

# Building/running Instructions
To build using gcc in Linux: gcc vmmgr.c -o vmmgr
To run: ./vmmgr addresses.txt
NOTE: you need to have a BACKING_STORE.bin file created in the same directory.
Also, in order to use the FIFO page-replacement policy, you need to change line 23 from 256 to 128.

# Sample Output (using the default addresses.txt file provided)
./vmmgr addresses.txt
Logical Address: 16916, Physical Address: 20, Value: 0
Logical Address: 62493, Physical Address: 285, Value: 0
Logical Address: 30198, Physical Address: 758, Value: 29
Logical Address: 53683, Physical Address: 947, Value: 108
Logical Address: 40185, Physical Address: 1273, Value: 0
... (the majority of these middle entries have been cutoff so that it does not take up too much space in this sample output section)
Logical Address: 31260, Physical Address: 23324, Value: 0
Logical Address: 17071, Physical Address: 175, Value: -85
Logical Address: 8940, Physical Address: 46572, Value: 0
Logical Address: 9929, Physical Address: 44745, Value: 0
Logical Address: 45563, Physical Address: 46075, Value: 126
Logical Address: 12107, Physical Address: 2635, Value: -46
Page Fault Rate = 0.244
TLB Hit Rate = 0.055