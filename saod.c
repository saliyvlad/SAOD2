#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGE_SIZE 20

typedef struct record
{
    char depositor[30];     // ФИО вкладчика (30 символов)
    unsigned short amount;  // Сумма вклада (unsigned short int)
    char date[10];          // Дата вклада (10 символов)
    char lawyer[22];        // ФИО адвоката (22 символа)
} record;

// Функция сравнения для сортировки по ФИО адвоката и сумме вклада
int compare_records(const void *a, const void *b) {
    const record *rec_a = (const record *)a;
    const record *rec_b = (const record *)b;
    
    // Сравниваем ФИО адвоката
    int lawyer_cmp = strncmp(rec_a->lawyer, rec_b->lawyer, 22);
    if (lawyer_cmp != 0) {
        return lawyer_cmp;
    }
    
    // Если ФИО адвоката совпадают, сравниваем сумму вклада
    if (rec_a->amount < rec_b->amount) return -1;
    if (rec_a->amount > rec_b->amount) return 1;
    return 0;
}

// Функция для получения первых трех букв фамилии адвоката
void get_lawyer_surname_prefix(const char *lawyer, char *prefix) {
    // Копируем первые 3 символа
    strncpy(prefix, lawyer, 3);
    prefix[3] = '\0';
}

// Функция бинарного поиска по первым трем буквам фамилии адвоката
void search_by_lawyer_prefix(record *DB, int total_records, const char *search_prefix) {
    printf("\nРезультаты поиска по адвокату \"%s\":\n", search_prefix);
    printf("-------------------------------------------------------------------------------\n");
    
    int found = 0;
    for (int i = 0; i < total_records; i++) {
        char current_prefix[4];
        get_lawyer_surname_prefix(DB[i].lawyer, current_prefix);
        
        if (strncmp(current_prefix, search_prefix, 3) == 0) {
            printf("%-30s %-6hu %-10s %-22s\n", 
                   DB[i].depositor, 
                   DB[i].amount, 
                   DB[i].date, 
                   DB[i].lawyer);
            found++;
        }
    }
    
    if (found == 0) {
        printf("Адвокатов с фамилией, начинающейся на \"%s\", не найдено.\n", search_prefix);
    } else {
        printf("\nНайдено записей: %d\n", found);
    }
}

int main()
{
    // Устанавливаем кодировку консоли
    system("chcp 866 > nul");
    
    FILE *fp;
    fp = fopen("testBase3.dat", "rb");

    if (fp == NULL) {
        printf("Ошибка открытия файла!\n");
        return 1;
    }

    record DB[4000] = {0};
    int total_records = fread(DB, sizeof(record), 4000, fp);
    fclose(fp);

    printf("Загружено записей: %d\n", total_records);

    // СОРТИРОВКА по ФИО адвоката и сумме вклада
    printf("Сортируем данные по ФИО адвоката и сумме вклада...\n");
    qsort(DB, total_records, sizeof(record), compare_records);
    printf("Сортировка завершена.\n");

    int page = 0;
    char command;
    int max_page = (total_records - 1) / PAGE_SIZE;

    do {
        system("cls");
        
        int start = page * PAGE_SIZE;
        int end = start + PAGE_SIZE;
        if (end > total_records) end = total_records;
        
        system("chcp 65001 > nul");
        printf("Страница %d/%d\n", page + 1, max_page + 1);
        printf("%-30s             %-6s    %-10s   %-22s\n", 
               "ФИО вкладчика", "Сумма", "Дата", "Адвокат");
        printf("-------------------------------------------------------------------------------\n");
        
        system("chcp 866 > nul");
        for (int i = start; i < end; ++i)
        {
            printf("%-30s %-6hu %-10s %-22s\n", 
                   DB[i].depositor, 
                   DB[i].amount, 
                   DB[i].date, 
                   DB[i].lawyer);
        }
        
        system("chcp 65001 > nul");
        printf("\nПоказаны записи %d–%d из %d\n", start + 1, end, total_records);
        printf("Команды: [n] +1  [p] -1  [N] +10  [P] -10  [s] поиск  [q] выход: ");
        command = getchar();

        // Очистить буфер ввода
        while (getchar() != '\n');
        system("chcp 866 > nul");

        
if (command == 'n') {  // следующая страница (+1)
            if (page < max_page)
                page++;
        }
        else if (command == 'p') {  // предыдущая страница (-1)
            if (page > 0)
                page--;
        }
        else if (command == 'N') {  // +10 страниц (Shift + n)
            page += 10;
            if (page > max_page) page = max_page;
        }
        else if (command == 'P') {  // -10 страниц (Shift + p)
            page -= 10;
            if (page < 0) page = 0;
        }
        else if (command == 's' || command == 'S') {
            // Поиск по первым трем буквам фамилии адвоката
            system("chcp 65001 > nul");
            printf("\nВведите первые 3 буквы фамилии адвоката для поиска: ");
            char search_prefix[4];
            scanf("%3s", search_prefix);
            while (getchar() != '\n'); // Очистка буфера
            
            system("chcp 866 > nul");
            search_by_lawyer_prefix(DB, total_records, search_prefix);
            
            printf("\nНажмите Enter для продолжения...");
            getchar();
        }

    } while (command != 'q' && command != 'Q');

    return 0;
}