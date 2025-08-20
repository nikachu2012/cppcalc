extern int puts(char *);
extern int printf(const char *, ...);

int putInt(int i)
{
    return printf("%d", i);
}

int print(char *c)
{
    return printf("%s", c);
}

int printInt(long i)
{
    return printf("%ld", i);
}
