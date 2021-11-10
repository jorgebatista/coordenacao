#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>

#define NUM_BARBEIROS 3
#define NUM_CLIENTES 50
#define CAPACIDADE_MAX 20
#define NUM_SOFA 4
#define NUM_CADEIRAS 3

// capacidade: semáforo para controlar o limite de clientes dentro da barbearia
// sofa: semáforo para a quantidade de clientes sentados no sofá
// cadeira: semáforo para controlar a quantidade de clientes sentados nas cadeiras dos barbeiros
// fila_corte: semáforo para controlar quantos clientes estão na fila para o corte
// pagamento: semáforo para controlar quantos clientes estão na fila de pagamento
// recibo: semáforo para sinalizar a entrega do recibo para o cliente
// controle: semáforo para evitar condições de corrida
// mutex_caixa: semáforo para controlar o acesso ao caixa
// entra_caixa: semáforo para registrar que um barbeiro foi para o caixa
// fecha_caixa: semáforo para indicar que um barbeiro saiu do caixa
sem_t capacidade, sofa, cadeira, fila_corte, corte, pagamento, recibo, controle, mutex_caixa, entra_caixa, fecha_caixa;



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



void *threadCaixa() {
  while(1) {
    sem_wait(&entra_caixa); // Abre o caixa
    // Realiza o pagamento de todos clientes na fila
    while(sem_trywait(&pagamento) == 0) {
      mssleep(rand() % 500);
      printf("Um cliente fez o pagamento.\nRecibo entregue.\n");
      sem_post(&recibo); // Entrega o recibo
    }
    sem_post(&fecha_caixa); // Fecha o caixa
  }
}



void *threadBarbeiro(void *arg) {
  long tid = (long) arg;
  int sval;

  while(1) {
    // Confere se tem algúem esperando por um corte
    if(sem_trywait(&fila_corte) == 0) {
      mssleep(rand() % 3000);
      printf("O barbeiro %ld fez um corte.\n", tid);
      sem_post(&corte); // Finaliza o corte
    }
    // Tenta acessar o mutex do caixa
    if(sem_trywait(&mutex_caixa) == 0) {
      sem_wait(&controle); // Espera o controle
      sem_getvalue(&pagamento, &sval); // Checa se tem clientes na fila de pagamento
      if(sval > 0) {
        printf("O barbeiro %ld foi para o caixa.\n" , tid);
        sem_post(&entra_caixa); // Vai para o caixa
        sem_post(&controle); // Libera o controle
        sem_wait(&fecha_caixa); // Sai do caixa
        printf("O barbeiro %ld saiu do caixa.\n", tid);
      }
      else {
        sem_post(&controle); // Libera o controle
      }
      sem_post(&mutex_caixa); // Libera o mutex do caixa
    }
  }
}




// Código das threads dos clientes
void *threadCliente() {
  mssleep(rand() % 5000);
  // Tenta baixar o semáforo para entrar na barbearia
  if(sem_trywait(&capacidade) == 0) {
    printf("Um cliente entrou na barbearia.\n");
    sem_wait(&sofa); // Espera para sentar no sofá
    printf("Um cliente sentou no sofá.\n");
    sem_wait(&cadeira); // Espera para sentar na cadeira do barbeiro
    sem_post(&sofa); // Libera um assento do sofá
    printf("Um cliente sentou na cadeira do barbeiro.\n");
    sem_post(&fila_corte); // Sinaliza que está pronto fazer o corte
    sem_wait(&corte); // Espera pelo corte

    sem_wait(&controle); // Espera pelo controle
    printf("Um cliente levantou da cadeira de barbeiro e foi para o caixa.\n");
    sem_post(&cadeira); // Libera a cadeira do barbeiro
    sem_post(&pagamento); // Entra na fila de pagamento
    sem_post(&controle); // Libera o controle

    sem_wait(&recibo); // Espera pelo recibo
    sem_post(&capacidade); // Sai da barbearia
  }
  else {
    printf("Um cliente encontrou a barbearia lotada e foi embora.\n");
  }

  pthread_exit(NULL);
}



int main() {
  // Inicializa os semáforos
  sem_init(&capacidade, 0, CAPACIDADE_MAX);
  sem_init(&sofa, 0, NUM_SOFA);
  sem_init(&cadeira, 0, NUM_CADEIRAS);
  sem_init(&fila_corte, 0, 0);
  sem_init(&corte, 0, 0);
  sem_init(&pagamento, 0, 0);
  sem_init(&controle, 0, 1);
  sem_init(&mutex_caixa, 0, 1);
  sem_init(&entra_caixa, 0, 0);
  sem_init(&fecha_caixa, 0, 0);

  pthread_t barbeiro[NUM_BARBEIROS], cliente[NUM_CLIENTES], caixa;

  // Cria a thread do caixa
  if(pthread_create(&caixa, NULL, threadCaixa, NULL)) {
    perror("pthread_create");
    exit(1);
  }
  // Cria as threads dos barbeiros
  for(long i = 1; i <= NUM_BARBEIROS; i++) {
    if(pthread_create(&barbeiro[i], NULL, threadBarbeiro, (void *) i)) {
      perror("pthread_create");
      exit(1);
    }
  }
  // Cria as threads dos clientes
  for(long i = 0; i < NUM_CLIENTES; i++) {
    if(pthread_create(&cliente[i], NULL, threadCliente, NULL)) {
      perror("pthread_create");
      exit(1);
    }
  }

  pthread_exit(NULL);
}
