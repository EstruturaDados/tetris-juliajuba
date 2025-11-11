#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAX_FILA 5

typedef struct {
    char nome; /* Tipo da peça: 'I', 'O', 'T', 'L' */
    int id;    /* Identificador único */
} Peca;

typedef struct {
    Peca dados[MAX_FILA];
    int frente; /* índice do elemento da frente */
    int tras;    /* índice onde será inserido o próximo elemento */
    int qtd;     /* quantidade atual de elementos na fila */
} Fila;

/* Gera uma peça com tipo aleatório e id único */
Peca gerarPeca() {
    static int proximoId = 1;
    char tipos[] = {'I', 'O', 'T', 'L'};
    Peca p;
    p.nome = tipos[rand() % (sizeof(tipos)/sizeof(tipos[0]))];
    p.id = proximoId++;
    return p;
}

/* Inicializa a fila vazia */
void inicializarFila(Fila *f) {
    f->frente = 0;
    f->tras = 0;
    f->qtd = 0;
}

/* Retorna true se a fila estiver cheia */
bool filaCheia(Fila *f) {
    return f->qtd == MAX_FILA;
}

/* Retorna true se a fila estiver vazia */
bool filaVazia(Fila *f) {
    return f->qtd == 0;
}

/* Enfileira uma peça. Retorna true se sucesso, false se cheia */
bool enqueue(Fila *f, Peca p) {
    if (filaCheia(f)) return false;
    f->dados[f->tras] = p;
    f->tras = (f->tras + 1) % MAX_FILA;
    f->qtd++;
    return true;
}

/* Desenfileira a peça da frente. Retorna true se sucesso, false se vazia.
   Se sucesso, copia a peça em *p */
bool dequeue(Fila *f, Peca *p) {
    if (filaVazia(f)) return false;
    *p = f->dados[f->frente];
    f->frente = (f->frente + 1) % MAX_FILA;
    f->qtd--;
    return true;
}

/* Mostra a fila no estado atual (do elemento da frente ao fim) */
void mostrarFila(Fila *f) {
    printf("Estado atual da fila (qtd=%d):\n", f->qtd);
    if (filaVazia(f)) {
        printf("  <vazia>\n");
        return;
    }
    for (int i = 0; i < f->qtd; ++i) {
        int idx = (f->frente + i) % MAX_FILA;
        Peca p = f->dados[idx];
        printf("  [%d] id=%d tipo=%c\n", i, p.id, p.nome);
    }
}

/* Preenche a fila com exatamente MAX_FILA peças geradas automaticamente */
void preencherFilaInicial(Fila *f) {
    while (!filaCheia(f)) {
        Peca p = gerarPeca();
        enqueue(f, p);
    }
}

int menu() {
    printf("\nMenu:\n");
    printf("  1 - Mostrar fila\n");
    printf("  2 - Jogar peça (dequeue + inserir automaticamente nova peça)\n");
    printf("  3 - Inserir nova peça manualmente (enqueue se houver espaço)\n");
    printf("  4 - Sair\n");
    printf("Escolha: ");
    int opc;
    if (scanf("%d", &opc) != 1) {
        /* limpar entrada inválida */
        while (getchar() != '\n');
        return -1;
    }
    return opc;
}

int main(void) {
    srand((unsigned) time(NULL));
    Fila fila;
    inicializarFila(&fila);
    preencherFilaInicial(&fila);

    printf("=== Tetris Stack: controle de peças futuras (fila circular 5) ===\n");
    mostrarFila(&fila);

    for (;;) {
        int opc = menu();
        if (opc == -1) {
            printf("Entrada inválida. Tente novamente.\n");
            continue;
        }

        if (opc == 1) {
            mostrarFila(&fila);
        } else if (opc == 2) {
            /* Jogar peça: remove a da frente e automaticamente insere nova no final */
            Peca jogada;
            if (dequeue(&fila, &jogada)) {
                printf("\nPeça jogada: id=%d tipo=%c\n", jogada.id, jogada.nome);
                /* inserir automaticamente nova peça para manter exatamente 5 elementos */
                Peca nova = gerarPeca();
                if (!enqueue(&fila, nova)) {
                    /* Em teoria não deverá ocorrer porque tiramos um antes */
                    printf("Erro: não foi possível inserir nova peça (fila cheia).\n");
                } else {
                    printf("Nova peça inserida automaticamente: id=%d tipo=%c\n", nova.id, nova.nome);
                }
            } else {
                printf("Fila vazia. Nenhuma peça para jogar.\n");
            }
            mostrarFila(&fila);
        } else if (opc == 3) {
            /* Inserir manualmente (apenas se houver espaço) */
            if (filaCheia(&fila)) {
                printf("A fila já está cheia. Não é possível inserir manualmente.\n");
            } else {
                Peca nova = gerarPeca();
                if (enqueue(&fila, nova)) {
                    printf("Peça inserida manualmente: id=%d tipo=%c\n", nova.id, nova.nome);
                } else {
                    printf("Falha ao inserir peça.\n");
                }
                mostrarFila(&fila);
            }
        } else if (opc == 4) {
            printf("Saindo...\n");
            break;
        } else {
            printf("Opção desconhecida. Tente novamente.\n");
        }
    }

    return 0;
}