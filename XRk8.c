#include <stdio.h>
#include <stdlib.h>

int main() {
    // Imagine you need to store 3 numbers
    // Each number (int) needs 4 bytes of space
    
    printf("Each integer needs %d bytes\n", sizeof(int));
    
    // malloc(3 numbers × 4 bytes each) = 12 bytes total
    int* numbers = (int*)malloc(3 * sizeof(int));
    
    // Check if we got the memory
    if (numbers == NULL) 
    {
        printf("Oops! No memory available!\n");
        return 1;
    }
    
    printf("\nMemory allocated at address: %p\n", numbers);
    printf("Total bytes allocated: %d\n", 3 * sizeof(int));
    
    // Put numbers in our boxes
    numbers[0] = 10;  // First box
    numbers[1] = 20;  // Second box  
    numbers[2] = 30;  // Third box
    
    // Print what we stored
    printf("\nStored numbers:\n");
    printf("Box 0: %d\n", numbers[0]);
    printf("Box 1: %d\n", numbers[1]); 
    printf("Box 2: %d\n", numbers[2]);
    
    // ALWAYS clean up!
    free(numbers);
    
    return 0;
}