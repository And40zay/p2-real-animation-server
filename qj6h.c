#include <stdio.h>

double circle_area(double params[], int count)
{
    if(count<1)
    {
        printf("Error, circle needs 1 parameter");
        return 0;   
    }
    const int PI = 3.14;
    double radius = params[0];
    return radius * radius * PI;

}
double rectangle_area(double params[], int count)
{
    if(count<2)
    {
        printf("Error, rectangle needs 2 params");
        return 0;
    }
    double length = params[0];
    double height = params[1];

    return length*height;
}
double triangle_area(double params[], int count)
{
    if(count<2)
    {
        printf("Error, Triangle needs base and height");
        return 0;
    }
    double base = params[0];
    double height = params[1];

    return 0.5 * base * height;
}
double sqare_area(double params[], int count)
{
    if(count<1)
    {
        printf("error: Square needs at least a length variable");
        return 0;
    }
    double length = params[0];
    
    return length * length;
}

double 

int main()
{
   





    return 0;
}