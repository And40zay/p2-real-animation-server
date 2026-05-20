#include <stdio.h>

typedef struct MyFamily{
    char name[100];
    int age;
    struct MyFamily *mother;
    struct MyFamily *father;
    
}Family;

int main()
{
    Family Babu = {"Robert", 84, NULL, NULL};
    Family Nana = {"Julie", 83, NULL, NULL};
    Family Dad = {"Matthew", 58, &Nana, &Babu};
    Family Pete = {"Peter", 56, &Nana, &Babu};
    Family Mum = {"Agneta", 50, NULL, NULL};
    Family Jake = {"Jake", 26, &Dad, &Mum};
    Family Ellie = {"Ellie", 21, &Dad, &Mum};

    printf(" %s", Dad.father-> name);


}