#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BITS 256

int get_log2_floor(int n) {
    if (n <= 0) return 0;
    int order = 0;
    while (n > 1) {
        n >>= 1;
        order++;
    }
    return order;
}

int get_full_bit_length(int n) {
    if (n <= 0) return 0;
    return get_log2_floor(n) + 1;
}

void to_bin_fixed(int value, int bits, char* out_buf) {
    for (int i = bits - 1; i >= 0; i--) {
        char bit = ((value >> i) & 1) ? '1' : '0';
        *out_buf = bit;
        out_buf++;
    }
    *out_buf = '\0';
}

void get_mantissa(int n, int order, char* out_buf) {
    for (int i = order - 1; i >= 0; i--) {
        *out_buf = ((n >> i) & 1) ? '1' : '0';
        out_buf++;
    }
    *out_buf = '\0';
}

void code_fixed_variable(int n, char* result) {
    int full_len = get_full_bit_length(n);
    char order_part[10];
    to_bin_fixed(full_len, 4, order_part);
    char mantissa_part[33];
    get_mantissa(n, full_len - 1, mantissa_part);
    
    if (strlen(mantissa_part) == 0) {
        strcpy(result, order_part);
    } else {
        sprintf(result, "%s %s", order_part, mantissa_part);
    }
}

void code_elias_gamma(int n, char* result) {
    if (n == 0) { strcpy(result, "-"); return; }
    int order = get_log2_floor(n);
    int idx = 0;
    for (int i = 0; i < order; i++) {
        result[idx++] = '0';
    }
    result[idx++] = '1';
    char mantissa[33];
    get_mantissa(n, order, mantissa);
    
    strcpy(&result[idx], mantissa);
}
void code_elias_omega(int n, char* result) {
    if (n == 0) { strcpy(result, "-"); return; }
    
    if (n == 1) {
        strcpy(result, "0");
        return;
    }

    char buffer[MAX_BITS] = "0"; 
    char temp_prefix[MAX_BITS];
    char bin_val[64];
    
    int current = n;
    
    while (current > 1) {
        int len = get_full_bit_length(current);
        to_bin_fixed(current, len, bin_val);
        strcpy(temp_prefix, bin_val);
        strcat(temp_prefix, " "); 
        strcat(temp_prefix, buffer);
        strcpy(buffer, temp_prefix);
        
        current = len - 1;
    }
    
    strcpy(result, buffer);
}

int main() {
    int max_num = 256;
    char fv[64], gamma[64], omega[64];
    printf("===============================================================================\n");
    printf("| %-5s | %-18s | %-20s | %-20s |\n", "N", "Fixed+Var", "Gamma", "Omega");
    printf("|-------|--------------------|----------------------|----------------------|\n");

    for (int i = 0; i <= max_num; i++) {
        code_fixed_variable(i, fv);
        code_elias_gamma(i, gamma);
        code_elias_omega(i, omega);
        
        printf("| %-5d | %-18s | %-20s | %-20s |\n", i, fv, gamma, omega);
    }
    
    printf("===============================================================================\n");
    return 0;

}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_BLOCK_SIZE 1024 // Максимальный размер блока (экспериментально определён)
#define PRECISION 32        // Точность для арифметического кодирования

typedef struct {
    unsigned char symbol;
    double low;
    double high;
} SymbolRange;

// Функция для вычисления частот символов
void calculate_frequencies(unsigned char *data, int len, double freq[256]) {
    int i;
    for (i = 0; i < 256; i++) freq[i] = 0.0;
    for (i = 0; i < len; i++) freq[data[i]] += 1.0;
    for (i = 0; i < 256; i++) freq[i] /= len;
}

