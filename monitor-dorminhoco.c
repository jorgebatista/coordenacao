#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NUM_ALUNOS 100 // Número de alunos que irão procurar atendimento de monitoria

// fila: semaforo que registra quantos alunos estão aguardando atendimento.
// controle: mutex para evitar condições de corrida ao gerenciar o semaforo de fila e o contador de vagas
// monitoria: semaforo para um aluno aguardar a final;ização do seu atendimento
sem_t fila, controle, monitoria;
int cadeiras, vagas;


int nanosleep(const struct timespec *req, struct timespec *rem);

// Função para suspender execução, em milisegundos
int mssleep(long miliseconds)
{
  struct timespec rem;
  struct timespec req = {
    (int)(miliseconds / 1000),
    (miliseconds % 1000) * 1000000
  };

  return nanosleep(&req , &rem);
}


// Código da thread do monitor
void *threadMonitor () {
  while(1) {
    sem_wait(&fila); // Espera ter alguém na fila
    mssleep(rand() % 1000);
    sem_wait(&controle); // Espera pelo controle
    printf("O monitor atendeu um aluno.\n");
    sem_post(&monitoria); // Realiza a monitoria
    vagas++; // Libera uma vaga
    sem_post(&controle); // Libera o controle
  }
}


// Código das threads dos alunos
void *threadAluno () {
  mssleep(rand() % 60000); // Espera um tempo aleatório antes de ir para a monitoria
  sem_wait(&controle); // Espera pelo controle
  // Testa se há vagas
  if(vagas > 0) {
    vagas--;
    sem_post(&fila); // Entra na fila
    // Checa se o monitor está dormindo (se ele é o único aluno na fila)
    if(vagas == cadeiras) {
      printf("Um aluno chegou e acordou o monitor.\n");
    }
    else {
      printf("Um aluno chegou e entrou na fila.\n");
    }
    sem_post(&controle); // Libera controle

    sem_wait(&monitoria); // Aguarda a finalizaçào do atendimento
  }
  else {
    printf("Um aluno chegou e não encontrou vagas. Ele retornará em outro horário.\n");
    sem_post(&controle); // Libera o controle
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
  // Testa se o argumento é um inteiro
  cadeiras = strtol(argv[1], &end, 10);
  if(cadeiras < 0 || *end != 0) {
    printf("\nERRO - O programa deve receber um único inteiro maior ou igual a 0 como argumento. (número de cadeiras na sala de espera)\n\n");
    exit(1);
  }

  vagas = cadeiras + 1;

  // Inicializa os semáforos
  sem_init(&monitoria, 0, 0);
  sem_init(&fila, 0, 0);
  sem_init(&controle, 0, 1);

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
