/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Codigo: Comunicação entre threads usando variável compartilhada e exclusao mutua com bloqueio */

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

#define TAM 100000

long int soma = 0; //variavel compartilhada entre as threads
pthread_mutex_t mutex; //variavel de lock para exclusao mutua
pthread_cond_t cond_print;
pthread_cond_t cond_multi;

//funcao executada pelas threads
void *ExecutaTarefa (void *arg) {
    long int id = (long int) arg;
    printf("Thread : %ld esta executando...\n", id);

    for (int i=0; i<TAM; i++) {
        //--entrada na SC
        pthread_mutex_lock(&mutex);
        //--SC (seção critica)
        soma++; //incrementa a variavel compartilhada

        if(!(soma%1000)) {
            //printf("%ld\n", soma);
            pthread_cond_broadcast(&cond_multi);
            pthread_cond_wait(&cond_print, &mutex);
        }
        //--saida da SC
        pthread_mutex_unlock(&mutex);
    }
    printf("Thread : %ld terminou!\n", id);
    pthread_exit(NULL);
}

//funcao executada pela thread de log
void *extra (void *args) {
    int nthreads = (int)(long int)args;
    printf("Extra : esta executando...\n");
    long int esperado = nthreads * TAM;
    while(soma < esperado) {
        // Espera até que 'soma' atinja o valor 
        pthread_mutex_lock(&mutex);

        pthread_cond_wait(&cond_multi, &mutex);
        printf("soma = %ld \n", soma);
        pthread_cond_broadcast(&cond_print);

        pthread_mutex_unlock(&mutex);
    }
    printf("Extra : terminou!\n");
    pthread_exit(NULL);
}

//fluxo principal
int main(int argc, char *argv[]) {
    pthread_t *tid; //identificadores das threads no sistema
    int nthreads; //qtde de threads (passada linha de comando)

    //--le e avalia os parametros de entrada
    if(argc<2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);

    //--aloca as estruturas
    tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreads+1));
    if(tid==NULL) {puts("ERRO--malloc"); return 2;}

    //--inicilaiza o mutex (lock de exclusao mutua)
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init (&cond_multi, NULL);
    pthread_cond_init (&cond_print, NULL);
    
    //--cria thread de log
    if (pthread_create(&tid[nthreads], NULL, extra, (void *)(long int)nthreads)) {
        printf("--ERRO: pthread_create()\n"); exit(-1);
    }

    //--cria as threads
    for(long int t=0; t<nthreads; t++) {
        if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)t)) {
            printf("--ERRO: pthread_create()\n"); exit(-1);
        }
    }

    //--espera todas as threads terminarem
    for (int t=0; t<nthreads+1; t++) {
        if (pthread_join(tid[t], NULL)) {
            printf("--ERRO: pthread_join() \n"); exit(-1); 
        } 
    } 

    //--finaliza o mutex
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_multi);
    pthread_cond_destroy(&cond_print);
    free(tid);

    printf("Valor de 'soma' = %ld\n", soma);

    return 0;
}
