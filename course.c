#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#define PAGE_SIZE 20

typedef struct record
{
    char depositor[31];     // ФИО вкладчика (30 символов + 1 на '\0')
    unsigned short amount;  // Сумма вклада (unsigned short int)
    char date[11];          // Дата вклада (10 символов + 1 на '\0')
    char lawyer[23];        // ФИО адвоката (22 символа + 1 на '\0')
} record;

int main()
{
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

    int page = 0;
    char command;

    do {
        system("cls");
        
        int start = page * PAGE_SIZE;
        int end = start + PAGE_SIZE;
        if (end > total_records) end = total_records;
        system("chcp 65001 > nul");
        printf("%-30s %-6hu %-10s %-22s\n", 
               "Фио", "Сумма", "Дата", "Адвокат");
        printf("-------------------------------------------------------------------------------\n");
        system("chcp 866 > nul");
        for (int i = start; i < end; ++i)
        {
            printf("%-32s %-18s %-6hd %-10hd\n", 
                   DB[i].depositor, 
                   DB[i].amount, 
                   DB[i].date, 
                   DB[i].lawyer);
        }
        //setlocale(LC_ALL, "Russian");
        system("chcp 65001 > nul");
        printf("\nПоказаны записи %d–%d из %d\n", start + 1, end, total_records);
        printf("[n] - следующая, [p] - предыдущая, [q] - выход: ");
        command = getchar();

        // Очистить буфер ввода (если пользователь ввёл более 1 символа)
        while (getchar() != '\n');
        system("chcp 866 > nul");
        if (command == 'n' || command == 'N') {
            if ((page + 1) * PAGE_SIZE < total_records)
                page++;
        }
        else if (command == 'p' || command == 'P') {
            if (page > 0)
                page--;
        }

    } while (command != 'q' && command != 'Q');

    return 0;
}