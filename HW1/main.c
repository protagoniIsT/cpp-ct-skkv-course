#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define MERSENNE_NUMBER 2147483647;

uint8_t width_n = 1;
uint8_t width_f = 2;

uint32_t calculate_basic_factorial(uint16_t number) {
    if (number <= 1) {
        return 1;
    }
    uint64_t factorial = 1;
    for (uint64_t i = 1; i <= number; i++) {
        factorial = (factorial * i) % MERSENNE_NUMBER;
    }
    return (uint32_t) factorial;
}

uint32_t calculate_next_factorial(uint32_t start_factorial, uint32_t start, uint32_t end) {
    if (end <= 1) {
        return 1;
    }
    uint64_t factorial = (uint64_t) start_factorial;
    for (uint64_t i = start + 1; i <= end; i++) {
        factorial = (factorial * i) % MERSENNE_NUMBER;
    }
    return (uint32_t) factorial;
}

uint8_t number_length(uint32_t number) {
    if (number == 0) return 1;
    return (uint8_t) floor(log10(number)) + 1;
}

void count_f_width(uint16_t n_start, uint16_t n_end, uint32_t curr_factorial) {
    uint8_t curr_width = number_length(curr_factorial);
    width_f = curr_width > width_f ? curr_width : width_f;
    if (n_start <= n_end) {
        for (uint32_t i = n_start + 1; i <= n_end; i++) {
            curr_factorial = calculate_next_factorial(curr_factorial, i - 1, i);
            curr_width = number_length(curr_factorial);
            width_f = curr_width > width_f ? curr_width : width_f;
        }
    } else {
        for (uint32_t i = n_start + 1; i <= UINT16_MAX; i++) {
            curr_factorial = calculate_next_factorial(curr_factorial, i - 1, i);
            curr_width = number_length(curr_factorial);
            width_f = curr_width > width_f ? curr_width : width_f;
        }
        for (uint32_t i = 0; i <= n_end; i++) {
            curr_factorial = calculate_next_factorial(curr_factorial, i - 1, i);
            curr_width = number_length(curr_factorial);
            width_f = curr_width > width_f ? curr_width : width_f;
        }
    }
    width_f += 2;
}

void count_n_width(uint16_t n_start, uint16_t n_end) {
    if (n_start <= n_end) {
        for (uint32_t i = n_start; i <= n_end; i++) {
            uint8_t curr_width = number_length(i);
            width_n = curr_width >= width_n ? curr_width : width_n;
        }
    } else {
        for (uint32_t i = n_start; i <= UINT16_MAX; i++) {
            uint8_t curr_width = number_length(i);
            width_n = curr_width >= width_n ? curr_width : width_n;
        }
        for (uint32_t i = 0; i <= n_end; i++) {
            uint8_t curr_width = number_length(i);
            width_n = curr_width >= width_n ? curr_width : width_n;
        }
    }
    width_n += 2;
}

void print_whitespaces(uint8_t amount) {
    for (uint8_t i = 1; i <= amount; i++) {
        printf(" ");
    }
}

void print_formatted_table(int8_t align, uint8_t width, uint32_t current_factorial, uint16_t current_number, uint8_t column_number) {
    uint8_t left_offset = 1, right_offset = 1;
    uint8_t current_num_width = 0;

    if (column_number == 1) {
        current_num_width = number_length(current_number);
    } else {
        current_num_width = number_length(current_factorial);
    }

    if (align == -1) {
        right_offset = width - current_num_width - left_offset;
    } else if (align == 0) {
        right_offset = (width - current_num_width) / 2;
        left_offset = width - right_offset - current_num_width;
    } else {
        left_offset = width - current_num_width - right_offset;
    }

    if (column_number == 1) {
        printf("|");
        print_whitespaces(left_offset);
        printf("%u", current_number);
        print_whitespaces(right_offset);
    } else {
        print_whitespaces(left_offset);
        printf("%u", current_factorial);
        print_whitespaces(right_offset);
        printf("|");
    }
}

