#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#define N 100

typedef struct Vertex
{
    int Data;
    struct Vertex *Left;
    struct Vertex *Right;
} Vertex;

Vertex *create_vertex(int Data)
{
    Vertex *newVertex = (Vertex *)malloc(sizeof(Vertex));
    newVertex->Data = Data;
    newVertex->Left = NULL;
    newVertex->Right = NULL;
    return newVertex;
}

void Left_to_Right(Vertex *p)
{
    if (p != NULL)
    {
        Left_to_Right(p->Left);
        printf("%d ", p->Data);
        Left_to_Right(p->Right);
    }
}

int Search(Vertex *p, int X)
{
    while (p != NULL)
    {
        if (X < p->Data)
        {
            p = p->Left;
        }
        else if (X > p->Data)
        {
            p = p->Right;
        }
        else
            break;
    }
    return (p != NULL);
}

int Size(Vertex *p)
{
    if (p == NULL)
    {
        return 0;
    }
    else
    {
        return 1 + Size(p->Left) + Size(p->Right);
    }
}

int CheckSum(Vertex *p)
{
    if (p == NULL)
    {
        return 0;
    }
    else
    {
        return p->Data + CheckSum(p->Left) + CheckSum(p->Right);
    }
}

int Height(Vertex *p)
{
    if (p == NULL)
    {
        return 0;
    }
    else
    {
        int LeftHeight = Height(p->Left);
        int RightHeight = Height(p->Right);
        return 1 + (LeftHeight > RightHeight ? LeftHeight : RightHeight);
    }
}

int PathLengthSum(Vertex *p, int level)
{
    if (p == NULL)
    {
        return 0;
    }
    else
    {
        return level +
               PathLengthSum(p->Left, level + 1) +
               PathLengthSum(p->Right, level + 1);
    }
}

float AverageHeight(Vertex *root)
{
    if (root == NULL)
    {
        return 0.0f;
    }
    int total_path_length = PathLengthSum(root, 1);
    int tree_size = Size(root);
    return (float)total_path_length / tree_size;
}

void generator(int arr[], int n)
{
    srand(time(NULL));
    printf("\033[34mГенерация случайной последовательности:\033[0m ");
    int count = 0;
    while (count < n)
    {
        int num = rand() % 1001;
        int isUnique = 1;
        for (int i = 0; i < count; i++)
        {
            if (arr[i] == num)
            {
                isUnique = 0;
                break;
            }
        }
        if (isUnique)
        {
            arr[count] = num;
            count++;
            printf("%d ", num);
        }
    }
}

void PrintMas(int n, int A[])
{
    for (int i = 0; i < n; i++)
    {
        printf("%d ", A[i]);
    }
    printf("\n");
}

int insertSort(int n, int A[])
{
    int C = 0;
    int M = 0;

    for (int i = 1; i < n; i++)
    {
        int t = A[i];
        int j = i - 1;
        M++;

        while (j >= 0 && t < A[j])
        {
            C++;
            A[j + 1] = A[j];
            M++;
            j--;
        }
        if (j >= 0)
        {
            C++;
        }

        A[j + 1] = t;
        M++;
    }
    int trud = M + C;
    return trud;
}

Vertex *BuildISDP(int L, int R, int A[])
{
    if (L > R)
    {
        return NULL;
    }
    else
    {
        int m = (L + R) / 2;
        Vertex *p = (Vertex *)malloc(sizeof(Vertex));
        p->Data = A[m];
        p->Left = BuildISDP(L, m - 1, A);
        p->Right = BuildISDP(m + 1, R, A);
        return p;
    }
}

void add_DoubleSDP(Vertex **p, int Data)
{
    while (*p != NULL)
    {
        if (Data < (*p)->Data)
        {
            p = &((*p)->Left);
        }
        else if (Data > (*p)->Data)
        {
            p = &((*p)->Right);
        }
        else
        {
            return;
        }
    }
    if (*p == NULL)
    {
        *p = (Vertex *)malloc(sizeof(Vertex));
        (*p)->Data = Data;
        (*p)->Left = NULL;
        (*p)->Right = NULL;
    }
}

Vertex *add_RecursiveSDP(Vertex *p, int Data)
{
    if (p == NULL)
    {
        p = (Vertex *)malloc(sizeof(Vertex));
        p->Data = Data;
        p->Left = NULL;
        p->Right = NULL;
    }
    else if (Data < p->Data)
    {
        p->Left = add_RecursiveSDP(p->Left, Data);
    }
    else if (Data > p->Data)
    {
        p->Right = add_RecursiveSDP(p->Right, Data);
    }
    return p;
}

void PrintStatString(const char *name, Vertex *p)
{
    printf("| %-20s | %-10d | %-15d | %-10d | %-15.2f |\n",
           name,
           Size(p),
           CheckSum(p),
           Height(p),
           AverageHeight(p));
    printf("--------------------------------------------------------------------------------------\n");
}

int main()
{
    int initial[N];
    int sorted[N];

    generator(initial, N);
    for (int i = 0; i < N; i++)
    {
        sorted[i] = initial[i];
    }
    insertSort(N, sorted);
    printf("\033[34m \n\nОтсортированная последовательность: \033[0m");
    PrintMas(N, sorted);

    Vertex *root = BuildISDP(0, 99, sorted);
    printf("\033[34m \nОбход для идеально сбалансированного дерева поиска: \033[0m");
    Left_to_Right(root);

    Vertex *SDP1Root = NULL;
    for (int i = 0; i < N; i++)
    {
        add_DoubleSDP(&SDP1Root, initial[i]);
    }
    printf("\033[34m \n \nОбход для случайного дерева поиска с двойной косвенностью: \033[0m");
    Left_to_Right(SDP1Root);

    Vertex *SDP2Root = NULL;
    for (int i = 0; i < N; i++)
    {
        SDP2Root = add_RecursiveSDP(SDP2Root, initial[i]);
    }
    printf("\033[34m \n \nОбход для случайного дерева поиска построенного рекурсивно: \033[0m");
    Left_to_Right(SDP2Root);

    printf("\n\nСтатистика деревьев:\n");
    printf("--------------------------------------------------------------------------------------\n");
    printf("| %-20s | %-10s | %-15s | %-10s | %-15s |\n",
           "Тип дерева", "Размер", "Контр. сумма", "Высота", "Ср. высота");
    printf("--------------------------------------------------------------------------------------\n");

    PrintStatString("ИСДП", root);
    PrintStatString("СДП (двойная)", SDP1Root);
    PrintStatString("СДП (рекурсия)", SDP2Root);
    return 0;
}