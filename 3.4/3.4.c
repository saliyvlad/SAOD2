#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Структура узла дерева
struct Node {
    int key;
    struct Node *left, *right;
};

// Создание нового узла
struct Node* newNode(int item) {
    struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
    temp->key = item;
    temp->left = temp->right = NULL;
    return temp;
}

// Вставка узла в дерево поиска
struct Node* insert(struct Node* node, int key) {
    if (node == NULL) return newNode(key);
    if (key < node->key)
        node->left = insert(node->left, key);
    else if (key > node->key)
        node->right = insert(node->right, key);
    return node;
}

// Поиск узла с минимальным значением
struct Node* minValueNode(struct Node* node) {
    struct Node* current = node;
    while (current && current->left != NULL)
        current = current->left;
    return current;
}

// УДАЛЕНИЕ узла с заданным ключом (основная функция задания №2)
struct Node* deleteNode(struct Node* root, int key) {
    if (root == NULL) return root;

    // Ищем узел для удаления
    if (key < root->key)
        root->left = deleteNode(root->left, key);
    else if (key > root->key)
        root->right = deleteNode(root->right, key);
    else {
        // Найден узел для удаления

        // Случай 1: узел без детей или с одним ребёнком
        if (root->left == NULL) {
            struct Node* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            struct Node* temp = root->left;
            free(root);
            return temp;
        }

        // Случай 2: узел с двумя детьми
        struct Node* temp = minValueNode(root->right);
        root->key = temp->key;
        root->right = deleteNode(root->right, temp->key);
    }
    return root;
}

// Инфиксный обход (слева направо)
void inorder(struct Node* root) {
    if (root != NULL) {
        inorder(root->left);
        printf("%d ", root->key);
        inorder(root->right);
    }
}

// Печать дерева (простой инфиксный вывод)
void printTree(struct Node* root) {
    if (root == NULL) {
        printf("(пусто)\n");
        return;
    }
    printf("Инфиксный обход: ");
    inorder(root);
    printf("\n");
}

// Освобождение памяти дерева
void freeTree(struct Node* root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

// Основная функция
int main() {
    srand(time(0));

    // Создаём дерево из 20 случайных чисел от 1 до 100
    struct Node* root = NULL;
    printf("Создаём дерево из 20 случайных чисел (от 1 до 100):\n");
    for (int i = 0; i < 20; i++) {
        int val = rand() % 100 + 1;
        root = insert(root, val);
    }

    printTree(root);

    // Удаляем 10 вершин, введённых с клавиатуры
    printf("\nВведите 10 ключей для удаления:\n");
    for (int i = 0; i < 10; i++) {
        int key;
        printf("Ключ %d: ", i + 1);
        if (scanf("%d", &key) != 1) {
            printf("Некорректный ввод. Пропускаем.\n");
            while (getchar() != '\n'); // очистка буфера
            continue;
        }

        // Проверим, существует ли такой ключ
        struct Node* temp = root;
        int found = 0;
        while (temp) {
            if (temp->key == key) {
                found = 1;
                break;
            }
            temp = (key < temp->key) ? temp->left : temp->right;
        }

        if (!found) {
            printf("Ключ %d не найден в дереве.\n", key);
        } else {
            root = deleteNode(root, key);
            printf("Удалён ключ %d.\n", key);
        }

        printTree(root);
    }

    // Освобождаем память
    freeTree(root);
    return 0;
}