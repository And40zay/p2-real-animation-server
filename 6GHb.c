#include <stdio.h>
#include <string.h>

// Exercise3.c — reverse a string entered by the user

int main(void) {
    char s[256];
    printf("Enter a string (max 255 chars): ");
    if (!fgets(s, sizeof(s), stdin)) return 0;
    // remove newline if present
    size_t len = strlen(s);
    if (len > 0 && s[len-1] == '\n') s[--len] = '\0';

    printf("Original: %s\n", s);
    printf("Reversed: ");
    for (int i = (int)len - 1; i >= 0; --i) putchar(s[i]);
    printf("\n");
    return 0;
}
