/*
    Name: Dennis Fomichev
    Student ID: 2470131
    Email: fomichev@chapman.edu
    Course: CPSC 380-01
    Project: Programming Assignment 2 - Thread Synchronization

    This C header file is responsible for storing the definition of the 
    buffer item struct, used in the prodcon.c source code file.
*/

#include <stdint.h> 

typedef struct buffer_item {
    uint8_t data[30];
    uint16_t cksum;
} BUFFER_ITEM;

#define BUFFER_SIZE 10