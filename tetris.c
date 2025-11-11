#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAX_FILA 5
#define MAX_PILHA 3

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

typedef struct {
    Peca dados[MAX_PILHA];
    int topo;   /* índice do topo da pilha */
    int qtd;    /* quantidade atual de elementos na pilha */
} Pilha;

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

/* Inicializa a pilha vazia */
void inicializarPilha(Pilha *p) {
    p->topo = -1;
    p->qtd = 0;
}

/* Retorna true se a fila estiver cheia */
bool filaCheia(Fila *f) {
    return f->qtd == MAX_FILA;
}

/* Retorna true se a fila estiver vazia */
bool filaVazia(Fila *f) {
    return f->qtd == 0;
}

/* Retorna true se a pilha estiver cheia */
bool pilhaCheia(Pilha *p) {
    return p->qtd == MAX_PILHA;
}

/* Retorna true se a pilha estiver vazia */
bool pilhaVazia(Pilha *p) {
    return p->qtd == 0;
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

/* Empilha uma peça na pilha. Retorna true se sucesso, false se cheia */
bool push(Pilha *p, Peca peca) {
    if (pilhaCheia(p)) return false;
    p->topo++;
    p->dados[p->topo] = peca;
    p->qtd++;
    return true;
}

/* Desempilha uma peça da pilha. Retorna true se sucesso, false se vazia.
   Se sucesso, copia a peça em *peca */
bool pop(Pilha *p, Peca *peca) {
    if (pilhaVazia(p)) return false;
    *peca = p->dados[p->topo];
    p->topo--;
    p->qtd--;
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

/* Mostra a pilha no estado atual (do topo até a base) */
void mostrarPilha(Pilha *p) {
    printf("Estado atual da pilha de reserva (qtd=%d):\n", p->qtd);
    if (pilhaVazia(p)) {
        printf("  <vazia>\n");
        return;
    }
    for (int i = p->topo; i >= 0; --i) {
        Peca peca = p->dados[i];
        if (i == p->topo) {
            printf("  [TOPO] id=%d tipo=%c\n", peca.id, peca.nome);
        } else {
            printf("  [%d]   id=%d tipo=%c\n", p->topo - i, peca.id, peca.nome);
        }
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
    printf("\n========== Menu Aventureiro ==========\n");
    printf("  1 - Jogar peça\n");
    printf("  2 - Reservar peça\n");
    printf("  3 - Usar peça reservada\n");
    printf("  0 - Sair\n");
    printf("====================================\n");
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
    Pilha pilha;
    inicializarFila(&fila);
    inicializarPilha(&pilha);
    preencherFilaInicial(&fila);

    printf("=== Tetris Stack: Nível Aventureiro - Sistema de Reserva ===\n");
    printf("Fila permanece com 5 peças. Pilha de reserva com capacidade máxima de 3.\n\n");
    mostrarFila(&fila);
    mostrarPilha(&pilha);

    for (;;) {
        int opc = menu();
        if (opc == -1) {
            printf("Entrada inválida. Tente novamente.\n");
            continue;
        }

        if (opc == 1) {
            /* Jogar peça: remove a da frente e automaticamente insere nova no final */
            Peca jogada;
            if (dequeue(&fila, &jogada)) {
                printf("\n✓ Peça jogada: id=%d tipo=%c\n", jogada.id, jogada.nome);
                /* inserir automaticamente nova peça para manter exatamente 5 elementos */
                Peca nova = gerarPeca();
                if (!enqueue(&fila, nova)) {
                    /* Em teoria não deverá ocorrer porque tiramos um antes */
                    printf("✗ Erro: não foi possível inserir nova peça (fila cheia).\n");
                } else {
                    printf("✓ Nova peça inserida automaticamente: id=%d tipo=%c\n", nova.id, nova.nome);
                }
            } else {
                printf("✗ Fila vazia. Nenhuma peça para jogar.\n");
            }
            mostrarFila(&fila);
            mostrarPilha(&pilha);

        } else if (opc == 2) {
            /* Reservar peça: move a peça da frente da fila para a pilha */
            if (pilhaCheia(&pilha)) {
                printf("✗ Pilha de reserva está cheia (máximo 3 peças). Não é possível reservar.\n");
            } else {
                Peca reservada;
                if (dequeue(&fila, &reservada)) {
                    if (push(&pilha, reservada)) {
                        printf("\n✓ Peça reservada com sucesso: id=%d tipo=%c\n", reservada.id, reservada.nome);
                        /* inserir automaticamente nova peça na fila para manter 5 elementos */
                        Peca nova = gerarPeca();
                        if (!enqueue(&fila, nova)) {
                            printf("✗ Erro: não foi possível inserir nova peça (fila cheia).\n");
                        } else {
                            printf("✓ Nova peça inserida automaticamente: id=%d tipo=%c\n", nova.id, nova.nome);
                        }
                    } else {
                        printf("✗ Falha ao empilhar peça.\n");
                    }
                } else {
                    printf("✗ Fila vazia. Nenhuma peça para reservar.\n");
                }
            }
            mostrarFila(&fila);
            mostrarPilha(&pilha);

        } else if (opc == 3) {
            /* Usar peça reservada: remove do topo da pilha e coloca na frente da fila */
            Peca usada;
            if (pop(&pilha, &usada)) {
                printf("\n✓ Peça reservada utilizada: id=%d tipo=%c\n", usada.id, usada.nome);
                /* A peça usada foi removida da pilha e pode ser considerada "jogada" */
                printf("✓ Peça agora está em uso (saiu do jogo).\n");
            } else {
                printf("✗ Pilha de reserva vazia. Nenhuma peça para usar.\n");
            }
            mostrarFila(&fila);
            mostrarPilha(&pilha);

        } else if (opc == 0) {
            printf("\nSaindo do jogo...\n");
            break;
        } else {
            printf("Opção desconhecida. Tente novamente.\n");
        }
    }

    return 0;
}
