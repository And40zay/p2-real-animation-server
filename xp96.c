#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to print memory details
void print_memory_info(const char* label, void* ptr, size_t size) 
{
    printf("\n=== %s ===\n", label);
    printf("Pointer address: %p\n", ptr);
    printf("Size requested: %lu bytes\n", size);
    printf("Size of pointer: %lu bytes\n", sizeof(ptr));
    
    if (ptr != NULL) 
    {
        // Show first few bytes (for debugging)
        printf("First 16 bytes of memory: ");
        for (int i = 0; i < 16 && i < size; i++) 
        {
            printf("%02X ", ((unsigned char*)ptr)[i]);
        }
        printf("\n");
    }
}

int main() {
    printf("=== DYNAMIC MEMORY ALLOCATION DEBUG EXAMPLE ===\n");
    printf("sizeof(int) = %lu bytes\n", sizeof(int));
    printf("sizeof(char) = %lu bytes\n", sizeof(char));
    printf("sizeof(double) = %lu bytes\n", sizeof(double));
    
    // ==================== EXAMPLE 1: malloc ====================
    printf("\n\n--- Example 1: malloc() ---");
    
    // Allocate memory for 5 integers
    // Parameter size: 5 * sizeof(int) = 5 * 4 = 20 bytes (on most systems)
    size_t malloc_size = 5 * sizeof(int);
    printf("\nCalling: malloc(5 * sizeof(int))");
    printf("\nParameter size = %lu bytes\n", malloc_size);
    
    int* malloc_ptr = (int*)malloc(malloc_size);
    print_memory_info("After malloc", malloc_ptr, malloc_size);
    
    // Fill with data to see what malloc gives us (uninitialized)
    printf("\nFilling malloc'd memory with values 1-5...");
    for (int i = 0; i < 5; i++) 
    {
        malloc_ptr[i] = i + 1;  // Values 1, 2, 3, 4, 5
    }
    
    printf("\nArray contents: ");
    for (int i = 0; i < 5; i++) 
    {
        printf("%d ", malloc_ptr[i]);
    }
    
    // ==================== EXAMPLE 2: calloc ====================
    printf("\n\n\n--- Example 2: calloc() ---");
    
    // Allocate memory for 5 integers, initialized to 0
    // Parameters: num = 5, size = sizeof(int)
    // Total = 5 * 4 = 20 bytes
    printf("\nCalling: calloc(5, sizeof(int))");
    printf("\nParameter num = 5, size = %lu bytes", sizeof(int));
    printf("\nTotal memory = 5 * %lu = %lu bytes\n", sizeof(int), 5 * sizeof(int));
    
    int* calloc_ptr = (int*)calloc(5, sizeof(int));
    print_memory_info("After calloc", calloc_ptr, 5 * sizeof(int));
    
    printf("\ncalloc array (all should be 0): ");
    for (int i = 0; i < 5; i++) 
    {
        printf("%d ", calloc_ptr[i]);
    }
    
    // ==================== EXAMPLE 3: realloc ====================
    printf("\n\n\n--- Example 3: realloc() ---");
    
    // Resize malloc array from 5 to 10 integers
    // Parameter: new_size = 10 * sizeof(int) = 40 bytes
    size_t realloc_size = 10 * sizeof(int);
    printf("\nCalling: realloc(malloc_ptr, 10 * sizeof(int))");
    printf("\nOld pointer: %p", malloc_ptr);
    printf("\nParameter new_size = %lu bytes\n", realloc_size);
    
    int* realloc_ptr = (int*)realloc(malloc_ptr, realloc_size);
    print_memory_info("After realloc", realloc_ptr, realloc_size);
    
    // Note: malloc_ptr is now INVALID! Use realloc_ptr instead
    printf("\nWARNING: Original malloc_ptr (%p) is now INVALID!", malloc_ptr);
    printf("\nMust use realloc_ptr (%p) instead", realloc_ptr);
    
    // Show data was preserved
    printf("\nFirst 5 elements (preserved from malloc): ");
    for (int i = 0; i < 5; i++) 
    {
        printf("%d ", realloc_ptr[i]);
    }
    
    // New memory is uninitialized
    printf("\nLast 5 elements (uninitialized, could be anything): ");
    for (int i = 5; i < 10; i++) 
    {
        printf("%d ", realloc_ptr[i]);
    }
    
    // ==================== EXAMPLE 4: String Example ====================
    printf("\n\n\n--- Example 4: String Manipulation ---");
    
    // Allocate string with malloc
    // strlen("Hello") = 5, but need +1 for null terminator = 6 bytes
    char* str = (char*)malloc(6 * sizeof(char));  // 6 bytes
    printf("\nAllocating string: malloc(6 * sizeof(char))");
    printf("\nParameter size = 6 * %lu = %lu bytes", sizeof(char), 6 * sizeof(char));
    
    strcpy(str, "Hello");
    print_memory_info("String after malloc & copy", str, 6);
    printf("String: %s\n", str);
    printf("String length: %lu, Buffer size: 6 bytes\n", strlen(str));
    
    // Realloc to hold longer string
    char* new_str = (char*)realloc(str, 14 * sizeof(char));  // "Hello World!" + null
    printf("\nReallocating: realloc(str, 14 * sizeof(char))");
    printf("\nParameter size = 14 * %lu = %lu bytes", sizeof(char), 14 * sizeof(char));
    
    strcpy(new_str, "Hello World!");
    print_memory_info("String after realloc & copy", new_str, 14);
    printf("New string: %s\n", new_str);
    printf("String length: %lu, Buffer size: 14 bytes\n", strlen(new_str));
    
    // ==================== EXAMPLE 5: Error Handling ====================
    printf("\n\n\n--- Example 5: Error Handling ---");
    
    // Try to allocate huge memory
    size_t huge_size = 1000000000000;  // 1 trillion bytes ~ 1TB
    printf("\nTrying to allocate %lu bytes...", huge_size);
    
    int* huge_ptr = (int*)malloc(huge_size);
    if (huge_ptr == NULL) 
    {
        printf("\nSUCCESS: malloc correctly returned NULL (out of memory)");
        printf("\nPointer value: %p", huge_ptr);
    }
    
    // ==================== CLEANUP ====================
    printf("\n\n\n--- Cleanup ---");
    
    // Always free allocated memory
    free(realloc_ptr);  // Frees the realloc'd memory
    free(calloc_ptr);   // Frees the calloc'd memory  
    free(new_str);      // Frees the string memory
    
    printf("\nAll memory freed. Good practice prevents memory leaks!");
    
    return 0;
}