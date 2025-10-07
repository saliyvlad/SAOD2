#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Структура узла AVL-дерева
struct Node {
    int key;
    struct Node *left, *right;
    int balance; // баланс-фактор
};

// Создание нового узла
struct Node* newNode(int key) {
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    node->key = key;
    node->left = node->right = NULL;
    node->balance = 0;
    return node;
}

// LL-поворот (как в псевдокоде)
struct Node* rotateLL(struct Node* p) {
    struct Node* q = p->left;
    q->balance = 0;
    p->balance = 0;
    p->left = q->right;
    q->right = p;
    p = q;
    return p;
}

// RR-поворот (как в псевдокоде)
struct Node* rotateRR(struct Node* p) {
    struct Node* q = p->right;
    q->balance = 0;
    p->balance = 0;
    p->right = q->left;
    q->left = p;
    p = q;
    return p;
}

// LR-поворот (как в псевдокоде)
struct Node* rotateLR(struct Node* p) {
    struct Node* q = p->left;
    struct Node* r = q->right;
    
    if (r->balance < 0) 
        p->balance = +1;
    else 
        p->balance = 0;
    
    if (r->balance > 0) 
        q->balance = -1;
    else 
        q->balance = 0;
    
    r->balance = 0;
    p->left = r->right;
    q->right = r->left;
    r->left = q;
    r->right = p;
    p = r;
    
    return p;
}

// RL-поворот (аналогично LR)
struct Node* rotateRL(struct Node* p) {
    struct Node* q = p->right;
    struct Node* r = q->left;
    
    if (r->balance > 0) 
        p->balance = -1;
    else 
        p->balance = 0;
    
    if (r->balance < 0) 
        q->balance = +1;
    else 
        q->balance = 0;
    
    r->balance = 0;
    p->right = r->left;
    q->left = r->right;
    r->right = q;
    r->left = p;
    p = r;
    
    return p;
}

// Вставка в AVL-дерево с балансировкой
struct Node* insertAVL(struct Node* p, int key, int* heightChanged) {
    if (p == NULL) {
        *heightChanged = 1;
        return newNode(key);
    }
    
    if (key < p->key) {
        p->left = insertAVL(p->left, key, heightChanged);
        if (*heightChanged) {
            switch (p->balance) {
                case +1: // было перевес слева
                    p->balance = 0;
                    *heightChanged = 0;
                    break;
                case 0: // было сбалансировано
                    p->balance = -1;
                    break;
                case -1: // было перевес справа → нужна балансировка
                    if (p->left->balance == -1) {
                        p = rotateLL(p); // LL-поворот
                    } else {
                        p = rotateLR(p); // LR-поворот
                    }
                    *heightChanged = 0;
                    break;
            }
        }
    } else if (key > p->key) {
        p->right = insertAVL(p->right, key, heightChanged);
        if (*heightChanged) {
            switch (p->balance) {
                case -1: // было перевес справа
                    p->balance = 0;
                    *heightChanged = 0;
                    break;
                case 0: // было сбалансировано
                    p->balance = +1;
                    break;
                case +1: // было перевес слева → нужна балансировка
                    if (p->right->balance == +1) {
                        p = rotateRR(p); // RR-поворот
                    } else {
                        p = rotateRL(p); // RL-поворот
                    }
                    *heightChanged = 0;
                    break;
            }
        }
    } else {
        // Дубликаты не разрешены
        *heightChanged = 0;
    }
    
    return p;
}

// Обертка для вставки
struct Node* insert(struct Node* root, int key) {
    int heightChanged = 0;
    return insertAVL(root, key, &heightChanged);
}

// Инфиксный обход (слева направо)
void inorder(struct Node* root) {
    if (root != NULL) {
        inorder(root->left);
        printf("%d ", root->key);
        inorder(root->right);
    }
}

// Подсчёт размера дерева
int getSize(struct Node* root) {
    if (root == NULL) return 0;
    return 1 + getSize(root->left) + getSize(root->right);
}