void print_edging(int8_t align, uint8_t width_n, uint8_t width_f, uint8_t level) {
    printf("+");
    for (uint8_t i = 1; i <= width_n; i++) {
        printf("-");
    }
    printf("+");
    for (uint8_t i = 1; i <= width_f; i++) {
        printf("-");
    }
    printf("+");
    if (level == 1) {
        printf("\n");
        printf("|");
        if (align == -1) {
            printf(" ");
            printf("n");
            for (uint8_t i = 1; i <= width_n - 2; i++) {
                printf(" ");
            }
            printf("|");
            printf(" ");
            printf("n!");
            for (uint8_t i = 1; i <= width_f - 3; i++) {
                printf(" ");
            }
        } else if (align == 0) {
            uint8_t r1 = (width_n - 1) / 2; //1
            uint8_t r2 = (width_f - 2) / 2; // 3
            for (uint8_t i = 1; i <= (width_n - 1 - r1); i++) {
                printf(" ");
            }
            printf("n");
            for (uint8_t i = 1; i <= r1; i++) {
                printf(" ");
            }
            printf("|");
            for (uint8_t i = 1; i <= (width_f - 2 - r2); i++) {
                printf(" ");
            }
            printf("n!");
            for (uint8_t i = 1; i <= r2; i++) {
                printf(" ");
            }
        } else {
            for (uint8_t i = 1; i <= width_n - 2; i++) {
                printf(" ");
            }
            printf("n");
            printf(" ");
            printf("|");
            for (uint8_t i = 1; i <= width_f - 3; i++) {
                printf(" ");
            }
            printf("n!");
            printf(" ");
        }
        printf("|");
        printf("\n");
        printf("+");
        for (uint8_t i = 1; i <= width_n; i++) {
            printf("-");
        }
        printf("+");
        for (uint8_t i = 1; i <= width_f; i++) {
            printf("-");
        }
        printf("+");
    }
    printf("\n");
}

int main() {
    int32_t n_start_provided = 0, n_end_provided = 0;
    uint16_t n_start = 0, n_end = 0;
    int8_t align = 0;
    if (scanf("%u %u %hhd", &n_start_provided, &n_end_provided, &align) < 3 || n_start_provided < 0 || n_end_provided < 0) {
        fprintf(stderr, "Error: Invalid input format");
        return 1;
    }
    n_start = (uint16_t) n_start_provided;
    n_end = (uint16_t) n_end_provided;
	uint32_t factorial = calculate_basic_factorial(n_start - 1);
	count_f_width(n_start, n_end, calculate_next_factorial(factorial, n_start - 1, n_start));
	count_n_width(n_start, n_end);
	print_edging(align, width_n, width_f, 1);
    if (n_start > n_end) {
        for (uint32_t num = n_start; num <= UINT16_MAX; num++) {
            factorial = calculate_next_factorial(factorial, num - 1, num);
            print_formatted_table(align, width_n, factorial, num, 1);
            printf("|");
            print_formatted_table(align, width_f, factorial, num, 2);
            printf("\n");
        }
        for (uint32_t num = 0; num <= n_end; num++) {
            factorial = calculate_next_factorial(factorial, num - 1, num);
            print_formatted_table(align, width_n, factorial, num, 1);
            printf("|");
            print_formatted_table(align, width_f, factorial, num, 2);
            printf("\n");
        }
        print_edging(align, width_n, width_f, 2);
    } else {
        for (uint32_t num = n_start; num <= n_end; num++) {
            factorial = calculate_next_factorial(factorial, num - 1, num);
            print_formatted_table(align, width_n, factorial, num, 1);
            printf("|");
            print_formatted_table(align, width_f, factorial, num, 2);
            printf("\n");
        }
        print_edging(align, width_n, width_f, 2);
    }
    return 0;
}
