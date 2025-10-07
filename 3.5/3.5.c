#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Структура узла AVL-дерева
struct Node {
    int key;
    struct Node *left, *right;
    int balance; // баланс-фактор: -1 (левый перевес), 0, +1 (правый перевес)
};

// Создание нового узла
struct Node* newNode(int key) {
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    node->key = key;
    node->left = node->right = NULL;
    node->balance = 0;
    return node;
}

// LL-поворот
struct Node* rotateLL(struct Node* p) {
    struct Node* q = p->left;
    q->balance = 0;
    p->balance = 0;
    p->left = q->right;
    q->right = p;
    return q;
}

// RR-поворот
struct Node* rotateRR(struct Node* p) {
    struct Node* q = p->right;
    q->balance = 0;
    p->balance = 0;
    p->right = q->left;
    q->left = p;
    return q;
}

// LR-поворот
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
    return r;
}

// RL-поворот
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
    return r;
}

// Вставка в AVL-дерево с балансировкой (ИСПРАВЛЕННАЯ)
struct Node* insertAVL(struct Node* p, int key, int* heightChanged) {
    if (p == NULL) {
        *heightChanged = 1;
        return newNode(key);
    }
    
    if (key < p->key) {
        p->left = insertAVL(p->left, key, heightChanged);
        if (*heightChanged) {
            switch (p->balance) {
                case -1: // было перевес слева
                    p->balance = 0;
                    *heightChanged = 0;
                    break;
                case 0: // было сбалансировано
                    p->balance = +1;  // ИСПРАВЛЕНО: +1 для левого перевеса
                    break;
                case +1: // было перевес справа → нужна балансировка
                    if (p->left->balance == +1) {  // ИСПРАВЛЕНО: +1 для левого перевеса
                        p = rotateLL(p);
                    } else {
                        p = rotateLR(p);
                    }
                    *heightChanged = 0;
                    break;
            }
        }
    } else if (key > p->key) {
        p->right = insertAVL(p->right, key, heightChanged);
        if (*heightChanged) {
            switch (p->balance) {
                case +1: // было перевес справа
                    p->balance = 0;
                    *heightChanged = 0;
                    break;
                case 0: // было сбалансировано
                    p->balance = -1;  // ИСПРАВЛЕНО: -1 для правого перевеса
                    break;
                case -1: // было перевес слева → нужна балансировка
                    if (p->right->balance == -1) {  // ИСПРАВЛЕНО: -1 для правого перевеса
                        p = rotateRR(p);
                    } else {
                        p = rotateRL(p);
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

// Подсчёт размера дерева
int getSize(struct Node* root) {
    if (root == NULL) return 0;
    return 1 + getSize(root->left) + getSize(root->right);
}

// Инфиксный обход
void inorder(struct Node* root) {
    if (root != NULL) {
        inorder(root->left);
        printf("%d ", root->key);
        inorder(root->right);
    }
}

// Вычисление высоты дерева
int getHeight(struct Node* root) {
    if (root == NULL) return 0;
    int left = getHeight(root->left);
    int right = getHeight(root->right);
    return (left > right ? left : right) + 1;
}

// Главная функция для тестирования
int main() {
    srand(time(0));

    struct Node* root = NULL;
    int n = 100;

    printf("Строим AVL-дерево из %d случайных чисел (от 1 до 500):\n", n);
    for (int i = 0; i < n; i++) {
        int val = rand() % 500 + 1;
        root = insert(root, val);
    }

    int size_avl = getSize(root);
    printf("Размер AVL-дерева: %d\n", size_avl);
    printf("Ожидаемый размер: %d\n", n);
    
    if (size_avl != n) {
        printf("ОШИБКА: Дерево потеряло %d узлов!\n", n - size_avl);
    } else {
        printf("УСПЕХ: Размеры совпадают!\n");
    }

    printf("Высота AVL-дерева: %d\n", getHeight(root));
    
    freeTree(root);
    return 0;
}
