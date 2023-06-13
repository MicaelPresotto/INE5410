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
    double resultado;
} trecho_array;

void *produto_escalar(void *arg) {
    trecho_array *trecho = (trecho_array*) arg;
    double soma = 0;
    for (int i = (*trecho).inicio; i < (*trecho).fim; i++)
    {
        soma += (*trecho).array_a[i] * (*trecho).array_b[i];
    }
    trecho->resultado = soma;
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


// Avalia se o prod_escalar é o produto escalar dos vetores a e b. Assume-se
// que ambos a e b sejam vetores de tamanho size.
void avaliar(double* a, double* b, int size, double prod_escalar);

int main(int argc, char* argv[]) {
    srand(time(NULL));

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

    //Calcula produto escalar. Paralelize essa parte
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    printf("A execução do programa começou\n");

    double result = 0;
    pthread_t threads[n_threads];
    int tamanho_trecho = a_size/n_threads;
    int resto = a_size % n_threads;
    trecho_array trechos[n_threads];
    int inicio;
    int fim = 0;
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
        pthread_create(&threads[i],NULL,produto_escalar,(void *) &trechos[i]);
    }
    
    for (int j = 0; j < n_threads; j++)
    {
        pthread_join(threads[j], NULL);
        result += trechos[j].resultado;
    }
    gettimeofday(&end, NULL);
    //    +---------------------------------+
    // ** | IMPORTANTE: avalia o resultado! | **
    //    +---------------------------------+
    avaliar(a, b, a_size, result);

    //Libera memória
    free(a);
    free(b);

    double tempo = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec)/1000000;
 
    printf("Tempo de execução foi de %lf segundos.\n", tempo);

    return 0;
}
