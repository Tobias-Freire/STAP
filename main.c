#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#define NUM_CHAIRS 3
#define DEFAULT_NUM_STUDENTS 5

//Prototipos---------------------->
//Validacao entrada do usuario:
int isNumber(char number[]);
int string2number(char number[]);

//Verificacao do estado da sala de espera:
int isWaiting(int student_id);

//Tratamento das threads TA e estudantes.
void* ta(void* arg);
void* student(void* student_id);
//<--------------------------------

//TRATAMENTO DE CONDICAO DE CORRIDA!!!
//Criacao de semaforos e mutex para garantir sincronizacao entre as threads!!!
sem_t students_semaphore;
sem_t ta_semaphore;
pthread_mutex_t mutex_thread;

//Declarando variaveis do problema STAP, como o numero de cadeiras, estudantes em espera e etc.!!! 
int waiting_room_chairs[NUM_CHAIRS];
int total_teaching_time = 0;
int num_students_waiting = 0;
int next_seat_position = 0;
int next_teaching_position = 0;
int ta_sleeping_flag = 0;//Diz se o TA ta dormindo ou nao.
int students_served = 0;//Marca os estudantes que ja foram atendidos.
int total_students = 0;
int finished = 0;  //Flag para indicar que todos os estudantes foram atendidos e finalizar o programa.

int main(int argc, char* argv[]) {
    clock_t start_time = clock();

    int student_num;

    //Verificando se o numero de estudantes passado como argumento na hora de rodar o .exec eh valido ou nao!!! 
    if (argc > 1) {
        student_num = string2number(argv[1]);//Conversao do arg passado como string para inteiro. Se o arg nao for um numero, a funcao string2number retorna -1.
        if (student_num == -1) {
            printf("Numero de estudantes invalido!\n");
            return 1;
        }
    } else {
        student_num = DEFAULT_NUM_STUDENTS;
    }

    //Salvando o num. total de estudantes na variavel global 'total_students'.
    total_students = student_num;

    //Criacao das variaveis onde serao armazenadas informacoes da thread. Servem meio que como identificadores das threads!!!
    pthread_t students[student_num];//Estudantes
    pthread_t ta_thread;//TA

    //Inicializacao dos semaforos e mutex!!!
    sem_init(&students_semaphore, 0, 0); 
    sem_init(&ta_semaphore, 0, 1); 
    pthread_mutex_init(&mutex_thread, NULL);

    //Criacao da thread do TA!!!
    pthread_create(&ta_thread, NULL, ta, NULL);

    //Criacao das threads dos estudantes!!!
    for (int i = 0; i < student_num; i++) {
        int* student_id = malloc(sizeof(int));//Usando o malloc para armazenar o ID dos estudantes, pois a funcao pthread_create precisa receber um void* como argumento.
        *student_id = i + 1;
        pthread_create(&students[i], NULL, student, (void*) student_id);//(void*) student_id: argumento passado para a funcao student(), para que cada estudante tenha seu ID proprio.
    }

    //Basicamente o wait() para processos, soh que para threads. Ou seja,o programa espera a thread terminar o que esta fazendo!!!
    pthread_join(ta_thread, NULL);

    for (int i = 0; i < student_num; i++) {
        pthread_join(students[i], NULL);
    }

    clock_t end_time = clock();  
    double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("\n[METRICA] Tempo total de execução: %.2f segundos\n", total_time);

    double avg_teaching_time = (double) total_teaching_time / total_students;
    printf("[METRICA] Tempo médio de atendimento: %.2f segundos\n", avg_teaching_time);

    return 0;
}

//Verifica se a string number contem apenas numeros validos positivos!!!
int isNumber(char number[]) {
    int size = strlen(number);
    for (int i = 0; i < size; i++) {
        if (!isdigit(number[i])) return 0; 
    }
    return 1;
}

//Converte string para inteiro!!!
int string2number(char number[]) {
    return isNumber(number) ? atoi(number) : -1;//atoi: ASCII to Interger.
}

//Verifica se determinado estudante esta sentado na sala de espera!!!
int isWaiting(int student_id) {
    for (int i = 0; i < NUM_CHAIRS; i++) {
        if (waiting_room_chairs[i] == student_id) return 1;
    }
    return 0;
}

void* ta(void* arg) {
    printf("[TA] Iniciando. Verificando se ha estudantes...\n");

    while (students_served < total_students) {
        if (num_students_waiting > 0) {
            ta_sleeping_flag = 0;
            sem_wait(&students_semaphore);
            pthread_mutex_lock(&mutex_thread);

            int student_id = waiting_room_chairs[next_teaching_position];
            int teaching_time = (rand() % 5) + 1;
            printf("[TA] Vai ajudar o estudante %d por %d segundos...\n", student_id, teaching_time);
            sleep(teaching_time);
            total_teaching_time += teaching_time;

            printf("[TA] Estudante %d terminou de ser atendido.\n", student_id);
            waiting_room_chairs[next_teaching_position] = 0;
            next_teaching_position = (next_teaching_position + 1) % NUM_CHAIRS;
            num_students_waiting--;
            students_served++;

            pthread_mutex_unlock(&mutex_thread);
            sem_post(&ta_semaphore);
        } else {
            if (!ta_sleeping_flag) {
                printf("[TA] Nao ha estudantes esperando. TA foi dormir.\n");
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
    int id_student = *(int*)student_id; // conversão para inteiro
    free(student_id); // memória alocada para id do estudante é liberada

    while (1) {
        pthread_mutex_lock(&mutex_thread); // mutex para evitar condição de corrida
        if (finished) {
            pthread_mutex_unlock(&mutex_thread);
            break; //Faz o estudante sair do loop quando o atendimento acabar.
        }
        pthread_mutex_unlock(&mutex_thread); // libera o mutex

        if (isWaiting(id_student)) continue;

        // Simula o tempo que o estudante leva para programar.
        int time = (rand() % 5) + 1; 
        printf("[Estudante %d] Esta programando por %d segundos...\n", id_student, time);
        sleep(time);

        pthread_mutex_lock(&mutex_thread); 
        if (finished) {
            pthread_mutex_unlock(&mutex_thread);
            break;
        }
        
        // Verifica se há espaço na sala de espera
        if (num_students_waiting < NUM_CHAIRS) {
            waiting_room_chairs[next_seat_position] = id_student;
            num_students_waiting++;
            printf("[Estudante %d] Sentou na cadeira %d. (%d/%d ocupadas)\n",
                id_student, next_seat_position + 1, num_students_waiting, NUM_CHAIRS);
            next_seat_position = (next_seat_position + 1) % NUM_CHAIRS;

            pthread_mutex_unlock(&mutex_thread); // libera o mutex

            sem_post(&students_semaphore); // sinaliza que há um estudante esperando
            sem_wait(&ta_semaphore); // espera ser atendido
        } else { // Sala cheia
            pthread_mutex_unlock(&mutex_thread);
            printf("[Estudante %d] Sala cheia! Voltara depois...\n", id_student);
        }
    }

    // Estudante sai do sistema
    printf("[Estudante %d] Saiu do sistema.\n", id_student);
    return NULL;
}

