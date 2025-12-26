#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>

#define MAX_SYMBOLS 1024
#define MAX_TEXT_LEN 25000 

typedef struct {
    wchar_t symbol;
    double prob;
    int freq;
} Symbol;

typedef struct {
    int block_size;
    long compressed_bits;
    int success;
} TestResult;

int compare_prob(const void *a, const void *b) {
    double diff = ((Symbol*)b)->prob - ((Symbol*)a)->prob;
    return (diff > 0) - (diff < 0);
}

void double_to_binary(double val, int bits, char* buffer) {
    for (int i = 0; i < bits; i++) {
        val *= 2;
        if (val >= 1.0) {
            buffer[i] = '1';
            val -= 1.0;
        } else {
            buffer[i] = '0';
        }
    }
    buffer[bits] = '\0';
}

TestResult run_arithmetic_coding(int block_size, wchar_t *text, int text_len, double *Q, Symbol *symbols, int n_symbols, int mode) {
    TestResult result;
    result.block_size = block_size;
    result.compressed_bits = 0;
    result.success = 1;

    double l = 0.0;
    double h = 1.0;
    double r = 1.0;
    
    int chars_in_block = 0;
    int current_block_num = 1;

    for (int i = 0; i < text_len; i++) {
        wchar_t c = text[i];

        int m = -1;
        for (int j = 0; j < n_symbols; j++) {
            if (symbols[j].symbol == c) {
                m = j;
                break;
            }
        }
        
        if (m != -1) {
            double l_prev = l;
            double r_prev = r;

            l = l_prev + r_prev * Q[m];
            h = l_prev + r_prev * Q[m+1];
            r = h - l;
            
            chars_in_block++;
            if (mode == 1) {
                if (chars_in_block == 1) {
                    wprintf(L"\n--- Блок №%d ---\n", current_block_num);
                }
                wprintf(L"Символ '%lc': [%.10f, %.10f) r=%.10f\n", c, l, h, r);
            }

            if (r < 1e-15) {
                result.success = 0;
                return result; 
            }
        }
        if (chars_in_block == block_size || i == text_len - 1) {
            double code_val = (l + h) / 2.0;
            int bits = (int)ceil(-log2(r)) + 1;
            
            result.compressed_bits += bits;
            if (mode == 1) {
                char bin_code[128];
                double_to_binary(code_val, bits, bin_code);
                wprintf(L"ИТОГ БЛОКА %d: Интервал [%.10f, %.10f)\n", current_block_num, l, h);
                wprintf(L" -> Код (dec): %.10f\n", code_val);
                wprintf(L" -> Код (bin): 0.%s (%d бит)\n", bin_code, bits);
            }

            l = 0.0; h = 1.0; r = 1.0;
            chars_in_block = 0;
            current_block_num++;
        }
    }
    return result;
}

int main(void) {
    #ifdef _WIN32
    system("chcp 65001 > nul");
    system("cls");
    #else
    system("clear");
    #endif
    setlocale(LC_ALL, "");

    FILE *f = fopen("text.txt", "r, ccs=UTF-8");
    if (!f) {
        wprintf(L"Ошибка: Файл 'text.txt' не найден.\n");
        return 1;
    }

    static wchar_t text_buffer[MAX_TEXT_LEN];
    int text_len = 0;
    wint_t c;

    Symbol base[MAX_SYMBOLS];
    struct { wchar_t sym; int freq; } freq[MAX_SYMBOLS];
    int freq_count = 0;

    while ((c = fgetwc(f)) != WEOF && text_len < MAX_TEXT_LEN - 1) {
        text_buffer[text_len++] = c;
        int found = 0;
        for (int i = 0; i < freq_count; i++) {
            if (freq[i].sym == c) {
                freq[i].freq++;
                found = 1;
                break;
            }
        }
        if (!found) {
            freq[freq_count].sym = c;
            freq[freq_count].freq = 1;
            freq_count++;
        }
    }
    fclose(f);
    wprintf(L"Текст загружен: %d символов.\n", text_len);

    int n_symbols = 0;
    for (int i = 0; i < freq_count; i++) {
        base[n_symbols].symbol = freq[i].sym;
        base[n_symbols].freq = freq[i].freq;
        base[n_symbols].prob = (double)freq[i].freq / text_len;
        n_symbols++;
    }
    // qsort(base, n_symbols, sizeof(Symbol), compare_prob);

    double Q[MAX_SYMBOLS + 1];
    Q[0] = 0.0;
    for (int i = 0; i < n_symbols; i++) {
        Q[i+1] = Q[i] + base[i].prob;
    }

    int test_sizes[] = {1, 2, 3, 4, 5, 6, 7, 8};
    int num_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);
    TestResult results[32];
    
    int best_idx = -1;
    long min_bits = -1;

    for (int i = 0; i < num_tests; i++) {
        results[i] = run_arithmetic_coding(test_sizes[i], text_buffer, text_len, Q, base, n_symbols, 0);
        
        if (results[i].success) {
            if (min_bits == -1 || results[i].compressed_bits < min_bits) {
                min_bits = results[i].compressed_bits;
                best_idx = i;
            }
        }
    }

    if (best_idx != -1) {
        int best_size = results[best_idx].block_size;
        run_arithmetic_coding(best_size, text_buffer, text_len, Q, base, n_symbols, 1);
    } else {
        wprintf(L"\nОшибка: Не удалось найти подходящий размер блока (везде потеря точности).\n");
    }

    wprintf(L"\n\n=== Таблица вероятностей ===\n");
    wprintf(L" №  | Символ | Частота | Вероятность |  Q (начало)  |  Q (конец) \n");
    wprintf(L"----|--------|---------|-------------|--------------|------------\n");
    for (int i = 0; i < n_symbols; i++) {
        wchar_t d = base[i].symbol;
        
        wprintf(L"%3d |   '%lc'  |  %5d  |  %.6f   |   %.6f   |  %.6f\n", 
                i+1, d, base[i].freq, base[i].prob, Q[i], Q[i+1]);
    }
    wprintf(L"----|--------|---------|-------------|--------------|------------\n");

    long orig_bits = text_len * 8; 
    
    wprintf(L"\n=== Таблица сравнения размеров ===\n");
    wprintf(L"| Блок | Сжатый (бит) | Коэфф. сжатия | %% от исх. | Статус |\n");
    wprintf(L"|------|--------------|---------------|------------|--------|\n");

    for (int i = 0; i < num_tests; i++) {
        if (results[i].success) {
            double ratio = (double)orig_bits / results[i].compressed_bits;
            double percent = (double)results[i].compressed_bits / orig_bits * 100.0;
            
            wprintf(L"| %4d | %12ld | %13.4f | %9.2f%% | OK     |\n", 
                    results[i].block_size, results[i].compressed_bits, ratio, percent);
        } else {
            double ratio = (double)orig_bits / results[i].compressed_bits;
            double percent = (double)results[i].compressed_bits / orig_bits * 100.0;
            wprintf(L"| %4d | %12ld | %13.4f | %9.2f%% | loss   |\n", 
                    results[i].block_size, results[i].compressed_bits, ratio, percent);
        }
    }
    wprintf(L"===========================================================\n");
    double entropy = 0.0;
    for (int i = 0; i < n_symbols; i++) {
        if (base[i].prob > 0) entropy -= base[i].prob * log2(base[i].prob);
    }
    wprintf(L"Энтропия источника: %.4f бит/символ\n", entropy);

    return 0;
}