// Построение таблицы интервалов
void build_intervals(double freq[256], SymbolRange intervals[256], int *num_symbols) {
    int i, j = 0;
    double cum_prob = 0.0;
    *num_symbols = 0;
    for (i = 0; i < 256; i++) {
        if (freq[i] > 0.0) {
            intervals[j].symbol = (unsigned char)i;
            intervals[j].low = cum_prob;
            cum_prob += freq[i];
            intervals[j].high = cum_prob;
            (*num_symbols)++;
            j++;
        }
    }
}

// Поиск символа по интервалу
int find_symbol(SymbolRange intervals[], int num_symbols, double value) {
    int i;
    for (i = 0; i < num_symbols; i++) {
        if (value >= intervals[i].low && value < intervals[i].high) {
            return i;
        }
    }
    return -1;
}

// Арифметическое кодирование одного блока
double arithmetic_encode_block(unsigned char *block, int block_len, SymbolRange intervals[], int num_symbols) {
    double low = 0.0, high = 1.0;
    int i;
    for (i = 0; i < block_len; i++) {
        double range = high - low;
        int sym_idx = -1;
        for (int j = 0; j < num_symbols; j++) {
            if (intervals[j].symbol == block[i]) {
                sym_idx = j;
                break;
            }
        }
        if (sym_idx == -1) continue;
        high = low + range * intervals[sym_idx].high;
        low = low + range * intervals[sym_idx].low;
        printf("Блок %d: Интервал [%f, %f), Символ %c\n", i+1, low, high, block[i]);
    }
    return (low + high) / 2.0;
}

// Арифметическое декодирование
void arithmetic_decode_block(double code, int block_len, SymbolRange intervals[], int num_symbols, unsigned char *decoded) {
    double low = 0.0, high = 1.0;
    int i;
    for (i = 0; i < block_len; i++) {
        double range = high - low;
        double value = (code - low) / range;
        int sym_idx = find_symbol(intervals, num_symbols, value);
        if (sym_idx == -1) {
            decoded[i] = '?';
            continue;
        }
        decoded[i] = intervals[sym_idx].symbol;
        high = low + range * intervals[sym_idx].high;
        low = low + range * intervals[sym_idx].low;
        printf("Декодировано: %c (Интервал [%f, %f))\n", decoded[i], low, high);
    }
}

