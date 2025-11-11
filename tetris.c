#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define MAX_FILA 5
#define MAX_PILHA 3
#define UNDO_CAP 50

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

/* Snapshot para undo (cópia completa das estruturas) */
typedef struct {
    Fila f;
    Pilha p;
} Snapshot;

typedef struct {
    Snapshot pilha[UNDO_CAP];
    int topo; /* -1 se vazio */
} UndoStack;

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

/* Inicializa undo stack */
void inicializarUndo(UndoStack *u) {
    u->topo = -1;
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

/* Operações de snapshot/undo */
void snapshotPush(UndoStack *u, Fila *f, Pilha *p) {
    if (u->topo + 1 >= UNDO_CAP) {
        /* simples rotação: descartamos o mais antigo para abrir espaço */
        /* mover tudo para esquerda */
        memmove(&u->pilha[0], &u->pilha[1], sizeof(Snapshot) * (UNDO_CAP - 1));
        u->topo = UNDO_CAP - 2;
    }
    u->topo++;
    u->pilha[u->topo].f = *f;
    u->pilha[u->topo].p = *p;
}

bool snapshotPop(UndoStack *u, Fila *f_out, Pilha *p_out) {
    if (u->topo < 0) return false;
    *f_out = u->pilha[u->topo].f;
    *p_out = u->pilha[u->topo].p;
    u->topo--;
    return true;
}

/* Troca segura entre o elemento da frente da fila e o topo da pilha */
bool trocarTopoComFrente(Fila *f, Pilha *p) {
    if (filaVazia(f) || pilhaVazia(p)) return false;
    Peca frente = f->dados[f->frente];
    Peca topo = p->dados[p->topo];
    /* troca direta */
    f->dados[f->frente] = topo;
    p->dados[p->topo] = frente;
    return true;
}

/* Inverte conteúdo entre fila e pilha (restrição: tamanhos precisam caber) */
bool inverterFilaComPilha(Fila *f, Pilha *p) {
    /* Só realizamos a operação se ambos couberem nas capacidades opostas */
    if (f->qtd > MAX_PILHA || p->qtd > MAX_FILA) return false;
    /* construir temporários */
    Peca novaFilaDados[MAX_FILA];
    int novaFilaQtd = 0;
    /* A nova fila terá os elementos da pilha do topo para base, mantendo ordem onde frente=topo atual */
    for (int i = p->topo; i >= 0; --i) {
        novaFilaDados[novaFilaQtd++] = p->dados[i];
    }
    Peca novaPilhaDados[MAX_PILHA];
    int novaPilhaTopo = -1;
    /* A nova pilha terá os elementos da fila, onde o topo será a frente atual */
    for (int i = 0; i < f->qtd; ++i) {
        int idx = (f->frente + i) % MAX_FILA;
        novaPilhaDados[++novaPilhaTopo] = f->dados[idx];
    }

    /* aplicar nas estruturas originais */
    inicializarFila(f);
    for (int i = 0; i < novaFilaQtd; ++i) {
        enqueue(f, novaFilaDados[i]);
    }
    inicializarPilha(p);
    for (int i = 0; i <= novaPilhaTopo; ++i) {
        push(p, novaPilhaDados[i]);
    }
    return true;
}

int menu() {
    printf("\n========== Menu Mestre ==========");
    printf("\n  1 - Jogar peça");
    printf("\n  2 - Reservar peça");
    printf("\n  3 - Usar peça reservada");
    printf("\n  4 - Trocar topo da pilha com frente da fila");
    printf("\n  5 - Desfazer última jogada");
    printf("\n  6 - Inverter fila com pilha");
    printf("\n  0 - Sair");
    printf("\n================================");
    printf("\nEscolha: ");
    int opc;
    if (scanf("%d", &opc) != 1) {
        while (getchar() != '\n');
        return -1;
    }
    return opc;
}

void mostrarEstado(Fila *f, Pilha *p) {
    printf("\n");
    mostrarFila(f);
    mostrarPilha(p);
}

int main(void) {
    srand((unsigned) time(NULL));
    Fila fila;
    Pilha pilha;
    UndoStack undo;

    inicializarFila(&fila);
    inicializarPilha(&pilha);
    inicializarUndo(&undo);
    preencherFilaInicial(&fila);

    printf("=== Tetris Stack: Nível Mestre - Integração Avançada entre Fila e Pilha ===\n");
    printf("Fila com %d peças. Pilha de reserva com capacidade máxima de %d.\n\n", MAX_FILA, MAX_PILHA);

    mostrarEstado(&fila, &pilha);

    for (;;) {
        int opc = menu();
        if (opc == -1) {
            printf("Entrada inválida. Tente novamente.\n");
            continue;
        }

        if (opc == 0) {
            printf("\nSaindo do jogo...\n");
            break;
        }

        if (opc == 1) {
            /* Jogar peça: remove a da frente e automaticamente insere nova no final */
            snapshotPush(&undo, &fila, &pilha);
            Peca jogada;
            if (dequeue(&fila, &jogada)) {
                printf("\n✓ Peça jogada: id=%d tipo=%c\n", jogada.id, jogada.nome);
                /* inserir automaticamente nova peça para manter exatamente MAX_FILA elementos */
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
            mostrarEstado(&fila, &pilha);

        } else if (opc == 2) {
            /* Reservar peça: move a peça da frente da fila para a pilha */
            if (pilhaCheia(&pilha)) {
                printf("✗ Pilha de reserva está cheia (máximo %d peças). Não é possível reservar.\n", MAX_PILHA);
            } else {
                snapshotPush(&undo, &fila, &pilha);
                Peca reservada;
                if (dequeue(&fila, &reservada)) {
                    if (push(&pilha, reservada)) {
                        printf("\n✓ Peça reservada com sucesso: id=%d tipo=%c\n", reservada.id, reservada.nome);
                        /* inserir automaticamente nova peça na fila para manter MAX_FILA elementos */
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
            mostrarEstado(&fila, &pilha);

        } else if (opc == 3) {
            /* Usar peça reservada: remove do topo da pilha e considera jogada */
            if (pilhaVazia(&pilha)) {
                printf("✗ Pilha de reserva vazia. Nenhuma peça para usar.\n");
            } else {
                snapshotPush(&undo, &fila, &pilha);
                Peca usada;
                if (pop(&pilha, &usada)) {
                    printf("\n✓ Peça reservada utilizada: id=%d tipo=%c\n", usada.id, usada.nome);
                    printf("✓ Peça agora está em uso (saiu do jogo).\n");
                }
            }
            mostrarEstado(&fila, &pilha);

        } else if (opc == 4) {
            /* Trocar peça do topo da pilha com a da frente da fila */
            if (filaVazia(&fila) || pilhaVazia(&pilha)) {
                printf("✗ Operação inválida: é necessário que fila e pilha tenham peças.\n");
            } else {
                snapshotPush(&undo, &fila, &pilha);
                if (trocarTopoComFrente(&fila, &pilha)) {
                    printf("\n✓ Troca realizada entre frente da fila e topo da pilha.\n");
                } else {
                    printf("✗ Falha ao trocar elementos.\n");
                }
            }
            mostrarEstado(&fila, &pilha);

        } else if (opc == 5) {
            /* Desfazer última jogada */
            if (snapshotPop(&undo, &fila, &pilha)) {
                printf("\n↶ Última ação desfeita. Estado restaurado.\n");
            } else {
                printf("✗ Não há ações para desfazer.\n");
            }
            mostrarEstado(&fila, &pilha);

        } else if (opc == 6) {
            /* Inverter fila com pilha */
            /* verificamos se cabe */
            if (fila.qtd > MAX_PILHA || pilha.qtd > MAX_FILA) {
                printf("✗ Não é possível inverter: tamanhos incompatíveis com as capacidades (fila=%d, pilha=%d).\n", fila.qtd, pilha.qtd);
            } else {
                snapshotPush(&undo, &fila, &pilha);
                if (inverterFilaComPilha(&fila, &pilha)) {
                    printf("\n⇄ Inversão entre fila e pilha realizada com sucesso.\n");
                } else {
                    printf("✗ Falha ao inverter fila com pilha.\n");
                }
            }
            mostrarEstado(&fila, &pilha);

        } else {
            printf("Opção desconhecida. Tente novamente.\n");
        }
    }

    return 0;
}
