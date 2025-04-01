#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>

int isNumber(char number[]) {
    int size = strlen(number);
    for (int i = 0; i < size; i++) {
        if (!isdigit(number[i])) {
            return 0; // Não é um número
        }
    }
    return 1; // É um número
}

int string2number(char number[]) {
    if (isNumber(number)) {
        return atoi(number); // Converte string para inteiro e o retorna
    } else {
        return -1; // Erro
    }
}

int main() {
    char number[5];
    printf("Enter a number: ");
    scanf("%4s", number);
    int result = string2number(number);
    if (result != -1) {
        printf("The number is: %d\n", result);
    } else {
        printf("Error converting string to number.\n");
    }
    return 0;
}