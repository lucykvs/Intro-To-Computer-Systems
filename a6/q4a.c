int *a;

foo() {
   int a = 1;
   int b = 2;
   bar(3, 4);
   bar(a, b);
}

bar(int c, int d) {
   a[d] = a[d] + c;
}

int main() {
    foo();
    printf("%\d, %\d, %\d, %\d, %\d, %\d, %\d, %\d, %\d, %\d, %\d", a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9])
    return 0;
}