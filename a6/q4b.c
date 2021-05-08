#include <stdio.h>

int x[] = {1, 2, 3, -1, -2, 0, 184, 340057058};
int y[] = {0, 0, 0, 0, 0, 0, 0, 0};

int f(int z) {
    int n = 0x80000000;
    int j = 0;

    while(z != 0) {
        if ((z & n) != 0)
            j++;
        z = z*2;
    }

    return j;
}


int main() {
    for (int i = 8; i > 0; i--) {
        y[i-1] = f(x[i-1]);
    }
    
    for(int i = 0; i < 8; i++)
        printf("%d\n", x[i]);
    for(int i = 0; i < 8; i++)
        printf("%d\n", y[i]);
        
    return 0;
}