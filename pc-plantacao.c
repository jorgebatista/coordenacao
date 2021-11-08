#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

// buracos_abrir: semafóro indicando a quantidade de buracos que podem ser abertos
// colocar_sementes: semafóro indicando a quantidade de sementes que podem ser colocadas
// buracos_fechar: semafóro indicando a quantidade de buracos que podem ser fechados
// pa: mutex para controlar o acesso a pá
sem_t buracos_abrir, colocar_sementes, buracos_fechar, pa;

int nanosleep(const struct timespec *req, struct timespec *rem);

// Função para suspender execução, em milisegundos
int mssleep(long miliseconds)
{
   struct timespec rem;
   struct timespec req= {
       (int)(miliseconds / 1000),
       (miliseconds % 1000) * 1000000
   };

   return nanosleep(&req , &rem);
}

// Código da thread de João
void *joao() {
  while(1) {
    sem_wait(&buracos_abrir); // Espera autorização para abrir um buraco
    sem_wait(&pa); // Espera pelo controle da pá
    printf("João abriu um buraco.\n");
    sem_post(&colocar_sementes); // Autoriza colocar uma semente
    sem_post(&pa); // Libera o controle da pá
    mssleep(rand() % 1000); // Espera
  }
}



// Código da thread de Pedro
void *pedro() {
  while(1) {
    sem_wait(&colocar_sementes); // Espera autorização para colocar uma semente
    printf("Pedro colocou uma semente.\n");
    sem_post(&buracos_fechar); // Autoriza fechar um buraco
    mssleep(rand() % 1000); // Espera
  }
}



// Código da thread de Paulo
void *paulo() {
  while(1) {
    sem_wait(&buracos_fechar); // Espera autorização para fechar um buraco
    sem_wait(&pa); // Espera pelo controle da pá
    printf("Paulo fechou um buraco\n");
    sem_post(&buracos_abrir); // Autoriza abrir um buraco
    sem_post(&pa); // Devolve o controle da pá
    mssleep(rand() % 1000); // Espera
  }
}



int main(int argc, char *argv[]) {
  // Testa se foi fornecido um único argumento
  if(argc != 2) {
    printf("\nERRO - O programa deve receber um único inteiro positivo como argumento. (número máximo de buracos simultâneos)\n\n");
    exit(1);
  }

  long n;
  char *end;
  // Testa se o argumento é um inteiro positivo
  n = strtol(argv[1], &end, 10);
  if(n < 1 || *end != 0) {
    printf("\nERRO - O programa deve receber um único inteiro positivo como argumento. (número máximo de buracos simultâneos)\n\n");
    exit(1);
  }

  // Inicializa os semaforos
  sem_init(&buracos_abrir, 0, n);
  sem_init(&colocar_sementes, 0, 0);
  sem_init(&buracos_fechar, 0, 0);
  sem_init(&pa, 0, 1);

  pthread_t thread[3];

  // Cria as threads de João, Pedro e Paulo
  if(pthread_create(&thread[0], NULL, joao, NULL)) {
    perror("pthread_create");
    exit(1);
  }
  if(pthread_create(&thread[0], NULL, pedro, NULL)) {
    perror("pthread_create");
    exit(1);
  }
  if(pthread_create(&thread[0], NULL, paulo, NULL)) {
    perror("pthread_create");
    exit(1);
  }

  pthread_exit (NULL);
}
