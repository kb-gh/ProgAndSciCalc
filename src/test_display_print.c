#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "display_print.h"
#include "calc_types.h"

/* For testing functions display_print_emode and display_print_gmode
 * from display_print.c */


decContext dfp_context;

typedef struct
{
    char *val;
    int digits;
    char *expected;
} test_t;


static const test_t emode_tests[] =
{
    {"0", 8, "0"},
    {"9", 8, "9"},
    {"9.0000000000", 8, "9"},
    {"9.0000100000", 8, "9.00001"},
    {"-9.1000100000", 8, "-9.10001"},
    {"12345678", 8, "1.2345678e+7"},
    {"123456789", 8, "1.2345679e+8"},
    {"-12345678", 8, "-1.2345678e+7"},
    {"-123456789", 8, "-1.2345679e+8"},
    {"1234.56789", 6, "1.23457e+3"},
    {"12345678901234567890", 20, "1.234567890123456789e+19"},
    {"12345678901234567890", 10, "1.23456789e+19"},
    {"-1000000", 6, "-1e+6"},
    {"1000000000", 6, "1e+9"},
    {"7e7", 6, "7e+7"},
    {"-7.00e7", 6, "-7e+7"},
    {"1.23456789e21", 10, "1.23456789e+21"},
    {"-1.0001000e21", 10, "-1.0001e+21"},
    {"0.0001001", 8, "1.001e-4"},
    {"-0.0012345678", 6, "-1.23457e-3"},
    {"9999999", 6, "1e+7"},
    {"2.2222222222222e100", 6, "2.22222e+100"},
};


static const test_t gmode_tests[] =
{
    {"0", 8, "0"},
    {"9", 8, "9"},
    {"9.0000000000", 8, "9"},
    {"9.0000100000", 8, "9.00001"},
    {"-9.1000100000", 8, "-9.10001"},
    {"12345678", 8, "12345678"},
    {"123456789", 8, "1.2345679e+8"},
    {"-12345678", 8, "-12345678"},
    {"-123456789", 8, "-1.2345679e+8"},
    {"1234.56789", 6, "1234.57"},
    {"12345678901234567890", 20, "12345678901234567890"},
    {"12345678901234567890", 10, "1.23456789e+19"},
    {"-1000000", 6, "-1e+6"},
    {"1000000", 8, "1000000"},
    {"1000000000", 6, "1e+9"},
    {"7e7", 6, "7e+7"},
    {"7e7", 8, "70000000"},
    {"-7.00e7", 6, "-7e+7"},
    {"1.23456789e21", 10, "1.23456789e+21"},
    {"-1.0001000e21", 10, "-1.0001e+21"},
    {"0.0001001", 8, "0.0001001"},
    {"-0.0012345678", 6, "-0.00123457"},
    {"9999999", 6, "1e+7"},
    {"2.2222222222222e100", 6, "2.22222e+100"},
    {"1e9", 10, "1000000000"},
    {"-1e9", 10, "-1000000000"},
};

static bool test_emode(void)
{
    stackf_t fval;
    char buf[DFP_STRING_MAX];
    bool ok = true;

    for (unsigned int i = 0; i < sizeof(emode_tests) / sizeof(emode_tests[0]); i++)
    {
        dfp_from_string(&fval, emode_tests[i].val, &dfp_context);
#if 0
        dfp_to_string(&fval, buf);
        printf("orig %s\n", buf);
#endif
        display_print_emode(buf, fval, emode_tests[i].digits);
        if (strcmp(buf, emode_tests[i].expected) != 0)
        {
            printf("emode FAIL: got %s  expected %s\n", buf, emode_tests[i].expected);
            ok = false;
        }
    }
    return ok;
}

static bool test_gmode(void)
{
    stackf_t fval;
    char buf[DFP_STRING_MAX];
    bool ok = true;

    for (unsigned int i = 0; i < sizeof(gmode_tests) / sizeof(gmode_tests[0]); i++)
    {
        dfp_from_string(&fval, gmode_tests[i].val, &dfp_context);
#if 0
        dfp_to_string(&fval, buf);
        printf("orig %s\n", buf);
#endif
        display_print_gmode(buf, fval, gmode_tests[i].digits);
        if (strcmp(buf, gmode_tests[i].expected) != 0)
        {
            printf("gmode FAIL: got %s  expected %s\n", buf, gmode_tests[i].expected);
            ok = false;
        }
    }
    return ok;
}

int main(void)
{
    decContextDefault(&dfp_context, DEC_INIT_DECQUAD);

    bool ok = test_emode() && test_gmode();

    if (ok)
        printf("all tests OK\n");
    else
        printf("************* FAIL ***************\n");

    return 0;
}
