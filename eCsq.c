#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== Understanding malloc sizes ===\n\n");
    
    // What is sizeof(int) on THIS system?
    printf("On this system:\n");
    printf("  sizeof(int) = %zu bytes\n", sizeof(int));
    printf("  sizeof(char) = %zu bytes\n", sizeof(char));
    printf("  sizeof(double) = %zu bytes\n\n", sizeof(double));
    
    // WRONG: Thinking in "elements" not "bytes"
    int *wrong_ptr = malloc(4);
    printf("WRONG: malloc(4)\n");
    printf("  Asked for: 4 bytes\n");
    printf("  Need for 4 ints: %zu bytes\n", 4 * sizeof(int));
    printf("  Result: Only fits %d ints!\n\n", 4 / sizeof(int));
    
    // RIGHT: Calculating bytes correctly
    int *right_ptr = malloc(4 * sizeof(int));
    printf("RIGHT: malloc(4 * sizeof(int))\n");
    printf("  Asked for: %zu bytes\n", 4 * sizeof(int));
    printf("  That fits: %d ints perfectly!\n\n", 4);
    
    // Clean up
    free(wrong_ptr);
    free(right_ptr);
    
    return 0;
}