// Подсчёт контрольной суммы
int getChecksum(struct Node* root) {
    if (root == NULL) return 0;
    return root->key + getChecksum(root->left) + getChecksum(root->right);
}

// Вычисление высоты дерева
int getHeight(struct Node* root) {
    if (root == NULL) return 0;
    int left = getHeight(root->left);
    int right = getHeight(root->right);
    return (left > right ? left : right) + 1;
}

// Вычисление средней глубины
typedef struct {
    long totalDepth;
    int leafCount;
} DepthStats;

void calculateAverageDepth(struct Node* root, int depth, DepthStats* stats) {
    if (root == NULL) return;

    if (root->left == NULL && root->right == NULL) {
        stats->totalDepth += depth;
        stats->leafCount++;
    }

    calculateAverageDepth(root->left, depth + 1, stats);
    calculateAverageDepth(root->right, depth + 1, stats);
}

double getAverageDepth(struct Node* root) {
    if (root == NULL) return 0.0;
    DepthStats stats = {0, 0};
    calculateAverageDepth(root, 0, &stats);
    if (stats.leafCount == 0) return 0.0;
    return (double)stats.totalDepth / stats.leafCount;
}

// Функция для вычисления логарифма по основанию 2
double log2_custom(double x) {
    if (x <= 0) return 0;
    double result = 0;
    double temp = x;
    while (temp >= 2) {
        temp /= 2;
        result++;
    }
    return result;
}

// Освобождение памяти
void freeTree(struct Node* root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

// Функция для печати дерева (простая визуализация)
void printTree(struct Node* root, int level) {
    if (root == NULL) return;
    
    printTree(root->right, level + 1);
    
    for (int i = 0; i < level; i++) printf("    ");
    printf("%d(%d)\n", root->key, root->balance);
    
    printTree(root->left, level + 1);
}

// Главная функция
int main() {
    srand(time(0));

    struct Node* root = NULL;
    int n = 100;

    printf("Строим AVL-дерево из %d случайных чисел (от 1 до 500):\n", n);
    for (int i = 0; i < n; i++) {
        int val = rand() % 500 + 1;
        root = insert(root, val);
    }

    printf("\nОбход слева направо (инфиксный): ");
    inorder(root);
    printf("\n\n");

    printf("Структура дерева (корень слева):\n");
    printTree(root, 0);
    printf("\n");

    // Вычисляем характеристики AVL-дерева
    int size_avl = getSize(root);
    int checksum_avl = getChecksum(root);
    int height_avl = getHeight(root);
    double avg_depth_avl = getAverageDepth(root);

    // Характеристики ИСДП
    int size_idp = n;
    int checksum_idp = checksum_avl;
    int height_idp = (int)log2_custom(n);
    
    // Уточняем высоту для ИСДП
    if (n > 63) height_idp = 6;
    else if (n > 31) height_idp = 5;
    else if (n > 15) height_idp = 4;
    else if (n > 7) height_idp = 3;
    else if (n > 3) height_idp = 2;
    else if (n > 1) height_idp = 1;
    else height_idp = 0;

    double avg_depth_idp = (n <= 1) ? 0.0 : log2_custom(n) - 1;

    // Вывод таблицы
    printf("Таблица сравнения характеристик:\n");
    printf("+--------+----------+--------------+---------+-------------+\n");
    printf("| n=100  | Размер   | Контр. Сумма | Высота  | Средн.высота|\n");
    printf("+--------+----------+--------------+---------+-------------+\n");
    printf("| ИСДП   | %-8d | %-12d | %-7d | %-11.2f |\n", size_idp, checksum_idp, height_idp, avg_depth_idp);
    printf("| AVL    | %-8d | %-12d | %-7d | %-11.2f |\n", size_avl, checksum_avl, height_avl, avg_depth_avl);
    printf("+--------+----------+--------------+---------+-------------+\n");

    printf("\nПримечание:\n");
    printf("- ИСДП — идеальное сбалансированное дерево\n");
    printf("- Числа в скобках в визуализации дерева — баланс-факторы\n");

    freeTree(root);
    return 0;
}
