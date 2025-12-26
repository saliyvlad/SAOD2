#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_BLOCK_SIZE 1024
#define PRECISION_BITS 52  // double имеет ~52 бита мантиссы

typedef struct {
    unsigned char symbol;
    double low;
    double high;
} SymbolRange;

void calculate_frequencies(unsigned char *data, int len, double freq[256]) {
    int i;
    for (i = 0; i < 256; i++) freq[i] = 0.0;
    for (i = 0; i < len; i++) freq[data[i]] += 1.0;
    if (len > 0) {
        for (i = 0; i < 256; i++) freq[i] /= (double)len;
    }
}

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

int find_symbol(SymbolRange intervals[], int num_symbols, double value) {
    int i;
    for (i = 0; i < num_symbols; i++) {
        if (value >= intervals[i].low && value < intervals[i].high) {
            return i;
        }
    }
    return -1;
}

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
        if (sym_idx == -1) {
            continue;
        }
        high = low + range * intervals[sym_idx].high;
        low = low + range * intervals[sym_idx].low;
        printf("Блок %d: Интервал [%.6f, %.6f), Символ (код=%u, hex=%02X)\n",
               i + 1, low, high, block[i], block[i]);
    }
    return (low + high) / 2.0;
}

void arithmetic_decode_block(double code, int block_len, SymbolRange intervals[], int num_symbols, unsigned char *decoded) {
    double low = 0.0, high = 1.0;
    int i;
    for (i = 0; i < block_len; i++) {
        double range = high - low;
        double value = (code - low) / range;
        int sym_idx = find_symbol(intervals, num_symbols, value);
        if (sym_idx == -1) {
            decoded[i] = 0; // или 0xFF — неизвестный символ
            printf("Декодировано: неизвестный символ (код=???) (Интервал [%.6f, %.6f))\n", low, high);
            continue;
        }
        decoded[i] = intervals[sym_idx].symbol;
        printf("Декодировано: код=%u (hex=%02X) (Интервал [%.6f, %.6f))\n",
               decoded[i], decoded[i], low, high);
        high = low + range * intervals[sym_idx].high;
        low = low + range * intervals[sym_idx].low;
    }
}

void lab11() {
    FILE *input_file = fopen("input.txt", "rb");
    if (!input_file) {
        printf("Ошибка: не удалось открыть input.txt\n");
        return;
    }

    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
    rewind(input_file);

    if (file_size < 1024) {
        printf("Файл должен быть не менее 1024 байт. Текущий размер: %ld\n", file_size);
        fclose(input_file);
        return;
    }

    unsigned char *file_data = (unsigned char *)malloc(file_size);
    fread(file_data, 1, file_size, input_file);
    fclose(input_file);

    // Найти максимальный безопасный размер блока
    int max_safe_block_size = 1;
    double freq[256];
    SymbolRange intervals[256];
    int num_symbols;

    for (int block_size = 1; block_size <= MAX_BLOCK_SIZE && block_size <= file_size; block_size++) {
        calculate_frequencies(file_data, block_size, freq);
        build_intervals(freq, intervals, &num_symbols);

        double encoded_val = arithmetic_encode_block(file_data, block_size, intervals, num_symbols);

        unsigned char *decoded_block = (unsigned char *)malloc(block_size);
        arithmetic_decode_block(encoded_val, block_size, intervals, num_symbols, decoded_block);

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

    // Кодирование всего файла
    FILE *encoded_file = fopen("encoded.bin", "wb");
    if (!encoded_file) {
        printf("Ошибка создания encoded.bin\n");
        free(file_data);
        return;
    }

    int total_blocks = (file_size + max_safe_block_size - 1) / max_safe_block_size;

    for (int i = 0; i < total_blocks; i++) {
        int start = i * max_safe_block_size;
        int block_len = (start + max_safe_block_size <= file_size) ? max_safe_block_size : (int)(file_size - start);
        unsigned char *block = file_data + start;

        calculate_frequencies(block, block_len, freq);
        build_intervals(freq, intervals, &num_symbols);

        double code_val = arithmetic_encode_block(block, block_len, intervals, num_symbols);

        fwrite(&block_len, sizeof(int), 1, encoded_file);
        fwrite(&num_symbols, sizeof(int), 1, encoded_file);
        for (int j = 0; j < num_symbols; j++) {
            fwrite(&intervals[j].symbol, sizeof(unsigned char), 1, encoded_file);
            fwrite(&intervals[j].low, sizeof(double), 1, encoded_file);
            fwrite(&intervals[j].high, sizeof(double), 1, encoded_file);
        }
        fwrite(&code_val, sizeof(double), 1, encoded_file);
    }

    fclose(encoded_file);

    // Декодирование
    FILE *decode_in = fopen("encoded.bin", "rb");
    FILE *decoded_file = fopen("decoded.txt", "wb");
    if (!decode_in || !decoded_file) {
        printf("Ошибка открытия файлов для декодирования\n");
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

        unsigned char *decoded_block = (unsigned char *)malloc(block_len);
        arithmetic_decode_block(code_val, block_len, dec_intervals, num_syms, decoded_block);
        fwrite(decoded_block, 1, block_len, decoded_file);
        free(decoded_block);
    }

    fclose(decode_in);
    fclose(decoded_file);

    // Коэффициент сжатия
    FILE *enc_f = fopen("encoded.bin", "rb");
    fseek(enc_f, 0, SEEK_END);
    long encoded_size = ftell(enc_f);
    fclose(enc_f);

    double compression_ratio = (double)encoded_size / file_size * 100.0;
    printf("\nКоэффициент сжатия: %.2f%%\n", compression_ratio);

    // Зависимость от длины блока
    printf("\nЗависимость коэффициента сжатия от длины блока:\n");
    for (int bs = 1; bs <= max_safe_block_size; bs += 10) {
        if (bs > file_size) break;
        calculate_frequencies(file_data, bs, freq);
        build_intervals(freq, intervals, &num_symbols);
        double enc_val = arithmetic_encode_block(file_data, bs, intervals, num_symbols);
        // Оценка размера: блок_len (int) + num_sym (int) + sym table + double
        long size_est = sizeof(int) + sizeof(int) +
                        num_symbols * (sizeof(unsigned char) + 2 * sizeof(double)) +
                        sizeof(double);
        double ratio = (double)size_est / bs * 100.0;
        printf("Блок %d байт: %.2f%%\n", bs, ratio);
    }

    free(file_data);
    printf("\nЛР11 завершена. Результаты: encoded.bin, decoded.txt\n");
}
