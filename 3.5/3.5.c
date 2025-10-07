#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Структура узла AVL-дерева
struct Node {
    int key;
    struct Node *left, *right;
    int balance; // баланс-фактор: +1 (левый перевес), 0, -1 (правый перевес)
};

// Глобальные счетчики для диагностики
int inserted_count = 0;
int duplicate_count = 0;

// Создание нового узла
struct Node* newNode(int key) {
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    node->key = key;
    node->left = node->right = NULL;
    node->balance = 0;
    inserted_count++;
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
        p->balance = 1;
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
        q->balance = 1;
    else 
        q->balance = 0;
    
    r->balance = 0;
    p->right = r->left;
    q->left = r->right;
    r->right = q;
    r->left = p;
    return r;
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
                case 1: // было перевес слева
                    p->balance = 0;
                    *heightChanged = 0;
                    break;
                case 0: // было сбалансировано
                    p->balance = -1;
                    *heightChanged = 1;
                    break;
                case -1: // было перевес справа → нужна балансировка
                    if (p->left->balance == -1) {
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
                case -1: // было перевес справа
                    p->balance = 0;
                    *heightChanged = 0;
                    break;
                case 0: // было сбалансировано
                    p->balance = 1;
                    *heightChanged = 1;
                    break;
                case 1: // было перевес слева → нужна балансировка
                    if (p->right->balance == 1) {
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
        duplicate_count++;
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

// Проверка уникальности ключей в дереве
void checkUnique(struct Node* root, int* seen, int* duplicate_in_tree) {
    if (root == NULL) return;
    
    if (seen[root->key]) {
        (*duplicate_in_tree)++;
        printf("Дубликат в дереве: %d\n", root->key);
    } else {
        seen[root->key] = 1;
    }
    
    checkUnique(root->left, seen, duplicate_in_tree);
    checkUnique(root->right, seen, duplicate_in_tree);
}

// Освобождение памяти
void freeTree(struct Node* root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

// Главная функция для тестирования
int main() {
    srand(time(0));

    struct Node* root = NULL;
    int n = 100;
    
    // Сброс счетчиков
    inserted_count = 0;
    duplicate_count = 0;

    printf("Строим AVL-дерево из %d случайных чисел (от 1 до 500):\n", n);
    
    // Сохраняем все сгенерированные ключи для проверки
    int generated_keys[1000];
    int generated_count = 0;
    
    for (int i = 0; i < n; i++) {
        int val = rand() % 500 + 1;
        generated_keys[generated_count++] = val;
        root = insert(root, val);
    }

    int size_avl = getSize(root);
    
    printf("\n=== ДИАГНОСТИКА ===\n");
    printf("Попыток вставки: %d\n", n);
    printf("Успешных вставок: %d\n", inserted_count);
    printf("Обнаружено дубликатов при вставке: %d\n", duplicate_count);
    printf("Размер дерева (getSize): %d\n", size_avl);
    
    // Проверяем уникальность в дереве
    int seen[501] = {0}; // от 1 до 500
    int duplicate_in_tree = 0;
    checkUnique(root, seen, &duplicate_in_tree);
    printf("Дубликатов в дереве: %d\n", duplicate_in_tree);
    
    // Проверяем какие ключи были сгенерированы
    int unique_generated = 0;
    int gen_seen[501] = {0};
    for (int i = 0; i < generated_count; i++) {
        if (!gen_seen[generated_keys[i]]) {
            unique_generated++;
            gen_seen[generated_keys[i]] = 1;
        }
    }
    printf("Уникальных сгенерированных ключей: %d\n", unique_generated);
    
    if (size_avl != n) {
        printf("\nОШИБКА: Размер дерева (%d) не равен n (%d)\n", size_avl, n);
        printf("Потеряно узлов: %d\n", n - size_avl);
        
        // Ищем пропущенные ключи
        printf("\nПропущенные ключи:\n");
        int missing_count = 0;
        for (int i = 0; i < generated_count; i++) {
            int found = 0;
            // Простая проверка наличия ключа в дереве
            struct Node* current = root;
            while (current != NULL) {
                if (generated_keys[i] == current->key) {
                    found = 1;
                    break;
                } else if (generated_keys[i] < current->key) {
                    current = current->left;
                } else {
                    current = current->right;
                }
            }
            if (!found) {
                printf("%d ", generated_keys[i]);
                missing_count++;
                if (missing_count % 10 == 0) printf("\n");
            }
        }
        printf("\nВсего пропущено: %d\n", missing_count);
    } else {
        printf("\nУСПЕХ: Размеры совпадают!\n");
    }

    printf("Высота AVL-дерева: %d\n", getHeight(root));
    
    freeTree(root);
    return 0;
}
