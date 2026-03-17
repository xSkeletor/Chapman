/*
    Name: Dennis Fomichev
    Student ID: 2470131
    Email: fomichev@chapman.edu
    Course: CPSC 380-01
    Project: Programming Assignment 5 - Contiguous Memory Allocation

    This C source code file implements a contiguous memory allocation simulator.
    It allows users to allocate and release memory for processes using different strategies
    (first-fit, best-fit, worst-fit), compact memory, and view memory statistics.
    The program uses linked lists to track allocated memory blocks and free holes.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Linked list node structure
typedef struct Node {
    char process[16];
    int start;
    int size;
    struct Node *next;
} Node;

Node *allocatedList = NULL; // List of the allocated nodes (full memory)
Node *holeList = NULL; // List of the holes (free memory)
int max = 0; // Maximum memory size, to be set in main

// Returns a new node with the given parameters
Node *CreateNode(const char *proc, int start, int size) {
    Node *n = malloc(sizeof(Node));
    strcpy(n->process, proc);
    n->start = start;
    n->size = size;
    n->next = NULL;

    return n;
}

// Inserts a hole into the hole list in sorted order
void InsertHole(Node *hole) {
    // If holeList is empty or hole should be first before the already existing holes
    if (!holeList || hole->start < holeList->start) {
        hole->next = holeList;
        holeList = hole;
    } else { // Otherwise, find the correct position to insert the hole
        // Get the current node, traverse until we find the correct position and change the values in order to insert
        Node *cur = holeList;

        while (cur->next && cur->next->start < hole->start) {
            cur = cur->next;
        }

        hole->next = cur->next;
        cur->next = hole;
    }
}

// Merges adjacent holes in the hole list
void MergeHoles() {
    Node *cur = holeList;

    // While there are still holes to check
    while (cur && cur->next) {
        // If the current hole is adjacent to the next hole, merge them
        if (cur->start + cur->size == cur->next->start) {
            cur->size += cur->next->size;

            // Create a temporary variable to store the node it needs to delete
            Node *toDelete = cur->next;
            cur->next = cur->next->next;

            free(toDelete);
        } else { // Otherwise, move to the next hole
            cur = cur->next;
        }
    }
}

// Allocates memory for a process using the specified strategy
void RequestMemory(char *proc, int size, char strat) {
    if (size <= 0) {
        printf("Error: invalid size.\n");

        return;
    }

    // Check if the process already exists in the allocated list
    for (Node *n = allocatedList; n; n = n->next) {
        if (strcmp(n->process, proc) == 0) {
            printf("Error: process %s already allocated.\n", proc);

            return;
        }
    }

    Node *chosen = NULL; // The chosen hole for allocation
    Node *prev = NULL; // The previous node in the hole list
    Node *prevChosen = NULL; // The previous node of the chosen hole
    Node *cur = holeList; // Current node in the loop

    if (toupper(strat) == 'F') { // First-fit strategy
        while (cur) { // Go through all nodes
            // If we find the first node that fits the size, chose it and break out of loop
            if (cur->size >= size) { 
                chosen = cur;
                prevChosen = prev;

                break;
            }

            // Move to the next node
            prev = cur;
            cur = cur->next;
        }
    } else if (toupper(strat) == 'B') { // Best-fit strategy
        int best = 1e9; // Initialize best size to a large number by default
        
        while (cur) { // Go through all nodes
            // If we find the first node that fits the size AND is smaller than the best size found so far, select it as the best for now
            if (cur->size >= size && cur->size < best) { 
                best = cur->size;
                chosen = cur;
                prevChosen = prev;
            }

            // Move to the next node
            prev = cur;
            cur = cur->next;
        }
    } else if (toupper(strat) == 'W') { // Worst-fit strategy
        int worst = -1; // Initialize worst size to a small number by default, can't be smaller than 0

        while (cur) {
            // If we find the first node that fits the size AND is larger than the worst size found so far, select it as the worst for now
            if (cur->size >= size && cur->size > worst) {
                worst = cur->size;
                chosen = cur;
                prevChosen = prev;
            }

            // Move to the next node
            prev = cur;
            cur = cur->next;
        }
    } else { // They entered an invalid strategy, maybe didn't capitalize it, etc
        printf("Error: invalid strategy.\n");

        return;
    }

    if (!chosen) { // If no suitable hole was found, probably no memory for it
        printf("Error: insufficient memory.\n");

        return;
    }

    // Since we found space for the new process, create a new node for it
    Node *newNode = CreateNode(proc, chosen->start, size);

    // Insert into allocatedList sorted by start address
    if (!allocatedList || allocatedList->start > newNode->start) {
        // If the list is empty or the new node should be first, insert it at the beginning
        newNode->next = allocatedList;
        allocatedList = newNode;
    } else { // Otherwise, find the correct position to insert the new node
        Node *cur = allocatedList;

        // Traverse the current list to find the correct position
        while (cur->next && cur->next->start < newNode->start) {
            cur = cur->next;
        }

        // Insert the new node after the current node, and update the overall list values
        newNode->next = cur->next;
        cur->next = newNode;
    }

    // Update the chosen hole's start and size
    chosen->start += size;
    chosen->size -= size;

    // If the chosen hole is now of size 0, remove it from the hole list
    if (chosen->size == 0) {
        if (prevChosen) { // If there is a previous node, update its next pointer
            prevChosen->next = chosen->next;
        } else { // Otherwise, remove the first node in the hole list
            holeList = chosen->next;
        }

        // Free the memory allocated for the empty hole
        free(chosen);
    }
}

// Releases memory allocated to a specified process
void ReleaseMemory(char *proc) {
    Node *prev = NULL;
    Node *cur = allocatedList;

    // Find the process in the allocated list
    while (cur && strcmp(cur->process, proc) != 0) {
        prev = cur;
        cur = cur->next;
    }

    if (!cur) { // If the process was not found
        printf("Error: process not found.\n");

        return;
    }

    // Create a new hole for the released memory
    Node *hole = CreateNode("HOLE", cur->start, cur->size);
    InsertHole(hole);
    MergeHoles();

    // Remove the process from the allocated list
    if (prev) { // If there is a previous node, update its next pointer
        prev->next = cur->next;
    } else { // Otherwise, remove the first node in the allocated list
        allocatedList = cur->next;
    }

    // Free the memory allocated for the released process
    free(cur);
}

// Compacts memory by moving all allocated blocks to the start
void Compact() {
    int offset = 0;

    // Move all allocated blocks to the start of memory
    for (Node *n = allocatedList; n; n = n->next) {
        n->start = offset;
        offset += n->size;
    }

    // Rebuild the hole list
    while (holeList) {
        Node *tmp = holeList;
        holeList = holeList->next;

        free(tmp);
    }

    // If there is remaining memory, create a new hole
    if (offset < max) {
        holeList = CreateNode("HOLE", offset, max - offset);
    }
}

// Printing out the fragmentation and utilization metrics, also can show the optional visual
void Stat(int visual) {
    // Allocated memory section
    printf("Allocated memory:\n");

    for (Node *n = allocatedList; n; n = n->next) {
        printf("Process %s: Start = %d KB, End = %d KB, Size = %d KB\n", n->process, n->start, n->start + n->size, n->size);
    }

    // Free memory section
    printf("\nFree memory:\n");

    int totalFree = 0;
    int largestHole = 0;
    int holes = 0;

    for (Node *h = holeList; h; h = h->next) {
        // Incrementing holes count in the print statement to make it a bit cleaner, usually don't like doing this though
        printf("Hole %d: Start = %d KB, End = %d KB, Size = %d KB\n", ++holes, h->start, h->start + h->size, h->size);

        totalFree += h->size;

        if (h->size > largestHole) {
            largestHole = h->size;
        }
    }

    // Summary calculations
    int totalAlloc = 0;

    for (Node *n = allocatedList; n; n = n->next) {
        totalAlloc += n->size;
    }

    // External fragmentation: (1-largest free block/total free memory)
    double frag = 100.0 * (1.0 - (double)largestHole / totalFree);
    double avgHole = (double)totalFree / holes;

    printf("\nSummary:\n");
    printf("Total allocated: %d KB\nTotal free: %d KB\nLargest hole: %d KB\n", totalAlloc, totalFree, largestHole);
    printf("External fragmentation: %.2f%%\nAverage hole size: %.1f KB\n", frag, avgHole);

    // The optional visual component, only if specified with the -v flag
    if (visual) {
        printf("\nMemory Map:\n[");

        // 50 characters will be used to represent the memory, 2% each bar
        for (int i = 0; i < 50; i++) {
            // Determine the position in memory that this character represents
            int pos = (i * max) / 50;
            int occupied = 0;

            for (Node *n = allocatedList; n; n = n->next) {
                // Check if the current position is within an allocated block
                if (pos >= n->start && pos < n->start + n->size) {
                    occupied = 1;
                }
            }

            printf("%c", occupied ? '#' : '.');
        }

        // Finish the visual bar
        printf("]\n^0");

        // Move to the last character of the bar:
        for (int i = 0; i < 48; i++) {
            printf(" ");
        }

        printf("^%d\n", max);
    }
}

// Simulates commands from a file, mostly copied from main for the specific command parsing
void Sim(char *file) {
    // Open the file
    FILE *f = fopen(file, "r");

    // Check if the file was opened successfully
    if (!f) {
        printf("Error: cannot open %s\n", file);

        return;
    }

    // Used for storing commands from the file
    char line[256];

    while (fgets(line, sizeof(line), f)) {
        // Trim leading/trailing whitespace from user input
        int len = strlen(line);
        while(len > 0 && isspace(line[len - 1])) {
            line[--len] = '\0';
        }

        while(*line && isspace(*line)) {
            (*line)++;
        }

        // If the user just pressed enter, prompt again
        if (strlen(line) == 0) {
            continue;
        }

        // Parse the command and its arguments
        char cmd[8], arg1[16], arg2[16], arg3[8];
        int n = sscanf(line, "%s %s %s %s", cmd, arg1, arg2, arg3);
        
        // Go through the possible commands and call the appropriate functions
        if (strcmp(cmd, "RQ") == 0 && n == 4) {
            RequestMemory(arg1, atoi(arg2), arg3[0]);
        } else if (strcmp(cmd, "RL") == 0 && n == 2) {
            ReleaseMemory(arg1);
        } else if (strcmp(cmd, "C") == 0) {
            Compact();
        } else if (strcmp(cmd, "STAT") == 0) {
            Stat((n == 2 && strcmp(arg1, "-v") == 0));
        } else if (strcmp(cmd, "X") == 0) {
            break;
        } else {
            printf("Invalid command: %s\n", line);
        }
    }  
    
    // Close the file once done with simuation commands
    fclose(f);
}

// Main function that handles user input and commands
int main(int argc, char* argv[]) {
    // Make sure user inputs a max memory value
    if (argc != 2) {
        printf("Usage: ./allocator <max>\n");

        return 1;
    }

    // Store max memory and create a hole that spans it
    max = atoi(argv[1]);
    holeList = CreateNode("HOLE", 0, max);

    // Storing what the user inputs
    char line[256];
    printf("allocator>");

    while (fgets(line, sizeof(line), stdin)) {
        // Trim leading/trailing whitespace from user input
        int len = strlen(line);
        while(len > 0 && isspace(line[len - 1])) {
            line[--len] = '\0';
        }

        while(*line && isspace(*line)) {
            (*line)++;
        }

        // If the user just pressed enter, prompt again
        if (strlen(line) == 0) {
            printf("allocator>");

            continue;
        }

        // Parse the command and its arguments
        char cmd[8], arg1[16], arg2[16], arg3[8];
        int n = sscanf(line, "%s %s %s %s", cmd, arg1, arg2, arg3);
        
        // Go through the possible commands and call the appropriate functions
        if (strcmp(cmd, "RQ") == 0 && n == 4) {
            RequestMemory(arg1, atoi(arg2), arg3[0]);
        } else if (strcmp(cmd, "RL") == 0 && n == 2) {
            ReleaseMemory(arg1);
        } else if (strcmp(cmd, "C") == 0) {
            Compact();
        } else if (strcmp(cmd, "STAT") == 0) {
            Stat((n == 2 && strcmp(arg1, "-v") == 0));
        } else if (strcmp(cmd, "SIM") == 0 && n == 2) {
            Sim(arg1);
        } else if (strcmp(cmd, "X") == 0) {
            break;
        } else {
            printf("Invalid command: %s\n", line);
        }

        printf("allocator>");
    }  

    return 0;
}