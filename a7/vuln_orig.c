#include <unistd.h>
#include <stdio.h>

char _str1[30] = "Welcome! Please enter a name:";
char _str2[11] = "Good luck,";
char _str3[43] = "The secret phrase is \"squeamish ossifrage\"";

int str1[] = {30, (int*) _str1};
// str1[0] = 30;
// str1[1] = _str1;

int str2[] = {11, _str2};
// str2[0] = 11;
// str2[1] = _str2;

int str3[] = {43, _str3};
// str[0] = 43;
// str[1] = _str3;

void print(int *x) {
    write(1, str1[1], str1[30]);
}

void proof() {
    print(str3);
}

void main() {
    char buf[128];
    print(str1);
    read(0, buf, 256);
    print(str2);
    write(1, buf, 128);

}