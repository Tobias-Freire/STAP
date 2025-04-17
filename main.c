#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>

#define NUM_CHAIRS 3
#define DEFAULT_NUM_STUDENTS 5

int isNumber(char number[]);
int string2number(char number[]);
void* ta();

sem_t students_semaphore;
sem_t ta_semaphore;
pthread_mutex_t mutex_thread;

int waiting_room_chairs[NUM_CHAIRS];
int num_students_waiting = 0;
int next_seat_position = 0;
int next_teaching_position = 0;
int ta_sleeping_flag = 0; // 0 = TA está acordado, 1 = TA está dormindo

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

int isNumber(char number[]) {
    int size = strlen(number);
    for (int i = 0; i < size; i++) {
        if (!isdigit(number[i])) {
            return 0; 
        }
    }
    return 1; 
}

int string2number(char number[]) {
    if (isNumber(number)) {
        return atoi(number); // Converte string para inteiro e o retorna
    } else {
        return -1; // Erro
    }
}

void* ta() {
    printf("Verificando se há estudantes...\n");

    while(1) {
        if (num_students_waiting > 0) {
            printf("Há estudante(s)!\n\n");

            ta_sleeping_flag = 0; // Acorda o TA
            sem_wait(&students_semaphore); // Espera por um estudante
            pthread_mutex_lock(&mutex_thread); // Bloqueia o mutex

            int teaching_time = rand() % 5; // Tempo de aula aleatório entre 0 e 4

            printf("TA está ajudando o estudante por %d segundos...\n", teaching_time);
            printf("Número de estudantes esperando: %d\n", num_students_waiting);
            sleep(teaching_time);
            pthread_mutex_unlock(&mutex_thread);
            sem_post(&ta_semaphore); // Libera o semáforo do estudante
            printf("Estudante %d terminou de ser atendido!\n\n", waiting_room_chairs[next_teaching_position]);
            
            waiting_room_chairs[next_teaching_position] = 0;
            next_teaching_position = (next_teaching_position + 1) % NUM_CHAIRS;
            num_students_waiting--;
            
            printf("Estudante %d será atendido.\n\n", waiting_room_chairs[next_teaching_position]);
            
        } else {
            if (ta_sleeping_flag == 0) {
                printf("Não há estudantes!\n");
                ta_sleeping_flag = 1; // Coloca o TA para dormir
                printf("TA está dormindo...\n"); 
            }
        }
    }
}