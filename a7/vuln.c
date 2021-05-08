#include <unistd.h>
#include <stdio.h>
#include <string.h>

struct str_struct {
    int len;
    char* str;
};

char _str1[32] = "Welcome! Please enter a name:\n";
char _str2[12] = "Good luck, ";
char _str3[44] = "The secret phrase is \"squeamish ossifrage\" ";

struct str_struct str1 = {30, _str1};
struct str_struct str2 = {11, _str2};
struct str_struct str3 = {43, _str3};


void print(struct str_struct str) {
    write(1, str.str, str.len);
}

void proof() {
    print(str3);
}

int main() {
    char buf[128];
    print(str1);
    read(0, buf, 256);
    print(str2);
    write(1, buf, strlen(buf));
    return 0;
}