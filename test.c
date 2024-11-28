#include <stdio.h>
#include <stdlib.h>

int mystery[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

int func(int* var) { var[0]++; }

int main() {
    int* a = (int*)malloc(4);
    func(a);
    for (int i = 0; i < 10; i++) {
        a += mystery[i];
    }
    printf("Donezo");
    return 0;
}