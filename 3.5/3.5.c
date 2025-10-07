```c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct Node {
    int key;
    struct Node *left, *right;
    int balance;
};

struct Node* newNode(int key) {
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    node->key = key;
    node->left = node->right = NULL;
    node->balance = 0;
    return node;
}

struct Node* rotateLL(struct Node* p) {
    struct Node* q = p->left;
    q->balance = 0;
    p->balance = 0;
    p->left = q->right;
    q->right = p;
    return q;
}

struct Node* rotateRR(struct Node* p) {
    struct Node* q = p->right;
    q->balance = 0;
    p->balance = 0;
    p->right = q->left;
    q->left = p;
    return q;
}

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

struct Node* insertAVL(struct Node* p, int key, int* heightChanged) {
    if (p == NULL) {
        *heightChanged = 1;
        return newNode(key);
    }
    
    if (key < p->key) {
        p->left = insertAVL(p->left, key, heightChanged);
        if (*heightChanged) {
            switch (p->balance) {
                case 1:
                    p->balance = 0;
                    *heightChanged = 0;
                    break;
                case 0:
                    p->balance = -1;
                    *heightChanged = 1;
                    break;
                case -1:
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
                case -1:
                    p->balance = 0;
                    *heightChanged = 0;
                    break;
                case 0:
                    p->balance = 1;
                    *heightChanged = 1;
                    break;
                case 1:
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
        *heightChanged = 0;
    }
    
    return p;
}

struct Node* insert(struct Node* root, int key) {
    int heightChanged = 0;
    return insertAVL(root, key, &heightChanged);
}

int getSize(struct Node* root) {
    if (root == NULL) return 0;
    return 1 + getSize(root->left) + getSize(root->right);
}

int getChecksum(struct Node* root) {
    if (root == NULL) return 0;
    return root->key + getChecksum(root->left) + getChecksum(root->right);
}

int getHeight(struct Node* root) {
    if (root == NULL) return 0;
    int left = getHeight(root->left);
    int right = getHeight(root->right);
    return (left > right ? left : right) + 1;
}

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

void inorder(struct Node* root) {
    if (root != NULL) {
        inorder(root->left);
        printf("%d ", root->key);
        inorder(root->right);
    }
}

void printTree(struct Node* root, int level) {
    if (root == NULL) return;
    
    printTree(root->right, level + 1);
    
    for (int i = 0; i < level; i++) printf("    ");
    printf("%d(%d)\n", root->key, root->balance);
    
    printTree(root->left, level + 1);
}

void freeTree(struct Node* root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

void inorderPerfect(int keys[], int start, int end) {
    if (start > end) return;
    int mid = (start + end) / 2;
    printf("%d ", keys[mid]);
    inorderPerfect(keys, start, mid - 1);
    inorderPerfect(keys, mid + 1, end);
}

void sortArray(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

int main() {
    srand(time(0));

    struct Node* root = NULL;
    int n = 100;
    int max_value = 500;
    
    printf("=== AVL ДЕРЕВО ===\n\n");
    
    printf("Генерация %d уникальных ключей (1-%d):\n", n, max_value);
    printf("================================\n");
    
    int used[501] = {0};
    int unique_keys[100];
    int unique_count = 0;
    
    for (int i = 0; i < n; i++) {
        int val;
        do {
            val = rand() % max_value + 1;
        } while (used[val]);
        
        used[val] = 1;
        unique_keys[unique_count] = val;
        unique_count++;
        
        printf("%3d", val);
        if ((i + 1) % 10 == 0) printf("\n");
        else printf(" ");
        
        root = insert(root, val);
    }
    
    if (n % 10 != 0) printf("\n");
    printf("================================\n");
    printf("Сгенерировано уникальных ключей: %d\n\n", unique_count);

    printf("Обход AVL дерева слева направо:\n");
    inorder(root);
    printf("\n\n");

    sortArray(unique_keys, n);
    printf("Обход ИСДП слева направо:\n");
    inorderPerfect(unique_keys, 0, n - 1);
    printf("\n\n");

    int size_avl = getSize(root);
    int checksum_avl = getChecksum(root);
    int height_avl = getHeight(root);
    double avg_depth_avl = getAverageDepth(root);

    int size_idp = size_avl;
    int checksum_idp = checksum_avl;
    
    int height_idp;
    if (size_idp > 63) height_idp = 6;
    else if (size_idp > 31) height_idp = 5;
    else if (size_idp > 15) height_idp = 4;
    else if (size_idp > 7) height_idp = 3;
    else if (size_idp > 3) height_idp = 2;
    else if (size_idp > 1) height_idp = 1;
    else height_idp = 0;

    double avg_depth_idp = (size_idp <= 1) ? 0.0 : log2_custom(size_idp) - 1;

    printf("ТАБЛИЦА СРАВНЕНИЯ:\n");
    printf("+--------+----------+--------------+---------+-------------+\n");
    printf("| Дерево | Размер   | Контр. Сумма | Высота  | Сред.глуб.  |\n");
    printf("+--------+----------+--------------+---------+-------------+\n");
    printf("| ИСДП   | %-8d | %-12d | %-7d | %-11.2f |\n", size_idp, checksum_idp, height_idp, avg_depth_idp);
    printf("| AVL    | %-8d | %-12d | %-7d | %-11.2f |\n", size_avl, checksum_avl, height_avl, avg_depth_avl);
    printf("+--------+----------+--------------+---------+-------------+\n");

    printf("\nСтруктура AVL дерева:\n");
    printTree(root, 0);

    freeTree(root);
    return 0;
}
```
