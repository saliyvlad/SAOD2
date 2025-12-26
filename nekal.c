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
    printf("| %-5s | %-18s | %-20s | %-20s |\n", "N", "Fixed+Var", "Elias Gamma", "Elias Omega");
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