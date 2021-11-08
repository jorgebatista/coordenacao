#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

// Mutexes de controle de acesso dos buffers B1 e B2.
sem_t b1, b2;
// Semáforos para controlar o espaço livre e o número de itens nos buffers B1 e B2.
sem_t vagas_b1, vagas_b2, itens_b1, itens_b2;
int n_itens_b1 = 0, n_itens_b2 = 0;



// Texto explicando como utilizar os argumentos
int avisoArgumentos () {
  printf("\nO programa necessita de 5 argumentos inteiros positivos:\n\n");
  printf("pcbuffers X Y Z B1 B2\n\n");
  printf("X: Números de threads do tipo X.\n");
  printf("Y: Números de threads do tipo Y.\n");
  printf("Z: Números de threads do tipo Z.\n");
  printf("B1: Tamanho do buffer B1\n");
  printf("B2: Tamanho do buffer B2.\n\n");

  exit(1);
}



// Código das threads produtor X
void *threadX(void *arg) {
  long tid = (long) arg;

  while(1) {
    sem_wait(&vagas_b1); // Espera ter uma vaga disponível no buffer B1
    sem_wait(&b1); // Espera pelo controle de B1
    n_itens_b1++;
    printf("X%ld inseriu um item em B1. (B1: %d)\n", tid, n_itens_b1);
    usleep((rand() % 1000) * 1000);
    sem_post(&itens_b1); // Adiciona item em B1
    sem_post(&b1); // Libera o controle de B1
  }
}



// Código das threads consumidor/produtor Y
void *threadY(void *arg) {
  long tid = (long) arg;

  while(1) {
    sem_wait(&itens_b1); // Espera ter um item em B1
    sem_wait(&vagas_b2); // Espera ter uma vaga em B2
    sem_wait(&b1); // Espera pelo controle de B1
    sem_wait(&b2); // Espera pelo control de B2
    n_itens_b1--;
    printf("Y%ld retirou um item de B1. (B1: %d)\n", tid, n_itens_b1);
    n_itens_b2++;
    printf("Y%ld inseriu um item em B2. (B2: %d)\n", tid, n_itens_b2);
    usleep((rand() % 1000) * 1000);
    sem_post(&vagas_b1); // Libera uma vaga em B1
    sem_post(&itens_b2); // Coloca um item em B2
    sem_post(&b1); // Libera o controle de B1
    sem_post(&b2); // Libera o controle de B2
  }
}



// Código das threads consumidor Z
void *threadZ(void *arg) {
  long tid = (long) arg;

  while(1) {
    sem_wait(&itens_b2); // Espera ter um item em B2
    sem_wait(&b2); // Espera pelo controle de B2
    n_itens_b2--;
    printf("Z%ld retirou um item de B2. (B2: %d)\n", tid, n_itens_b2);
    usleep((rand() % 1000) * 1000);
    sem_post(&vagas_b2); // Libera uma vaga em B2
    sem_post(&b2); // Libera o controle de B2
  }
}



int main(int argc, char *argv[]) {
  // Testa se foram fornecido 5 argumentos
  if(argc != 6) {
    printf("ERRO - Número incorreto de argumentos\n");
    avisoArgumentos();
  }

  long n;
  char *end;
  // Testa se os 5 argumentos foram inteiros positivos
  for(int i = 1; i < argc; i++) {
    n = strtol(argv[i], &end, 10);
    if(n < 1 || *end != 0) {
      printf("ERRO - Argumento inválido: '%s'\n", argv[i]);
      avisoArgumentos();
    }
  }

  int num_x = atoi(argv[1]); // Número de threads do tipo X
  int num_y = atoi(argv[2]); // Número de threads do tipo Y
  int num_z = atoi(argv[3]); // Número de threads do tipo Z
  int size_b1 = atoi(argv[4]); // Tamanho do buffer B1
  int size_b2 = atoi(argv[5]); // Tamanho do buffer B2

  // Inicializa os semáforos
  sem_init(&b1, 0, 1);
  sem_init(&vagas_b1, 0, size_b1);
  sem_init(&itens_b1, 0, 0);
  sem_init(&b2, 0, 1);
  sem_init(&vagas_b2, 0, size_b2);
  sem_init(&itens_b2, 0, 0);

  pthread_t x[num_x];
  pthread_t y[num_y];
  pthread_t z[num_z];
  long status;

  // Cria as threads tipo X
  for(long i = 1; i <= num_x; i++) {
    if(pthread_create(&x[i-1], NULL, threadX, (void *) i)) {
      perror("pthread_create");
      exit(1);
    }
  }
  // Cria as threads tipo Y
  for(long i = 1; i <= num_x; i++) {
    if(pthread_create(&y[i-1], NULL, threadY, (void *) i)) {
      perror("pthread_create");
      exit(1);
    }
  }
  // Cria as threads tipo Z
  for(long i = 1; i <= num_x; i++) {
    if(pthread_create(&z[i-1], NULL, threadZ, (void *) i)) {
      perror("pthread_create");
      exit(1);
    }
  }

  pthread_exit (NULL);
}
