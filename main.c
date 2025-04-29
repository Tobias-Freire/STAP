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
int isWaiting(int student_id);
void* ta(void* arg);
void* student(void* student_id);

sem_t students_semaphore;
sem_t ta_semaphore;
pthread_mutex_t mutex_thread;

int waiting_room_chairs[NUM_CHAIRS];
int num_students_waiting = 0;
int next_seat_position = 0;
int next_teaching_position = 0;
int ta_sleeping_flag = 0;
int students_served = 0;
int total_students = 0;
int finished = 0;  //flag para indicar que todos os estudantes foram atendidos e finalizar o programa.

int main(int argc, char* argv[]) {
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

    total_students = student_num;

    pthread_t students[student_num];
    pthread_t ta_thread;

    sem_init(&students_semaphore, 0, 0); 
    sem_init(&ta_semaphore, 0, 1); 
    pthread_mutex_init(&mutex_thread, NULL);

    pthread_create(&ta_thread, NULL, ta, NULL);

    for (int i = 0; i < student_num; i++) {
        int* student_id = malloc(sizeof(int));
        *student_id = i + 1;
        pthread_create(&students[i], NULL, student, (void*) student_id);
    }

    pthread_join(ta_thread, NULL);

    for (int i = 0; i < student_num; i++) {
        pthread_join(students[i], NULL);
    }

    return 0;
}

int isNumber(char number[]) {
    int size = strlen(number);
    for (int i = 0; i < size; i++) {
        if (!isdigit(number[i])) return 0; 
    }
    return 1;
}

int string2number(char number[]) {
    return isNumber(number) ? atoi(number) : -1;
}

int isWaiting(int student_id) {
    for (int i = 0; i < NUM_CHAIRS; i++) {
        if (waiting_room_chairs[i] == student_id) return 1;
    }
    return 0;
}

void* ta(void* arg) {
    printf("[TA] Iniciando. Verificando se há estudantes...\n");

    while (students_served < total_students) {
        if (num_students_waiting > 0) {
            ta_sleeping_flag = 0;
            sem_wait(&students_semaphore);
            pthread_mutex_lock(&mutex_thread);

            int student_id = waiting_room_chairs[next_teaching_position];
            int teaching_time = (rand() % 5) + 1;
            printf("[TA] Vai ajudar o estudante %d por %d segundos...\n", student_id, teaching_time);
            sleep(teaching_time);

            printf("[TA] Estudante %d terminou de ser atendido.\n", student_id);
            waiting_room_chairs[next_teaching_position] = 0;
            next_teaching_position = (next_teaching_position + 1) % NUM_CHAIRS;
            num_students_waiting--;
            students_served++;

            pthread_mutex_unlock(&mutex_thread);
            sem_post(&ta_semaphore);
        } else {
            if (!ta_sleeping_flag) {
                printf("[TA] Não há estudantes esperando. TA foi dormir.\n");
                ta_sleeping_flag = 1;
            }
            usleep(100000); // Evita busy-wait/deixa o TA dormindo um pouco entre as verificações, sem travar o processador.
        }
    }

    printf("[TA] Todos os estudantes foram atendidos. TA encerrando atendimento.\n");

    pthread_mutex_lock(&mutex_thread);
    finished = 1; //Avisa aos estudantes que o atendimento terminou.
    pthread_mutex_unlock(&mutex_thread);

    return NULL;
}

void* student(void* student_id) {
    int id_student = *(int*)student_id;
    free(student_id);

    while (1) {
        pthread_mutex_lock(&mutex_thread);
        //Faz o estudante sair do loop quando o atendimento acabar.
        if (finished) {
            pthread_mutex_unlock(&mutex_thread);
            break;
        }
        pthread_mutex_unlock(&mutex_thread);

        if (isWaiting(id_student)) continue;

        int time = (rand() % 5) + 1;
        printf("[Estudante %d] Está programando por %d segundos...\n", id_student, time);
        sleep(time);

        pthread_mutex_lock(&mutex_thread);
        if (finished) {
            pthread_mutex_unlock(&mutex_thread);
            break;
        }

        if (num_students_waiting < NUM_CHAIRS) {
            waiting_room_chairs[next_seat_position] = id_student;
            num_students_waiting++;
            printf("[Estudante %d] Sentou na cadeira %d. (%d/%d ocupadas)\n",
                id_student, next_seat_position + 1, num_students_waiting, NUM_CHAIRS);
            next_seat_position = (next_seat_position + 1) % NUM_CHAIRS;

            pthread_mutex_unlock(&mutex_thread);

            sem_post(&students_semaphore);
            sem_wait(&ta_semaphore);
        } else {
            pthread_mutex_unlock(&mutex_thread);
            printf("[Estudante %d] Sala cheia! Voltará depois...\n", id_student);
        }
    }

    printf("[Estudante %d] Saiu do sistema.\n", id_student);
    return NULL;
}

