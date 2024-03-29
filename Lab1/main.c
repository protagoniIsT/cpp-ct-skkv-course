 #include "return_codes.h"
 #include <stdio.h>
 #include <inttypes.h>

 typedef struct {
     uint8_t format;
     uint8_t type;
     uint8_t sign;
     int16_t exponent;
     uint64_t mantissa;
 } Number;

 Number to_IEEE754_standard(char, uint32_t);

 uint8_t sign(uint8_t, uint32_t);

 uint8_t exponent(uint8_t, uint32_t);

 uint32_t mantissa(uint8_t, uint32_t);

 uint8_t exp_size(uint8_t);

 uint8_t mant_size(uint8_t);

 uint8_t exp_mask(uint8_t);

 uint32_t mant_mask(uint8_t);

 uint8_t exp_offset(uint8_t);

 uint64_t implicit_bit(uint8_t);

 uint8_t mant_gap_to_print(uint8_t);

 uint8_t digits_to_print(uint8_t);

 Number add(uint8_t, Number, Number);

 Number subtract(uint8_t, Number, Number);

 Number multiply(uint8_t, Number, Number);

 Number divide(uint8_t, Number, Number);

 uint8_t normalize(uint8_t, Number *);

 void round_number(uint8_t rounding, Number *num);

 uint8_t most_significant_bit(uint64_t);

 void shift_to_right(uint64_t *);

 void print_result(Number);

 Number to_IEEE754_standard(char format, uint32_t number) {
     Number num;
     num.format = format == 'h';
     num.sign = sign(num.format, number);
     num.exponent = exponent(num.format, number);
     num.mantissa = mantissa(num.format, number);
     num.type = normalize(mant_size(num.format), &num);
     return num;
 }

 uint8_t sign(uint8_t format, uint32_t number) {
     return number >> (exp_size(format) + mant_size(format));
 }

 uint8_t exponent(uint8_t format, uint32_t number) {
     return (number >> mant_size(format)) & exp_mask(format);
 }

 uint32_t mantissa(uint8_t format, uint32_t number) {
     return number & mant_mask(format);
 }

 uint8_t exp_size(uint8_t format) {
     return format ? 5 : 8;
 }

 uint8_t mant_size(uint8_t format) {
     return format ? 10 : 23;
 }

 uint8_t exp_mask(uint8_t format) {
     return format ? 0x1F : 0xFF;
 }

 uint32_t mant_mask(uint8_t format) {
     return format ? 0x3FF : 0x7FFFFF;
 }

 uint8_t exp_offset(uint8_t format) {
     return format ? 15 : 127;
 }

 uint64_t implicit_bit(uint8_t format) {
     return format ? 0x400 : 0x800000;
 }

 uint8_t mant_gap_to_print(uint8_t format) {
     return format ? 2 : 1;
 }

 uint8_t digits_to_print(uint8_t format) {
     return format ? 3 : 6;
 }

 Number add(uint8_t rounding, Number num1, Number num2) {
     Number result;
     result.format = num1.format;
     result.type = 1;
     if (num1.type == 3 || num2.type == 3) {
         result.type = 3;
     } else if (num1.type == 2 || num2.type == 2) {
         if (num1.type == 2 && num2.type == 2 && num1.sign != num2.sign) {
             result.type = 3;
         } else {
             result.type = 2;
             result.sign = num1.type == 2 ? num1.sign : num2.sign;
         }
     } else if (num1.type == 0 || num2.type == 0) {
         result = num1.type == 0 ? num2 : num1;
     } else if (num1.exponent == num2.exponent && num1.mantissa == num2.mantissa && num1.sign != num2.sign) {
         result.type = 0;
     } else {
         num1.mantissa += implicit_bit(result.format);
         num2.mantissa += implicit_bit(result.format);
         if (num1.exponent < num2.exponent) {
             Number tmp = num1;
             num1 = num2;
             num2 = tmp;
         }
         result.sign = num1.sign;
         result.exponent = num1.exponent;
         while (num1.exponent > num2.exponent) {
             num1.mantissa <<= 1;
             ++num2.exponent;
         }
         result.mantissa = num1.sign ^ num2.sign ? num1.mantissa - num2.mantissa : num1.mantissa + num2.mantissa;
         normalize(most_significant_bit(num1.mantissa), &result);
     }
     round_number(rounding, &result);
     return result;
 }

 Number subtract(uint8_t rounding, Number num1, Number num2) {
     Number result;
     result.format = num1.format;
     result.type = 1;
     if (num1.type == 3 || num2.type == 3) {
         result.type = 3;
     } else if (num1.type == 2 || num2.type == 2) {
         if (num1.type == 2 && num2.type == 2 && num1.sign == num2.sign) {
             result.type = 3;
         } else {
             result.type = 2;
             result.sign = num1.type == 2 ? num1.sign : !num2.sign;
         }
     } else if (num1.type == 0 || num2.type == 0) {
         result = num1.type == 0 ? num2 : num1;
     } else if (num1.exponent == num2.exponent && num1.mantissa == num2.mantissa && num1.sign == num2.sign) {
         result.type = 0;
     } else {
         num1.mantissa += implicit_bit(result.format);
         num2.mantissa += implicit_bit(result.format);
         if (num1.exponent < num2.exponent) {
             Number tmp = num1;
             num1 = num2;
             num2 = tmp;
             num1.sign = !num1.sign;
             num2.sign = !num2.sign;
         }
         result.sign = num1.sign;
         result.exponent = num1.exponent;
         while (num1.exponent > num2.exponent) {
             num1.mantissa <<= 1;
             ++num2.exponent;
         }
         result.mantissa = num1.sign ^ num2.sign ? num1.mantissa + num2.mantissa : num1.mantissa - num2.mantissa;
         normalize(most_significant_bit(num1.mantissa), &result);
     }
     round_number(rounding, &result);
     return result;
 }

 Number multiply(uint8_t rounding, Number num1, Number num2) {
     Number result;
     result.format = num1.format;
     result.type = 1;
     result.sign = num1.sign ^ num2.sign;
     if (num1.type == 3 || num2.type == 3) {
         result.type = 3;
     } else if (num1.type == 2 || num2.type == 2) {
         if (num1.type == 0 || num2.type == 0) {
             result.type = 3;
         } else {
             result.type = 2;
         }
     } else if (num1.type == 0 || num2.type == 0) {
         result.type = 0;
     } else {
         num1.mantissa += implicit_bit(result.format);
         num2.mantissa += implicit_bit(result.format);
         shift_to_right(&num1.mantissa);
         shift_to_right(&num2.mantissa);
         result.exponent = num1.exponent + num2.exponent - exp_offset(result.format);
         result.mantissa = num1.mantissa * num2.mantissa;
         normalize(most_significant_bit(num1.mantissa) + most_significant_bit(num2.mantissa) - 1, &result);
     }
     round_number(rounding, &result);
     return result;
 }

 Number divide(uint8_t rounding, Number num1, Number num2) {
     Number result;
     result.format = num1.format;
     result.type = 1;
     result.sign = num1.sign ^ num2.sign;
     if (num1.type == 3 || num2.type == 3) {
         result.type = 3;
     } else if (num1.type == 2 || num2.type == 2) {
         if ((num1.type == 2) && (num2.type == 2)) {
             result.type = 3;
         } else {
             result.type = num1.type == 2 ? 2 : 0;
         }
     } else if (num1.type == 0 || num2.type == 0) {
         if (num1.type == 0 && num2.type == 0) {
             result.type = 3;
         } else {
             result.type = num1.type == 0 ? 0 : 2;
         }
     } else {
         num1.mantissa += implicit_bit(result.format);
         num2.mantissa += implicit_bit(result.format);
         shift_to_right(&num1.mantissa);
         shift_to_right(&num2.mantissa);
         result.exponent = num1.exponent - num2.exponent + exp_offset(result.format);
         result.mantissa = num1.mantissa / num2.mantissa;
         result.mantissa <<= mant_size(result.format) - most_significant_bit(result.mantissa) + 1;
     }
     round_number(rounding, &result);
     return result;
 }

 uint8_t normalize(uint8_t mantissa, Number *num) {
     if (num->mantissa) {
         while (most_significant_bit(num->mantissa) > mantissa) {
             ++num->exponent;
             num->mantissa >>= 1;
         }
         if (!num->exponent && num->mantissa) {
             while (!(num->mantissa & (implicit_bit(num->format))) && (num->mantissa & mant_mask(num->format))) {
                 --num->exponent;
                 num->mantissa <<= 1;
             }
             num->mantissa &= mant_mask(num->format);
             ++num->exponent;
         }
     }
     if (!(num->exponent ^ exp_mask(num->format))) {
         return num->mantissa ? 3 : 2;
     }
     return num->exponent ? 1 : 0;
 }

 void round_number(uint8_t rounding, Number *num) {
     if (num->type == 0 && rounding == 3) {
         *num = to_IEEE754_standard(num->format, 0);
         num->sign = 1;
     }
     uint64_t GSR;
     if (num->type == 1) {
         switch (rounding) {
             case 0: // К нулю
                 num->mantissa >>= most_significant_bit(num->mantissa) - mant_size(num->format) - 1;
                 break;
             case 1: // К ближайшему чётному
                 GSR = num -> mantissa & (~(mant_mask(num->format) << (most_significant_bit(num->mantissa) -
                 mant_size(num->format))));
		 if ((GSR & (1 << (most_significant_bit(num->mantissa) - mant_size(num->format) - 2))) &&
					((GSR & (1 << (most_significant_bit(num->mantissa) - mant_size(num->format) - 1))) ||
					 (GSR & (~(3 << (most_significant_bit(num->mantissa) - mant_size(num->format) - 2)))))) {
                     num->mantissa >>= most_significant_bit(num->mantissa) - mant_size(num->format) - 1;
                     ++num->mantissa;
                     normalize(mant_mask(num->format), num);
                 } else {
                     num->mantissa >>= most_significant_bit(num->mantissa) - mant_size(num->format) - 1;
                 }
                 break;
             case 2: // К +бесконечности
                 if (num->mantissa << (sizeof(num->mantissa) * 8 - most_significant_bit(num->mantissa) + mant_size(num->format) + 1)) {
                     num->mantissa >>= most_significant_bit(num->mantissa) - mant_size(num->format) - 1;
                     if (!num->sign) {
                         ++num->mantissa;
                         normalize(mant_mask(num->format), num);
                     }
                 }
                 break;
             case 3: // К -бесконечности
                 if (num->mantissa << (sizeof(num->mantissa) * 8 - most_significant_bit(num->mantissa) +
                 mant_size(num->format) + 1)) {
                     num->mantissa >>= most_significant_bit(num->mantissa) - mant_size(num->format) - 1;
                     if (num->sign) {
                         ++num->mantissa;
                         normalize(mant_mask(num->format), num);
                     }
                 }
                 break;
             default:
                 break;
         }
         num->mantissa -= implicit_bit(num->format);
     }
 }

 uint8_t most_significant_bit(uint64_t value) {
     uint8_t bit = 1;
     while (value >>= 1) ++bit;
     return bit;
 }

 void shift_to_right(uint64_t *mantissa) {
     while (!(*mantissa & 1) && *mantissa) {
         *mantissa >>= 1;
     }
 }

 void print_result(Number num) {
     if (num.type != 3 && num.sign) {
         printf("-");
     }
     switch (num.type) {
         case 0:
             printf("0x0.%0*dp+0\n", digits_to_print(num.format), 0);
             break;
         case 1:
             printf("0x1.%0*" PRIx64 "p%+hd\n", digits_to_print(num.format),
                    num.mantissa << mant_gap_to_print(num.format), num.exponent - exp_offset(num.format));
             break;
         case 2:
             printf("inf\n");
             break;
         case 3:
             printf("nan\n");
             break;
         default:
             break;
     }
 }

 int main(int argc, char *argv[]) {
     char format = '\0', operation = '\0';
     uint8_t rounding = 0;
     uint32_t number1 = 0, number2 = 0;

     switch (argc) {
         case 6:
             sscanf(argv[4], "%c", &operation);
             sscanf(argv[5], "%i", &number2);
             if (operation != '+' && operation != '-' && operation != '*' && operation != '/') {
                 fprintf(stderr, "Error: Unsupported operation: %c\n", operation);
                 return ERROR_ARGUMENTS_INVALID;
             }
         case 4:
             sscanf(argv[1], "%c", &format);
             sscanf(argv[2], "%hhu", &rounding);
             sscanf(argv[3], "%i", &number1);

             if (format != 'h' && format != 'f') {
                 fprintf(stderr, "Error: Unsupported data format: %c\n", format);
                 return ERROR_ARGUMENTS_INVALID;
             }
             if (rounding > 3) {
                 fprintf(stderr, "Error: Unsupported rounding type: %hhd\n", rounding);
                 return ERROR_ARGUMENTS_INVALID;
             }
             break;
         default:
             fprintf(stderr, "Error: Invalid input format\n");
             return ERROR_ARGUMENTS_INVALID;
     }

     Number num = to_IEEE754_standard(format, number1);
     if (argc == 6) {
         Number num2 = to_IEEE754_standard(format, number2);
         switch (operation) {
             case '+':
                 num = add(rounding, num, num2);
                 break;
             case '-':
                 num = subtract(rounding, num, num2);
                 break;
             case '*':
                 num = multiply(rounding, num, num2);
                 break;
             case '/':
                 num = divide(rounding, num, num2);
                 break;
             default:
                 fprintf(stderr, "Error: Unsupported operation: %c\n", operation);
                 return ERROR_UNSUPPORTED;
         }
     }
     print_result(num);
     return SUCCESS;
 }

