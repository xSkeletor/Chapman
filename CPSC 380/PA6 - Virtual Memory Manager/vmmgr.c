/*
    Name: Dennis Fomichev
    Student ID: 2470131
    Email: fomichev@chapman.edu
    Course: CPSC 380-01
    Project: Programming Assignment 6 - Virtual Memory Manager

    This C source code file implements a simple Virtual Memory Manager that simulates
    address translation using a TLB and a page table. It handles page faults by loading
    pages from a backing store file into physical memory. The program uses the LRU
    algorithm for TLB replacement and FIFO for page replacement when the number of frames
    is limited to 128. The program reads logical addresses from the addresses.txt file and outputs
    the corresponding physical addresses and values stored at those addresses.
*/

#include <stdio.h>

// The main constant variables, from the assignment description
#define PAGE_SIZE 256
#define FRAME_SIZE 256
#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define NUM_FRAMES 256 // CHANGE THIS TO 128 FOR FIFO PAGE REPLACEMENT MODE!

// The physical memory will be simulated as a 2D array
signed char physicalMemory[NUM_FRAMES][FRAME_SIZE];

int pageTable[PAGE_TABLE_SIZE]; // -1 means its not in memory
int frameAge[NUM_FRAMES]; // Used for FIFO to keep track of frame ages

// Structure for TLB entries
typedef struct {
    int page;
    int frame;
    int last_used; // Used with the LRU replacement
} tlb_entry;

// TLB array
tlb_entry TLB[TLB_SIZE];

// Global variables for stats
int freeFrame = 0;
int timeCounter = 0;
int tlbHits = 0;
int totalAddresses = 0;
int pageFaults = 0;

// Searches the TLB for a page number, returns frame number if found, -1 otherwise
int SearchTLB(int pageNumber) {
    // Loop through TLB entries
    for (int i = 0; i < TLB_SIZE; i++) {
        // Check if the page number matches, return the frame if it does
        if (TLB[i].page == pageNumber) {
            TLB[i].last_used = timeCounter++;

            return TLB[i].frame;
        }
    }

    return -1; // Not in the TLB yet
}

// FIFO page replacement algorithm
int FIFO() {
    int oldest = 0;
    int oldestAge = frameAge[0];

    // Find the frame with the oldest age
    for (int i = 1; i < NUM_FRAMES; i++) {
        if (frameAge[i] < oldestAge) {
            oldestAge = frameAge[i];
            oldest = i;
        }
    }

    // Return the frame number to be replaced based on oldest age
    return oldest; 
}

// Adds a new page/frame to the TLB using LRU replacement
void AddToTLB(int pageNumber, int frameNumber) {
    // Try to find an empty slot first
    for (int i = 0; i < TLB_SIZE; i++) {
        if (TLB[i].page == -1) {
            TLB[i].page = pageNumber;
            TLB[i].frame = frameNumber;
            TLB[i].last_used = timeCounter++;

            return;
        }
    }

    // If no empty slot, find the LRU entry and replace it
    int lruIndex = 0;
    int lruTime = TLB[0].last_used;

    // Find the least recently used entry
    for (int i = 1; i < TLB_SIZE; i++) {
        if (TLB[i].last_used < lruTime) {
            lruTime = TLB[i].last_used;

            lruIndex = i;
        }
    }
    
    // Replace the LRU entry
    TLB[lruIndex].page = pageNumber;
    TLB[lruIndex].frame = frameNumber;
    TLB[lruIndex].last_used = timeCounter++;
}

// Invalidate TLB entries that map to a specific frame number
void InvalidateTLBFrame(int frameNumber) {
    // Loop through TLB entries
    for (int i = 0; i < TLB_SIZE; i++) {
        // If the frame matches, invalidate the entry
        if (TLB[i].frame == frameNumber) {
            TLB[i].page = -1;

            return;
        }
    }
}

int main(int argc, char *argv[]) {
    // Check if user gave the addresses.txt file
    if (argc != 2) {
        printf("Usage: %s addresses.txt\n", argv[0]);
        
        return 1;
    }

    // Check if the addresses file can be opened
    FILE *addressFile = fopen(argv[1], "r");
    if (!addressFile) {
        printf("Error: Unable to open the addresses.txt file\n");
        
        return 1;
    }

    // Check if the BACKING_STORE.bin file can be opened
    FILE *backingStoreFile = fopen("BACKING_STORE.bin", "rb");
    if (!backingStoreFile) {
        printf("Error: Unable to open the BACKING_STORE.bin file\n");
        
        return 1;
    }

    // Initialize page table entries to -1 (nothing is in memory yet)
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        pageTable[i] = -1; 
    }

    // Initialize TLB entries to -1
    for (int i = 0; i < TLB_SIZE; i++) {
        TLB[i].page = -1;
    }

    // Intilaize all the frame ages to 0 (FIFO)
    for (int i = 0; i < NUM_FRAMES; i++) {
        frameAge[i] = 0;
    }

    int logicalAddress;

    // Read each logical address from the file, store it temporarily in logicalAddress
    while (fscanf(addressFile, "%d", &logicalAddress) == 1) {
        totalAddresses++;

        // Extract page number and offset
        int mask = logicalAddress & 0xFFFF;
        int pageNumber = (mask >> 8) & 0xFF;
        int offset = mask & 0xFF;

        // TLB Lookup
        int frameNumber = SearchTLB(pageNumber);

        if (frameNumber != -1) { // TLB hit
            tlbHits++;
        } else { // TLB miss
            // Checking the page table
            frameNumber = pageTable[pageNumber];

            if (frameNumber == -1) {
                // Page fault, need to load from backing store
                pageFaults++;

                // If no free frames, need to evict a page (FIFO)
                if (freeFrame < NUM_FRAMES) {
                    frameNumber = freeFrame;

                    freeFrame++;
                } else { // Evict a page using FIFO
                    int selectedFrame = FIFO();

                    // Find which page is currently using the new selected frame
                    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
                        if (pageTable[i] == selectedFrame) {
                            pageTable[i] = -1; // Invalidate that page table entry

                            break;
                        }
                    }

                    // Invalidate any TLB entry that maps to the new selected frame
                    InvalidateTLBFrame(selectedFrame);

                    // Use the new selected frame for the new page
                    frameNumber = selectedFrame;
                }

                // Read the page from backing store into physical memory
                fseek(backingStoreFile, pageNumber * PAGE_SIZE, SEEK_SET);
                fread(physicalMemory[frameNumber], sizeof(signed char), PAGE_SIZE, backingStoreFile);

                // Update the page table
                pageTable[pageNumber] = frameNumber;
            }

            // Update the TLB with the new page/frame mapping
            AddToTLB(pageNumber, frameNumber);
        }

        // Update age for FIFO
        frameAge[frameNumber] = timeCounter++;

        // Compute the physical address and retrieve the value
        int physicalAddress = frameNumber * FRAME_SIZE + offset;
        signed char value = physicalMemory[frameNumber][offset];

        // Print out the final result
        printf("Logical Address: %d, Physical Address: %d, Value: %d\n", logicalAddress, physicalAddress, value);
    }

    // Final statistics
    printf("Page Fault Rate = %.3f\n", (double)pageFaults / totalAddresses);
    printf("TLB Hit Rate = %.3f\n", (double)tlbHits / totalAddresses);
    
    // Cleanup (close the opened files)
    fclose(addressFile);
    fclose(backingStoreFile);

    return 0;
}