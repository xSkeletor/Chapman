#include <stdio.h>
#include <stdlib.h>

int requests[] = {2069, 1212, 2296, 2800, 544, 1618, 356, 1523, 4965, 3681};
const int maxCylinder = 5000;
const int numElements = 10;

int main(int argc, char *argv[]) {
    int totalMovement = 0;

    if (argc != 2) {
        printf("Usage: %s \n", argv[0]);

        return 1;
    }

    int initialPosition = atoi(argv[1]);

    if (initialPosition < 0 || initialPosition >= maxCylinder) {
        printf("Initial position must be between 0 and %d\n", maxCylinder - 1);

        return 1;
    }

    for (int i = 0; i < numElements; i++) {
        totalMovement += abs(requests[i] - initialPosition);
        initialPosition = requests[i];
    }

    printf("Total Head Movement: %d cylinders\n", totalMovement);

    return 0;
}