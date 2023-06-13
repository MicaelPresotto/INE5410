#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct {
    int inicio;
    int fim;
    double *array_a;
    double *array_b;
    double *array_c;
} trecho_array;

void *somar_vetor(void *arg) {
    trecho_array trecho = *((trecho_array*) arg);
    for (int i = trecho.inicio; i < trecho.fim; i++)
    {
        trecho.array_c[i] = trecho.array_a[i] + trecho.array_b[i];
    }
    pthread_exit(NULL);
}

// Lê o conteúdo do arquivo filename e retorna um vetor E o tamanho dele
// Se filename for da forma "gen:%d", gera um vetor aleatório com %d elementos
//
// +-------> retorno da função, ponteiro para vetor malloc()ado e preenchido
// | 
// |         tamanho do vetor (usado <-----+
// |         como 2o retorno)              |
// v                                       v
double* load_vector(const char* filename, int* out_size);


// Avalia o resultado no vetor c. Assume-se que todos os ponteiros (a, b, e c)
// tenham tamanho size.
void avaliar(double* a, double* b, double* c, int size);


int main(int argc, char* argv[]) {
    // Gera um resultado diferente a cada execução do programa
    // Se **para fins de teste** quiser gerar sempre o mesmo valor
    // descomente o srand(0)
    srand(time(NULL)); //valores diferentes
    //srand(0);        //sempre mesmo valor

    //Temos argumentos suficientes?
    if(argc < 4) {
        printf("Uso: %s n_threads a_file b_file\n"
               "    n_threads    número de threads a serem usadas na computação\n"
               "    *_file       caminho de arquivo ou uma expressão com a forma gen:N,\n"
               "                 representando um vetor aleatório de tamanho N\n",
               argv[0]);
        return 1;
    }
  
    //Quantas threads?
    int n_threads = atoi(argv[1]);
    if (!n_threads) {
        printf("Número de threads deve ser > 0\n");
        return 1;
    }
    //Lê números de arquivos para vetores alocados com malloc
    int a_size = 0, b_size = 0;
    double* a = load_vector(argv[2], &a_size);
    if (!a) {
        //load_vector não conseguiu abrir o arquivo
        printf("Erro ao ler arquivo %s\n", argv[2]);
        return 1;
    }
    double* b = load_vector(argv[3], &b_size);
    if (!b) {
        printf("Erro ao ler arquivo %s\n", argv[3]);
        return 1;
    }
    
    //Garante que entradas são compatíveis
    if (a_size != b_size) {
        printf("Vetores a e b tem tamanhos diferentes! (%d != %d)\n", a_size, b_size);
        return 1;
    }
    //Cria vetor do resultado 
    double* c = malloc(a_size*sizeof(double));

    // Calcula com uma thread só. Programador original só deixou a leitura 
    // do argumento e fugiu pro caribe. É essa computação que você precisa 
    // paralelizar

    struct timeval start, end;
    gettimeofday(&start, NULL);
    printf("A execução do programa começou\n");

    pthread_t threads[n_threads];
    int tamanho_trecho = a_size/n_threads;
    int resto = a_size % n_threads;
    trecho_array trechos[n_threads];
    int inicio;
    int fim = 0;

    if (n_threads > a_size){
        n_threads = a_size;
    }
    for (int i = 0; i < n_threads; i++)
    {
        inicio = fim;
        fim += tamanho_trecho;
        if (resto > 0) {
            resto--;
            fim++;
        }
        trechos[i].inicio = inicio;
        trechos[i].fim = fim;
        trechos[i].array_a = a;
        trechos[i].array_b = b;
        trechos[i].array_c = c;

        pthread_create(&threads[i],NULL,somar_vetor,(void *) &trechos[i]);
    }
    
    for (int j = 0; j < n_threads; j++)
    {
        pthread_join(threads[j], NULL);
    }
    
    gettimeofday(&end, NULL);
    //    +---------------------------------+
    // ** | IMPORTANTE: avalia o resultado! | **
    //    +---------------------------------+
    avaliar(a, b, c, a_size);
    

    //Importante: libera memória
    free(a);
    free(b);
    free(c);

    double tempo = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec)/1000000;
 
    printf("Tempo de execução foi de %lf segundos.\n", tempo);

    return 0;
}
