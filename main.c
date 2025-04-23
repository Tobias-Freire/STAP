#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>
#include <unistd.h>

#define NUM_CHAIRS 3
#define DEFAULT_NUM_STUDENTS 5

int isNumber(char number[]);
int string2number(char number[]);
int isWaiting( int student_id );
void* ta( void* arg );
void* student( void* student_id );

sem_t students_semaphore;
sem_t ta_semaphore;
pthread_mutex_t mutex_thread;

int waiting_room_chairs[NUM_CHAIRS];
int num_students_waiting = 0;
int next_seat_position = 0;
int next_teaching_position = 0;
int ta_sleeping_flag = 0; // 0 = TA está acordado, 1 = TA está dormindo

int main(int argc, char* argv[]) {
    int i;
    int student_num;

    if (argc > 1) {
        student_num = string2number(argv[1]);
        if (student_num == -1) {
            printf("Número de estudantes inválido!\n");
            return 1;
        }
    } else {
        student_num = DEFAULT_NUM_STUDENTS;
    }

    int student_ids[student_num];
    pthread_t students[student_num];
    pthread_t ta_thread;

    sem_init(&students_semaphore, 0, 0); 
    sem_init(&ta_semaphore, 0, 1); 

    pthread_mutex_init(&mutex_thread, NULL);
    pthread_create(&ta_thread, NULL, ta, NULL);
    for (i = 0; i < student_num; i++) {
        int* student_id = malloc(sizeof(int));
        if (student_id == NULL) {
            perror("Erro ao alocar memória");
            exit(1);
        }
        *student_id = i + 1;
        pthread_create(&students[i], NULL, student, (void*) student_id);
    }

    pthread_join(ta_thread, NULL);
    for (i = 0; i < student_num; i++) {
        pthread_join(students[i], NULL);
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

int isWaiting( int student_id ) {
    for(int i = 0; i < 3; i++) {
        if ( waiting_room_chairs[i] == student_id ) { return 1; } 
    }
    return 0; 
}

void* ta( void* arg ) {
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

void* student( void* student_id) {
    int id_student = *(int*)student_id;
    free(student_id);

    while(1) {
        if ( isWaiting( id_student ) == 1 ) { continue; }

        int time = rand() % 5; // Tempo de espera aleatório entre 0 e 4
        printf("Estudante %d está programando por %d segundos...\n", id_student, time);
        sleep(time);

        pthread_mutex_lock(&mutex_thread); 

        if ( num_students_waiting < NUM_CHAIRS) {
            waiting_room_chairs[next_seat_position] = id_student;
            num_students_waiting++;

            printf("Estudante %d entrou na sala de espera!\n", id_student);
            printf("Número de estudantes esperando: %d\n\n", num_students_waiting);

            next_seat_position = (next_seat_position + 1) % NUM_CHAIRS;
            pthread_mutex_unlock(&mutex_thread);

            // Acorda o TA
            sem_post(&students_semaphore);
            sem_wait(&ta_semaphore); 
        } else {
            pthread_mutex_unlock(&mutex_thread);
            printf("Estudante %d não conseguiu entrar na sala de espera!\n\n", id_student);
        }
    }
}