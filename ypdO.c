#include <stdio.h>

typedef struct{
    char name[100];
    int age;
    Family mother;
    Family father;
    
} Family;

int main()
{
    Family Babu = {"Robert", 84};
    Family Nana = {"Julie", 83};
    Family Dad = {"Matthew", 58, Nana, Babu};
    Family Pete = {"Peter", 56, Nana, Babu};
    Family Mum = {"Agneta", 50}
    Family Jake = {"Jake", 26, Dad, Mum};
    Family Ellie = {"Ellie", 21, Dad, Mum};
    

}