// Основная функция для ЛР11
void lab11() {
    FILE *input_file = fopen("input.txt", "rb");
    if (!input_file) {
        printf("Ошибка открытия входного файла!\n");
        return;
    }

    // Получаем размер файла
    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
    rewind(input_file);

    if (file_size < 1024) {
        printf("Файл должен быть не менее 1 Кбайта (1024 байт). Увеличьте файл.\n");
        fclose(input_file);
        return;
    }

    unsigned char *file_data = (unsigned char*)malloc(file_size);
    fread(file_data, 1, file_size, input_file);
    fclose(input_file);

    // Экспериментальное определение максимального размера блока без потери точности
    int max_safe_block_size = 1;
    double freq[256];
    SymbolRange intervals[256];
    int num_symbols;

    for (int block_size = 1; block_size <= MAX_BLOCK_SIZE; block_size++) {
        if (block_size > file_size) break;

        calculate_frequencies(file_data, block_size, freq);
        build_intervals(freq, intervals, &num_symbols);

        double encoded_val = arithmetic_encode_block(file_data, block_size, intervals, num_symbols);
        unsigned char *decoded_block = (unsigned char*)malloc(block_size);
        arithmetic_decode_block(encoded_val, block_size, intervals, num_symbols, decoded_block);

        // Проверяем, совпадает ли декодированный блок с оригиналом
        int match = 1;
        for (int i = 0; i < block_size; i++) {
            if (file_data[i] != decoded_block[i]) {
                match = 0;
                break;
            }
        }

        free(decoded_block);

        if (match) {
            max_safe_block_size = block_size;
        } else {
            break;
        }
    }

    printf("\nМаксимальный размер блока без потери точности: %d байт\n", max_safe_block_size);

    // Закодируем весь файл блоками
    FILE *encoded_file = fopen("encoded.bin", "wb");
    FILE *decoded_file = fopen("decoded.txt", "wb");

    int total_blocks = (file_size + max_safe_block_size - 1) / max_safe_block_size;
    double *codes = (double*)malloc(total_blocks * sizeof(double));

    for (int i = 0; i < total_blocks; i++) {
        int start = i * max_safe_block_size;
        int block_len = (start + max_safe_block_size <= file_size) ? max_safe_block_size : file_size - start;
        unsigned char *block = file_data + start;

        calculate_frequencies(block, block_len, freq);
        build_intervals(freq, intervals, &num_symbols);

        codes[i] = arithmetic_encode_block(block, block_len, intervals, num_symbols);

        // Сохраняем код и метаданные (длина блока и количество символов)
        fwrite(&block_len, sizeof(int), 1, encoded_file);
        fwrite(&num_symbols, sizeof(int), 1, encoded_file);
        for (int j = 0; j < num_symbols; j++) {
            fwrite(&intervals[j].symbol, sizeof(unsigned char), 1, encoded_file);
            fwrite(&intervals[j].low, sizeof(double), 1, encoded_file);
            fwrite(&intervals[j].high, sizeof(double), 1, encoded_file);
        }
        fwrite(&codes[i], sizeof(double), 1, encoded_file);
    }

    fclose(encoded_file);

    // Декодирование
    FILE *decode_in = fopen("encoded.bin", "rb");
    if (!decode_in) {
        printf("Ошибка открытия закодированного файла!\n");
        free(codes);
        free(file_data);
        return;
    }

    for (int i = 0; i < total_blocks; i++) {
        int block_len, num_syms;
        fread(&block_len, sizeof(int), 1, decode_in);
        fread(&num_syms, sizeof(int), 1, decode_in);
        SymbolRange dec_intervals[256];
        for (int j = 0; j < num_syms; j++) {
            fread(&dec_intervals[j].symbol, sizeof(unsigned char), 1, decode_in);
            fread(&dec_intervals[j].low, sizeof(double), 1, decode_in);
            fread(&dec_intervals[j].high, sizeof(double), 1, decode_in);
        }
        double code_val;
        fread(&code_val, sizeof(double), 1, decode_in);

        unsigned char *decoded_block = (unsigned char*)malloc(block_len);
        arithmetic_decode_block(code_val, block_len, dec_intervals, num_syms, decoded_block);
        fwrite(decoded_block, 1, block_len, decoded_file);
        free(decoded_block);
    }

    fclose(decode_in);
    fclose(decoded_file);

    // Оценка коэффициента сжатия
    FILE *enc_file = fopen("encoded.bin", "rb");
    fseek(enc_file, 0, SEEK_END);
    long encoded_size = ftell(enc_file);
    fclose(enc_file);

    double compression_ratio = (double)encoded_size / file_size * 100.0;
    printf("\nКоэффициент сжатия: %.2f%%\n", compression_ratio);

    // Определение зависимости коэффициента сжатия от длины блока
    printf("\nЗависимость коэффициента сжатия от длины блока:\n");
    for (int bs = 1; bs <= max_safe_block_size; bs += 10) {
        if (bs > file_size) break;
        calculate_frequencies(file_data, bs, freq);
        build_intervals(freq, intervals, &num_symbols);
        double enc_val = arithmetic_encode_block(file_data, bs, intervals, num_symbols);
        // Размер кода — 8 байт (double) + метаданные
        long coded_size = 8 + 4 + 4 + num_symbols * (1 + 8 + 8); // int + int + symbols * (char + 2*double)
        double ratio = (double)coded_size / bs * 100.0;
        printf("Блок %d байт: %.2f%%\n", bs, ratio);
    }

    free(codes);
    free(file_data);
    printf("\nГотово! Результаты сохранены в encoded.bin и decoded.txt\n");
}
