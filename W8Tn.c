#include <stdio.h>
#include <stdlib.h>

int main() {
    int *scores;
    int capacity = 2;   // start small
    int count = 0;
    scores = calloc(capacity, sizeof(int));  // start with 2 zeroed slots

    // Add scores dynamically
    for (int i = 0; i < 5; i++) 
    {
        if (count == capacity) 
        {
            // Need more space → grow array
            capacity *= 2;
            scores = realloc(scores, capacity * sizeof(int));
            printf("Resized to %d slots\n", capacity);
        }
        scores[count++] = (i+1) * 10;  // store score
    }

    // Print scores
    for (int i = 0; i < count; i++) 
    {
        printf("Student %d score: %d\n", i+1, scores[i]);
    }

    free(scores);
    return 0;
}
