#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#define MERSENNE_NUMBER 2147483647

uint32_t calculate_basic_factorial(uint16_t number) {
    if (number <= 1) {
        return 1;
    }
    uint32_t factorial = 1;
    for (uint64_t i = 1; i <= (uint64_t) number; i++) {
        factorial = ((uint64_t) factorial * i) % MERSENNE_NUMBER;
    }
    return factorial;
}

uint32_t calculate_next_factorial(uint32_t start_factorial, uint32_t start, uint32_t end) {
    if (end <= 1) {
        return 1;
    }
    uint32_t factorial = start_factorial;
    for (uint64_t i = start + 1; i <= (uint64_t) end; i++) {
        factorial = ((uint64_t) factorial * i) % MERSENNE_NUMBER;
    }
    return factorial;
}

uint8_t number_length(uint32_t number) {
    if (number == 0) return 1;
    return (uint8_t) floor(log10(number)) + 1;
}

uint8_t find_max_width(uint8_t type_width, uint32_t number) {
	uint8_t curr_width = number_length(number);
	return curr_width > type_width ? curr_width : type_width;
}

uint8_t count_f_width(uint16_t n_start, uint16_t n_end, uint32_t curr_factorial) {
    uint8_t curr_width = number_length(curr_factorial);
	uint8_t width_f = 2;
    width_f = curr_width > width_f ? curr_width : width_f;
    if (n_start <= n_end) {
        for (uint32_t i = n_start + 1; i <= n_end; i++) {
            curr_factorial = calculate_next_factorial(curr_factorial, i - 1, i);
            width_f = find_max_width(width_f, curr_factorial);
        }
    } else {
        for (uint32_t i = n_start + 1; i <= UINT16_MAX; i++) {
			curr_factorial = calculate_next_factorial(curr_factorial, i - 1, i);
			width_f = find_max_width(width_f, curr_factorial);
        }
        for (uint32_t i = 0; i <= n_end; i++) {
			curr_factorial = calculate_next_factorial(curr_factorial, i - 1, i);
			width_f = find_max_width(width_f, curr_factorial);
        }
    }
    width_f += 2;
	return width_f;
}

uint8_t count_n_width(uint16_t n_start, uint16_t n_end) {
	uint8_t width_n = 1;
    if (n_start <= n_end) {
        for (uint32_t i = n_start; i <= n_end; i++) {
            width_n = find_max_width(width_n, i);
        }
    } else {
        for (uint32_t i = n_start; i <= UINT16_MAX; i++) {
			width_n = find_max_width(width_n, i);
        }
        for (uint32_t i = 0; i <= n_end; i++) {
			width_n = find_max_width(width_n, i);
        }
    }
    width_n += 2;
	return width_n;
}

void print_character(uint8_t amount, char symbol_code) {
    for (uint8_t i = 1; i <= amount; i++) {
        printf("%c", symbol_code);
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
        right_offset = (width - current_num_width) >> 1;
        left_offset = width - right_offset - current_num_width;
    } else {
        left_offset = width - current_num_width - right_offset;
    }

    if (column_number == 1) {
        printf("|");
		print_character(left_offset, 32);
        printf("%" PRIu16, current_number);
		print_character(right_offset, 32);
    } else {
		print_character(left_offset, 32);
        printf("%" PRIu32, current_factorial);
		print_character(right_offset, 32);
        printf("|");
    }
}

void print_edging(int8_t align, uint8_t width_n, uint8_t width_f, uint8_t level) {
    printf("+");
	print_character(width_n, 45);
    printf("+");
	print_character(width_f, 45);
    printf("+");
    if (level == 1) {
		printf("\n|");
        if (align == -1) {
			printf(" n");
			print_character(width_n - 2, 32);
			printf("| n!");
			print_character(width_f - 3, 32);
        } else if (align == 0) {
            uint8_t r1 = (width_n - 1) >> 1;
            uint8_t r2 = (width_f - 2) >> 1;
			print_character(width_n - 1 - r1, 32);
            printf("n");
			print_character(r1, 32);
            printf("|");
			print_character(width_f - 2 - r2, 32);
            printf("n!");
			print_character(r2, 32);
        } else {
			print_character(width_n - 2, 32);
			printf("n |");
			print_character(width_f - 3, 32);
			printf("n! ");
        }
		printf("|\n+");
		print_character(width_n, 45);
        printf("+");
		print_character(width_f, 45);
        printf("+");
    }
    printf("\n");
}

void print_line(int8_t align, uint32_t factorial, uint32_t num, uint8_t width_n, uint8_t width_f) {
	print_formatted_table(align, width_n, factorial, num, 1);
	printf("|");
	print_formatted_table(align, width_f, factorial, num, 2);
	printf("\n");
}

int main() {
    int32_t n_start_provided = 0, n_end_provided = 0;
    uint16_t n_start = 0, n_end = 0;
    int8_t align = 0;
    if (scanf("%" SCNd32 " %" SCNd32 " %" SCNd8, &n_start_provided, &n_end_provided, &align) < 3 || n_start_provided < 0 || n_end_provided < 0) {
        fprintf(stderr, "Error: Invalid input format");
        return 1;
    }
    n_start = (uint16_t) n_start_provided;
    n_end = (uint16_t) n_end_provided;
	uint32_t factorial = calculate_basic_factorial(n_start - 1);
	uint8_t width_n = count_n_width(n_start, n_end);
	uint8_t width_f = count_f_width(n_start, n_end, calculate_next_factorial(factorial, n_start - 1, n_start));
	print_edging(align, width_n, width_f, 1);
    if (n_start > n_end) {
        for (uint32_t num = n_start; num <= UINT16_MAX; num++) {
            factorial = calculate_next_factorial(factorial, num - 1, num);
			print_line(align, factorial, num, width_n, width_f);
        }
        for (uint32_t num = 0; num <= n_end; num++) {
            factorial = calculate_next_factorial(factorial, num - 1, num);
			print_line(align, factorial, num, width_n, width_f);
        }
        print_edging(align, width_n, width_f, 2);
    } else {
        for (uint32_t num = n_start; num <= n_end; num++) {
            factorial = calculate_next_factorial(factorial, num - 1, num);
			print_line(align, factorial, num, width_n, width_f);
        }
        print_edging(align, width_n, width_f, 2);
    }
    return 0;
}
