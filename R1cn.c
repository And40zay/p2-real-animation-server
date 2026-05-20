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
    Family Jake = {"Jake", 26, &Mum, &Dad};
    Family Ellie = {"Ellie", 21, &Mum, &Dad};

    Family people[] = {Babu, Nana, Dad, Pete, Mum, Jake, Ellie};

    int size = sizeOf(people)/sizeOf(people[0]);

    for(int i = 0; i<size; i++)
    {
       printf("Name: %s\nAge: %d", people[i].name, people[i].age);
    }
    return 0;

}