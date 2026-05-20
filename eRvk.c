int factorial(int n);

int main()
{
    printf("Enter a number to find the factorial of: ");
    int num1;
    scanf("%d", &num1);
    int result = factorial(num1);
    printf("Result: %d ", result);

}
int factorial(int n)
{
    if(n>=1)
    {
        return n * factorial(n-1);

    }
    else
    {
        return 1;
    }
}