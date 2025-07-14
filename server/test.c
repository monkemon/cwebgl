#include <unistd.h>
#include <string.h>
#include <stdio.h>

void print_binary(int n) {
    for (int i = sizeof(n) * 8 - 1; i >= 0; --i) {
        printf("%d", (n >> i) & 1);
    }
    printf("\n");
}

int main(int argc, char** args)
{
    char buff[128];
    char* msg = "\r\n\r\nGeneral kenobi omg\n";

    int* asint;

    for (int i = 0; i < 4; i++)
    {
        asint = (int*) msg + i;
        
        char b[5];
        strncpy(b, msg + i, 4);
        printf("range %d-%d, text [%s], int %d\n", i, i+4, b, *asint);
        //print_binary(*asint);
        for (int j = 0; j < 4; j++)
        {
            printf("byte %d > %d\t%x\n", j, msg[i + j], msg[i+j]);
        }
        printf("\n");
    }
}