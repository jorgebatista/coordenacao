#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NUM_ALUNOS 100 // Número de alunos que irão procurar atendimento de monitoria

// vagas: semaforo que registra as vagas (número de cadeiras de espera + cadeira da sala do mobitor)
// controle_fila: mutex para evitar condições de corrida ao gerenciar o semaforo de vagas
// monitor: semaforo para controlar o estado do monitor (dormindo/acordado)
sem_t vagas, controle_fila, monitor;
int cadeiras; // Cadeiras de espera



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


// Código da thread do monitor
void *threadMonitor () {
  int sval;

  while(1) {
    sem_wait(&monitor); // Espera ser acordado
    sem_wait(&controle_fila);
    sem_getvalue(&vagas, &sval); // Checa o tamanho da fila
    sem_post(&controle_fila);

    // Repete o loop até liberar todas as vagas
    while(sval <= cadeiras) {
      sem_wait(&controle_fila);
      printf("Um aluno foi atendido.\n"); // Atende um aluno
      sem_post(&vagas); // Libera uma vaga
      sem_getvalue(&vagas, &sval);
      // Dorme se todas as vagas estiverem liberadas
      if(sval == cadeiras+1) {
        printf("O monitor dormiu.\n");
      }
      sem_post(&controle_fila);
      mssleep(rand() % 2000);
    }
  }
}


// Código das threads dos alunos
void *threadAluno () {
  int sval, na_fila = 0;

  // Executa o loop enquanto o aluno não entrar na fila
  while(na_fila == 0) {
    mssleep(rand() % (NUM_ALUNOS * 1000)); // Espera aleatória
    sem_wait(&controle_fila);
    sem_getvalue(&vagas, &sval);
    // Checa se tem vaga na fila
    if(sval > 0) {
      // Se a fila estiver vazia acorda o monitor
      if(sval == cadeiras + 1) {
        printf("Um aluno acordou o monitor\n");
        sem_post(&monitor);
      }
      // Se a fila não está vazia mas tem vaga, entra na fila
      else {
        printf("Um aluno entrou na fila.\n");
      }
      sem_wait(&vagas);
      na_fila = 1;
    }
    // Se não tiver vaga repete o loop
    else {
      printf("Um aluno chegou e encontrou a fila cheia. Ele retornará mais tarde.\n");
    }
    sem_post(&controle_fila);
  }

  pthread_exit(NULL);
}



int main(int argc, char *argv[]) {
  // Testa se foi fornecido um único argumento
  if(argc != 2) {
    printf("\nERRO - O programa deve receber um único inteiro positivo como argumento. (número de  cadeira na sala de espera)\n\n");
    exit(1);
  }

  char *end;
  // Testa se o argumento é um inteiro positivo
  cadeiras = strtol(argv[1], &end, 10);
  if(cadeiras < 0 || *end != 0) {
    printf("\nERRO - O programa deve receber um único inteiro positivo como argumento. (número de cadeiras na sala de espera)\n\n");
    exit(1);
  }

  // Inicializa os semáforos
  sem_init(&vagas, 0, cadeiras+1); // Cadeiras de espera + cadeira da sala do monitor
  sem_init(&controle_fila, 0, 1);
  sem_init(&monitor, 0, 0);


  pthread_t monitor, alunos[NUM_ALUNOS];

  // Cria a thread do monitor
  if(pthread_create(&monitor, NULL, threadMonitor, NULL)) {
    perror("pthread_create");
    exit(1);
  }
  // Cria as threads dos alunos
  for(long i = 0; i < NUM_ALUNOS; i++) {
    if(pthread_create(&alunos[i], NULL, threadAluno, NULL)) {
      perror("pthread_create");
      exit(1);
    }
  }

  pthread_exit(NULL);
}
