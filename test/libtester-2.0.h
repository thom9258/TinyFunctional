/* 
# INTRODUCTION
	libtester - Library testing utility for c-c++ libraries,
	written in ansi-c99. Designed for test-driven development of
	of libraries & programs.

## ORIGINAL AUTHOR

Thomas Alexgaard Jensen (![github/Alexgaard](https://github.com/thom9258))

## LICENSE

Copyright (c) Thomas Alexgaard Jensen
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from
the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software in
   a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.

For more information, please refer to
![Zlib License](https://choosealicense.com/licenses/zlib/)




# CHANGELOG for version 2

- [2.1] Fixed c++ compilation warnings.
- [2.0] Imported and renamed entire library, restructured context
        management and usage.

*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h> /*stdout*/
#include <stdarg.h> /*va_list*/
#include <stdint.h> /*uint32_t, uint8_t*/
#include <time.h> /*clock_t, clock(), CLOCKS_PER_SEC*/
#include <stdbool.h> /*true, false*/

#ifndef _LIBTESTER_H_
#define _LIBTESTER_H_
    
#define LIBTESTER_VERSION "2.1"
    
#define TEST(_EXPR)                                                         \
	_lt_test_call(!(_EXPR), #_EXPR, __LINE__, NULL)

#define TESTM(_EXPR, _MSG)                                                  \
	_lt_test_call(!(_EXPR), #_EXPR, __LINE__, _MSG)

#define TEST_UNIT(FUNCTION)                                                     \
	_lt_pre_unit_call(__FILE__, #FUNCTION);                        \
	FUNCTION;                                                                  \
	_lt_post_unit_call(#FUNCTION);                                              \

#define LT_IS_MEMALIGNED(POINTER, BYTE_COUNT) \
	( (((uintptr_t)(const void*)(POINTER)) & (BYTE_COUNT-1)) == 0 )
    
#ifdef LIBTESTER_NO_COLOR
#define LT_RED
#define LT_GREEN
#define LT_BLUE
#define LT_RESET
#define LT_DEFAULT
#else /*NOT LIBTESTER_NO_COLOR*/
#define LT_RED     "\033[0;31m"
#define LT_GREEN   "\033[0;32m"
#define LT_BLUE    "\033[0;34m"
#define LT_RESET   "\033[0m"
#define LT_DEFAULT LT_BLUE
#endif /*LIBTESTER_NO_COLOR*/

void _lt_printf(const char* color, const char* fmt, ...);
#define lt_print(msg, ...)    _lt_printf(NULL,       (const char*)msg, ##__VA_ARGS__)
#define lt_bgprint(msg, ...)  _lt_printf((const char*)LT_DEFAULT, (const char*)msg, ##__VA_ARGS__)
#define lt_errprint(msg, ...) _lt_printf((const char*)LT_RED,     (const char*)msg, ##__VA_ARGS__)
#define lt_okprint(msg, ...)  _lt_printf((const char*)LT_GREEN,   (const char*)msg, ##__VA_ARGS__)
    
#define LT_LINE                                                                \
"----------------------------------------------------------------------------\n"

#define LT_BOLDLINE                                                            \
"============================================================================\n"

#ifndef LT_MAX_TESTS
#  define LT_MAX_TESTS 128
#endif
    
typedef struct {
	time_t start;
	double elapsed_sec;
}lt_timer;

struct {
    char* test_names[LT_MAX_TESTS];
    char test_statuses[LT_MAX_TESTS];
    int evaluated_tests_count;
    int curr_test_errors;
    int total_test_errors;
    uint32_t randseed;
    FILE* filetarget;
    lt_timer timer;
}lt_context;
 
char
lt_str_eq(const char* _a, const char* _b)
{
    char* a = (char*)_a;
    char* b = (char*)_b;
    if (a == NULL || b == NULL)
        return false;

    while (*a != '\0' && *b != '\0') {
        if (*a != *b)
            return false;
        a++;
        b++;
    }
    return true;
}

#define LT_SHIFTARGS(argc, argv) _argc--; _argv++
    
 /**
 * @brief Begin unit test context
 *
 * Parses optional arguments:
 *  -h             print help message
 *  -t <target>    set logging file target
 * 
 */
void
ltcontext_begin(int _argc, char** _argv) {
    lt_context.evaluated_tests_count = 0;
    lt_context.curr_test_errors = 0;
    lt_context.total_test_errors = 0;
    lt_context.randseed = 2501;
    lt_context.filetarget = NULL;
    lt_context.timer = (lt_timer) {0, 0};
    
    while (_argc > 1 || *_argv != NULL) {
        if (lt_str_eq("-f", *_argv) || lt_str_eq("--file", *_argv)) {
            LT_SHIFTARGS(_argc, _argv);
            lt_bgprint("File target: %s\n", *_argv);
            lt_context.filetarget = fopen(*_argv, "w");
        }

        LT_SHIFTARGS(_argc, _argv);
    }
}

 /**
 * @brief End unit test context
 *
 * Ends the unit test context and closes optional filetarget.
 */
int
ltcontext_end(void)
/**
 * ltcontext_end() - Print summary of all tests.
 */
{
	int i;
    int n_failed = 0;
	lt_bgprint(LT_BOLDLINE "Tests Summary:\n" LT_LINE);

	for (i = 0; i < lt_context.evaluated_tests_count; i++) {
		if (lt_context.test_statuses[i] == 0) {
			lt_okprint( "\t[%d] OK    %s\n", i, lt_context.test_names[i]);
		} else {
			lt_errprint("\t[%d] ERROR %s\n", i, lt_context.test_names[i]);
            n_failed++;
		}
	}
    lt_bgprint(LT_BOLDLINE);

    if (n_failed > 0) {
        lt_errprint("FAILED %d UNIT-TESTS OUT OF %d\n",
                    n_failed,
                    lt_context.evaluated_tests_count);
    }
    else {
        lt_okprint("PASSED ALL %d UNIT-TESTS.\n", lt_context.evaluated_tests_count);
    }

    if (lt_context.filetarget != NULL)
        fclose(lt_context.filetarget);

    return (n_failed > 0);
}

/**
 * @brief Default printing function
 *
 * @arg Formatting string
 * @arg [variadic] arguments
 *
 * prints to stdout, and optionally to filetarget if specified in "lt_context"
 */
void _lt_printf(const char* color, const char* fmt, ...) {
    va_list termlist;
    va_list filelist;

    va_start(termlist, fmt);
    if (lt_context.filetarget != NULL) {
        va_copy(filelist, termlist);
        vfprintf(lt_context.filetarget, fmt, filelist);
        va_end(filelist);
    }

    if (color != NULL)
        fprintf(stdout, "%s", color);
    vfprintf(stdout, fmt, termlist);
    if (color != NULL)
        fprintf(stdout, LT_RESET);
    va_end(termlist);
}

void
lt_timer_start(lt_timer* _t)
{
	if (_t == NULL)
		return;
	_t->start = clock();
}

void
lt_timer_stop(lt_timer* _t)
{
	if (_t == NULL)
		return;
	_t->elapsed_sec += (double)(clock() - _t->start) / CLOCKS_PER_SEC;
}

void
lt_timer_reset(lt_timer* _t)
{
	if (_t == NULL)
		return;
	_t->elapsed_sec = 0;
	_t->start = 0;
}

/**
 * @brief Test expression for pass/fail 
 * @arg1 Expression result
 * @arg2 Expression string
 * @arg3 Line number
 * @arg4 External message for test
 *
 * Tests function expression.
 */
char
_lt_test_call(char _result, const char* _expr_str, int _line, const char* _msg)
{
	char* msg;
    if (_msg == NULL) msg = (char*)"";
    else              msg = (char*)_msg;
	lt_context.total_test_errors++;                                                   

	if (_result != 0) {                                                   
		lt_context.curr_test_errors++;
        lt_errprint("[test L%d] FAILED ( %s )  %s\n", _line, _expr_str, msg);
	}                                                              
	else {                                                         
        lt_okprint( "[test L%d] PASSED ( %s )  %s\n", _line, _expr_str, msg);
	}                                                                 
    return _result;
}

/**
 * @brief Print status before unit-test.
 * @arg File name
 * @arg Test function name
 *
 * Prints:
 * - Visually eye-catching start of test section
 * - test file name
 * - function to test & its line number
 */
void
_lt_pre_unit_call(const char* _file_name, const char* _test_name)
{
	lt_context.curr_test_errors = 0;                     
	lt_context.total_test_errors = 0;                           

	lt_bgprint(LT_BOLDLINE);
	lt_bgprint("[ TEST %d ] File:  ", lt_context.evaluated_tests_count);
	lt_print("%s\n", _file_name);
	lt_bgprint("Name:  ");
	lt_print("%s\n", _test_name);
	lt_bgprint(LT_BOLDLINE);                                      

	lt_timer_reset(&lt_context.timer);
	lt_timer_start(&lt_context.timer);
}

/**
 * @brief Print status after unit-test.
 *
 * @arg Test function name
 *
 * Prints:
 * - Visually eye-catching passed/fail
 * - execution time
 * - outcome summation of tests for given function
 */
void
_lt_post_unit_call(const char* _test_name)
{
	lt_timer_stop(&lt_context.timer);
	lt_bgprint(LT_LINE);                                      
	lt_context.test_names[lt_context.evaluated_tests_count] = (char*)_test_name;
    
	if (lt_context.curr_test_errors == 0) {
		lt_context.test_statuses[lt_context.evaluated_tests_count] = 0;
        lt_okprint(">>>[[ TEST %d PASSED ]]  ->  (errors=%d/%d, time=%fs)\n",
                   lt_context.evaluated_tests_count,
                   lt_context.curr_test_errors,
                   lt_context.total_test_errors,
                   lt_context.timer.elapsed_sec);
        lt_bgprint(LT_BOLDLINE);
	}
	else {
		lt_context.test_statuses[lt_context.evaluated_tests_count] = 1;
        lt_errprint(">>>[[ TEST %d FAILED ]]  ->  (errors=%d/%d, time=%fs)\n",
                    lt_context.evaluated_tests_count,
                    lt_context.curr_test_errors,
                    lt_context.total_test_errors,
                    lt_context.timer.elapsed_sec);
        lt_bgprint(LT_BOLDLINE);
	}
	lt_context.evaluated_tests_count++;
    lt_print("\n");
}

 /**
 * @brief generate seeded random uint32.
 * @arg the seed used to generate random value.
 *
 * Generate psuedo-random unsigned integer of 32 bit.
 * seed value is incremented after use.
 *
 * @return generated number.
 */
uint32_t
lt_rand_uint(void)
{
	const uint32_t mangle_1     = 0x3D73;
	const uint32_t mangle_2     = 0xC0AE5;
	const uint32_t mangle_3     = 0x5208DD0D;
	const uint32_t mangle_shift = 13;
	uint32_t n;
	n = (lt_context.randseed << mangle_shift) ^ lt_context.randseed;
	n *= n * mangle_1;
	n += mangle_2;
	n *= n;
	n += mangle_3;
	lt_context.randseed++;
	return n;
}

/**
 * @brief generate seeded random bool (0/1).
 * @arg1 the seed used to generate random value.
 *
 * Generate psuedo-random boolean (0/1).
 * Effectively returns even/odd of lt_rand_uint().
 *
 * @return generated number.
 */
uint8_t
lt_rand_bool(void)
{
	return (lt_rand_uint() & 1);
}

/**
 * @brief generate seeded random between specified min/max.
 * @arg1 the seed used to generate random value.
 * @arg2 minimum generated value.
 * @arg3 maximum generated value.
 *
 * Generate psuedo-random unsigned integer 32 bit, generated number is modified
 * to fit between specified max/min).
 *
 * @return generated number.
 */
uint32_t
lt_rand_ubetween(uint32_t _min, uint32_t _max)
{
	return ((lt_rand_uint() % (_max - _min + 1)) + _min);
}

#endif /*_LIBTESTER_H_*/

#ifdef __cplusplus
}
#endif
