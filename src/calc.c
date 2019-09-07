/*****************************************************************************
 * File calc.c part of ProgAndSciCalc
 *
 * Copyright (C) 2018 Ken Bromham
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "calc_internal.h"

decContext dfp_context;

static int debug_level;

static void (*result_callback)(uint64_t, stackf_t);
static void (*history_callback)(uint64_t, stackf_t);
static void (*num_paren_callback)(int);
static void (*warn_callback)(const char *msg);
static void (*error_callback)(const char *msg);
static bool (*get_best_integer_callback)(uint64_t *, calc_width_enum, bool);

static calc_mode_enum calc_mode;
static calc_angle_enum calc_angle;
/* use unsigned in integer mode */
static bool use_unsigned;
static calc_width_enum integer_width = calc_width_64;

static bool warn_on_signed_overflow = true;
static bool warn_on_unsigned_overflow = true;

/* Priority for binary ops. Unary ops are grabbed immediately so effectively
 * have a priority above PRIORITY_MAX. For equals, use PRIORITY_MIN.
 * Bitwise and,or,xor use PRIORITY_ADD_SUB.
 * Shifts and gcd use PRIORITY_MUL_DIV. */
#define PRIORITY_ADD_SUB      0
#define PRIORITY_MUL_DIV      1
#define PRIORITY_POWER_ROOT   2
#define PRIORITY_MIN  PRIORITY_ADD_SUB
#define PRIORITY_MAX  PRIORITY_POWER_ROOT
#define NUM_PRIORITY (PRIORITY_MAX + 1)

/* pick a number for max nesting, 4 is enough for me */
#define MAX_PARENTHESES 4
/* So priority 0,1,2 raised to 10,11,12 in first level, 20,21,22 in
 * second level etc.  Value needs to be >= NUM_PRIORITY */
#define PARENTHESES_PRIORITY_RAISE 10
static int num_parentheses;
static bool paren_allowed;
static void report_num_used_parentheses(void);

/* Stack element */
typedef struct
{
    uint64_t ival;
    stackf_t fval;
} stack_el_t;

#define STACK_SIZE ((NUM_PRIORITY * (MAX_PARENTHESES + 1)) + 1)
static stack_el_t stack[STACK_SIZE];
static int stack_index;

#define NUM_MEMORY 2
static stack_el_t mem_val[NUM_MEMORY];

/* to provide the current value to the new mode when switching mode */
static stack_el_t save_val;
static bool init_from_save_val;

/* Stack element for binary operations */
typedef struct
{
    calc_op_enum cop;
    uint64_t (*iop)(uint64_t, uint64_t);
    stackf_t (*fop)(stackf_t, stackf_t);
    int priority;
} bop_stack_el_t;

#define BOP_STACK_SIZE (NUM_PRIORITY * (MAX_PARENTHESES + 1))
static bop_stack_el_t bop_stack[BOP_STACK_SIZE];
static int bop_stack_index;
static bool bin_op_was_entered;

/* Whether repeatedly entering equals will repeat the last binary
 * operation eg 2 + 3 = 5 = 8 = 11 (repeats the + 3 in this example). */
static bool allow_repeated_equals;

/* random_range == 0 is taken to mean generate number >= 0 and < 1,
 * random_range > 0 is taken to mean generate an integer number in
 * range 1 to random_range. */
static int random_range;
/* optionally some extra rounding on sin cos tan */
static bool use_sct_rounding;

static void calc_info(const char *msg)
{
    if (debug_level == 0)
        return;

    printf("calc info:  %s\n", msg);
}

static void print_stack(void)
{
    if (debug_level == 0)
        return;

    for (int i = 0; i < STACK_SIZE; i++)
    {
        char buf[DFP_STRING_MAX];
        dfp_to_string(&stack[i].fval, buf);
        if (use_unsigned)
        {
            printf("  [%" PRIu64 " %s] ", stack[i].ival, buf);
        }
        else
        {
            int64_t si = calc_util_get_signed(stack[i].ival, integer_width);
            printf("  [%" PRId64 " %s] ", si, buf);
        }
    }
    printf("\n");
    printf("  stack index %d\n", stack_index);
}

static void print_bop_stack(void)
{
    if (debug_level == 0)
        return;

    printf("  bop stack index %d\n", bop_stack_index);
}

void calc_error(const char *msg)
{
    if (error_callback)
    {
        error_callback(msg);
    }
    else
    {
        fprintf(stderr, "*************** CALC ERROR : %s\n", msg);
        exit(1);
    }
}

void calc_warn(const char *msg)
{
    if (warn_callback)
    {
        warn_callback(msg);
    }
    else
    {
        printf("**************** CALC WARN *** : %s\n", msg);
    }
}

static void history_update(const stack_el_t *s)
{
    calc_info("history_update");

    if (history_callback)
    {
        history_callback(s->ival, s->fval);
    }
}

#define stack_num_args() stack_index

static void stack_push(uint64_t iarg, stackf_t farg)
{
    if (stack_index < STACK_SIZE)
    {
        calc_info("stack push");
        stack[stack_index].ival = iarg;
        stack[stack_index].fval = farg;
        history_update(&stack[stack_index]);
        stack_index++;
        print_stack();
    }
    else
    {
        calc_error("stack push full");
    }
}

static stack_el_t stack_pop(void)
{
    if (stack_index > 0)
    {
        calc_info("stack pop");
        stack_index--;
        print_stack();
        return stack[stack_index];
    }
    else
    {
        calc_error("stack pop empty");
        return stack[0];
    }
}

static const stack_el_t *stack_peek(void)
{
    if (stack_index > 0)
    {
        calc_info("stack peek");
        return &stack[stack_index - 1];
    }
    else
    {
        calc_error("stack peek empty");
        return stack;
    }
}


#define bop_stack_num_args() bop_stack_index

static void bop_stack_push(calc_op_enum cop,
                           uint64_t (*fni)(uint64_t, uint64_t),
                           stackf_t (*fnf)(stackf_t, stackf_t),
                           int pri)
{
    if (bop_stack_index < BOP_STACK_SIZE)
    {
        calc_info("bop stack push");
        bop_stack[bop_stack_index].cop = cop;
        bop_stack[bop_stack_index].iop = fni;
        bop_stack[bop_stack_index].fop = fnf;
        bop_stack[bop_stack_index].priority = pri;
        bop_stack_index++;
        bin_op_was_entered = true;
        print_bop_stack();
    }
    else
    {
        calc_error("bop stack push full");
    }
}

/* Note, returning a pointer */
static const bop_stack_el_t *bop_stack_pop(void)
{
    if (bop_stack_index > 0)
    {
        calc_info("bop stack pop");
        bop_stack_index--;
        print_bop_stack();
        return &bop_stack[bop_stack_index];
    }
    else
    {
        calc_error("bop stack pop empty");
        return bop_stack;
    }
}

static const bop_stack_el_t *bop_stack_peek(void)
{
    if (bop_stack_index > 0)
    {
        calc_info("bop stack peek");
        return &bop_stack[bop_stack_index - 1];
    }
    else
    {
        calc_error("bop stack peek empty");
        return bop_stack;
    }
}

/* With decimal floating point, value zero can have a varying number
 * of digits (as long as all zero I presume) and a varying exponent.
 * This function turns any zero into a 'simple' zero, used before
 * pushing any new values onto the stack. This might make it a bit
 * simpler when displaying. */
static void dfp_normalise_zero(stackf_t *arg)
{
    if (dfp_is_zero(arg))
    {
        dfp_zero(arg);
    }
}

static void request_display_update(const stack_el_t *s)
{
    calc_info("request_update");

    if (result_callback)
        result_callback(s->ival, s->fval);
    else
        calc_error("result callback NULL");
}


static void unary_op(uint64_t (*fni)(uint64_t),
                     stackf_t (*fnf)(stackf_t))
{
    uint64_t iresult;
    stackf_t fresult;
    stack_el_t arg;

    if (calc_mode == calc_mode_integer && fni == NULL)
        return;
    if (calc_mode == calc_mode_float && fnf == NULL)
        return;

    arg = stack_pop();

    if (stack_num_args() < bop_stack_num_args())
    {
        /* Covers scenario where unary op is entered after binop without
         * another val being entered in between.
         * Put the arg back, so the result will be added to stack rather than
         * replacing the arg on the stack.
         * Covers scenario like
         *  2 +  +/- [-2]  = [0] = [-2] = [-4]
         *            ^
         *            |
         * take this as val for 2nd arg, as if you had entered
         *  2 + 2 +/- [-2] = [0] = [-2] = [-4]
         *
         * likewise
         *  2 + 3 + [5]  +/- [-5] = [0] = [-5] = [-10]
         * likewise
         *  10 + 2 * 3 * [6] +/- [-6] = [-26] ie. interpret as
         *  10 + 2 * 3 * -6 = -26
         * likewise all unary ops eg.
         *  10 + 2 * sqr [4] sqr [16] = [42]
         */
        stack_push(arg.ival, arg.fval);
    }
    if (calc_mode == calc_mode_integer)
    {
        iresult = fni(arg.ival);
        dfp_zero(&fresult);
    }
    else
    {
        iresult = 0;
        fresult = fnf(arg.fval);
        dfp_normalise_zero(&fresult);
    }
    stack_push(iresult, fresult);
    request_display_update(stack_peek());
}


/* Collapse any outstanding bin operations as far as priority allows */
static void process_bin_ops(int priority)
{
    while (bop_stack_num_args() > 0)
    {
        uint64_t iresult;
        stackf_t fresult;
        stack_el_t arg1;
        stack_el_t arg2;
        const bop_stack_el_t *bop_info;

        calc_info("bin ops loop");

        bop_info = bop_stack_peek();
        if (priority > bop_info->priority)
            break;

        bop_info = bop_stack_pop();

        /* There will be enough args on the stack, unless in a scenario like
         * 2 + =
         * ie. no 2nd arg entered for the +, so need to choose what to do.
         * Either drop the dangling +, so 2 + = 2 becomes simply 2 = [2],
         * or duplicate the 1st arg for the 2nd and interpret it as
         * 2 + 2 = [4]
         *
         * And need to generalise this for multiple priority eg.
         * 10 + 2 * =
         * can either be drop the dangling *, so 10 + 2 * = becomes simply
         * 10 + 2 = [12]
         * or duplicate the 2 so it becomes 10 + 2 * 2 = [14]
         *
         * Which method to choose depends on allow_repeated_equals.
         * If it's false, drop the dangling operator, if it's true,
         * duplicate. If it's false, however, then still push the duplicate,
         * then pop it off again, this way the duplicate will be there in case
         * the user changes allow_repeated_equals to true.
         *
         * Personally, by default, I prefer it to drop the dangling operator,
         * so you can do 1 + 2 + 3 + = 6 ie. if you inadvertantly hit the =
         * at the end it doesn't change the result and give you 12.
         *
         * For the test, remember we have already popped the bin_op.
         */
        if (stack_num_args() < bop_stack_num_args() + 2)
        {
            const stack_el_t *s;
            calc_info("duplicate arg1");
            s = stack_peek();
            stack_push(s->ival, s->fval);
            if (!allow_repeated_equals)
            {
                calc_info("discard dangling binop");
                (void)stack_pop();
                continue;
            }
        }

        arg2 = stack_pop();
        arg1 = stack_pop();
        if (calc_mode == calc_mode_integer)
        {
            iresult = bop_info->iop(arg1.ival, arg2.ival);
            dfp_zero(&fresult);
        }
        else
        {
            iresult = 0;
            fresult = bop_info->fop(arg1.fval, arg2.fval);
            dfp_normalise_zero(&fresult);
        }

        /* Put result back as arg for next up in the chain (if any).
         * This also means the final result is left on the stack */
        stack_push(iresult, fresult);
    }
    calc_info("bin ops end");
}

static void bin_op_common(calc_op_enum cop,
                          uint64_t (*fni)(uint64_t, uint64_t),
                          stackf_t (*fnf)(stackf_t, stackf_t),
                          int priority)
{
    if (calc_mode == calc_mode_integer && fni == NULL)
        return;
    if (calc_mode == calc_mode_float && fnf == NULL)
        return;

    /* Check for repeated bin_ops without an arg in between eg.
     *   10 + + - 3 = [7] = [4] = [1]
     *
     *   10 + 2 * * * + [12] 3 = [15] = [18] = [21]
     *
     * In a case of consecutive bin_ops with a ( in between eg.
     * 10 + ( +
     * then because a 0 is pushed when the ( is added you end up with
     * 10 + ( [0] +
     * so this doesn't trigger the repeated bin_op without an
     * arg in between test. I think. This is getting quite confusing.
     * Remember we haven't pushed the current bin_op yet.
     */
    if (bop_stack_num_args() > 0 &&
        bop_stack_num_args() >= stack_num_args())
    {
        /* Remove the existing bin_op, then run through as normal for
         * processing the new bin_op. */
        (void)bop_stack_pop();
    }

    /* Within parentheses, the priority of any operator is higher than any
     * preceeding operator outside the parentheses eg.
     * 0,1,2 raised to 10,11,12 and 10 is clearly > 2 */
    priority += num_parentheses * PARENTHESES_PRIORITY_RAISE;

    /* Collapse any outstanding bin operations as far as priority allows */
    process_bin_ops(priority);
    /* Then store the new bin op */
    bop_stack_push(cop, fni, fnf, priority);
    request_display_update(stack_peek());
    paren_allowed = true;

    /* priority tests
     *
     * 10 + 2 * 5 * [10] 6 = [70] = [130] = [190]
     *
     * 10 * 2 * [20] * * + 5 = [25] = [30] = [35]
     *
     * 10 + 2 * 5 * [10] 6 / [60] 12 = [15] = [20] = [25]
     *
     * 10 + 2 + [12] 3 * 4 * [12] 5 = [72] = [132] = [192]
     *
     */

    /* parentheses tests
     *
     * 10 * ( [0] 2 + 3 [5] + 4) [9] = [90]
     *
     * 10 * ( [0] (2 - 3) [-1] * 20) [-20] = [-200]
     *
     * 1 + ( [0] 2 + ( [0] ) [0] ) [2]  = [3]
     *
     * 1 + ( [0] ((( )))) = [1]
     *
     * ((((10 - 2) [8] + 3 * 2) [14] + 7 + [21] 3) [24] + 20 / 2) [34]
     *
     * 1 + 2 * 2 pow (1 + 2 * 1 pow (1 + 2 * 1 pow (1 + 2 * 1 pow (1 + 2 * 1 pow 1) [3] ) [3] ) [3] ) [3] = [17]
     *
     * what about 10 * ( + ), this is 10 * (0 +), then depends on repeated_eq
     *  either 10 * (0) or 10 * (0 + 0)
     * likewise 10 * (/)
     *  either 10 * (0) or 10 * (0 / 0) ==> nan
     */
}


static void op_equals(void)
{
    if (bop_stack_num_args() > 0)
    {
        process_bin_ops(PRIORITY_MIN);
        request_display_update(stack_peek());
    }
    else if (bin_op_was_entered && allow_repeated_equals)
    {
        /* Last bin_op still sitting in bop_stack[0], and can simplify stack
         * handling since the 2 arguments must be in stack[0] and stack[1],
         * and the result will always go in stack[0]. */

        /* This should cover scenarios like :-
         *   repeated = without entering another value eg.
         *   1 + 2 = [3] = [5] = [7] ie. repats the +2
         *
         *   repeted = after a unary op without entering another value eg.
         *   1 + 2 = [3] +/- [-3] = [-1] = [1] = [3] ie. repeats the +2
         *
         *   repeated = after entering one value eg.
         *   1 + 2 = [3]
         *       5 = [7]  repeat +2
         *      10 = [12] repeat +2 etc.
         *
         *   repeated = after a unary op then enter one value
         *   1 + 2 = [3] +/- [-3]
         *               5 = [7]  repeats the +2
         *              10 = [12] repeats the +2
         */
        uint64_t iresult;
        stackf_t fresult;
        stack_el_t *arg1 = &stack[0];
        stack_el_t *arg2 = &stack[1];
        if (calc_mode == calc_mode_integer)
        {
            iresult = bop_stack[0].iop(arg1->ival, arg2->ival);
            dfp_zero(&fresult);
        }
        else
        {
            iresult = 0;
            fresult = bop_stack[0].fop(arg1->fval, arg2->fval);
            dfp_normalise_zero(&fresult);
        }
        stack[0].ival = iresult;
        stack[0].fval = fresult;
        history_update(stack_peek());
        request_display_update(&stack[0]);
    }
    else
    {
        /* scenario like
         *   10 =
         *    5 =
         */

        /* Nothing needed, each value entered just replaces the last one.
         * But still update display, so we always return the current top
         * of stack.
         * Test :-
         *  i) float mode, enter eg. 1.23e+04, then enter =, display should
         *      update with 12300
         */
        request_display_update(stack_peek());
    }
    paren_allowed = true;
    if (num_parentheses != 0)
    {
        num_parentheses = 0;
        report_num_used_parentheses();
    }
    calc_info("op eq end");
    print_stack();
    print_bop_stack();
}

static void new_arg(uint64_t iarg, stackf_t farg)
{
    /* Things should (mostly) be masked off already, but exceptions are
     * memory recall, value from history, or value pasted from clipboard */
    uint64_t iarg_masked = iarg;
    calc_util_mask_width(&iarg_masked, integer_width);

    /* Decide if the new arg should replace the current top of stack ie. pop
     * the stack first before pushing the new arg. For example,
     * starting over after an = without a bin op being entered first,
     * or replace the result of a unary op in a case like
     *     10 + 3 * 2 sqr [4] 8 = [34]
     *  ie. the 8 replaces the [4] so 10 + 3 * 8
     */
    if (stack_num_args() > bop_stack_num_args())
        (void)stack_pop();

    dfp_normalise_zero(&farg);
    stack_push(iarg_masked, farg);
    paren_allowed = false;
}


void calc_give_arg(uint64_t ival, stackf_t fval)
{
    new_arg(ival, fval);
}

static void report_num_used_parentheses(void)
{
    if (num_paren_callback)
    {
        num_paren_callback(num_parentheses);
    }
}

static void parentheses_left(void)
{
    if (num_parentheses < MAX_PARENTHESES && paren_allowed)
    {
        /* The normal case is enter a ( after a bin_op. Add a 0 arg
         * which will normally be overwritten by the next arg entered,
         * (new_arg handles this) but if no arg is entered before another
         * bin_op then it uses the 0 value. Effectively it is like starting
         * a separate stack.
         * Also allow ( after = or clr. In this case we want the new
         * 0 arg to replace current value on stack, new_arg will do that.
         * If there are repeated (( without anything in between, we should
         * only end up with one zero added, new_arg handles this.
         */
        stackf_t dzero;
        dfp_zero(&dzero);
        new_arg(0, dzero);
        /* need to reset paren_allowed */
        paren_allowed = true;
        request_display_update(stack_peek());
        num_parentheses++;
        report_num_used_parentheses();
    }
}

static void parentheses_right(void)
{
    if (num_parentheses > 0)
    {
        /* like equals but with priority as the min priority for the level */
        process_bin_ops(num_parentheses * PARENTHESES_PRIORITY_RAISE);
        request_display_update(stack_peek());
        num_parentheses--;
        report_num_used_parentheses();
        print_stack();
        print_bop_stack();
    }
}


static void memory_store(int m)
{
    const stack_el_t *s = stack_peek();
    if (calc_mode == calc_mode_integer)
    {
        mem_val[m].ival = s->ival;
        if (!use_unsigned)
        {
            /* If in say signed 8bit mode, store -1, then switch to signed 16bit,
             * makes sense for value to be -1 still, so store sign extended
             * rather than masked off to the width. */
            mem_val[m].ival = calc_util_get_signed(mem_val[m].ival, integer_width);
        }
    }
    else
    {
        mem_val[m].fval = s->fval;
    }
    request_display_update(stack_peek());
}

static void memory_recall(int m)
{
    new_arg(mem_val[m].ival, mem_val[m].fval);
    request_display_update(stack_peek());
}

static void memory_plus(int m)
{
    const stack_el_t *s;
    /* My pocket calc treats M+ like an equal, then adds the result
     * to the memory, entering = after M+ does not result in repeated
     * evaluation of last binop (if any). I'll do it the same. */
    bin_op_was_entered = false;
    op_equals();
    s = stack_peek();
    if (calc_mode == calc_mode_integer)
    {
        /* mem val as stored is not necessarily masked to the width */
        uint64_t mem_masked = mem_val[m].ival;
        calc_util_mask_width(&mem_masked, integer_width);
        mem_val[m].ival = bin_iop_add(mem_masked, s->ival);
        if (!use_unsigned)
        {
            /* see comment in memory_store */
            mem_val[m].ival = calc_util_get_signed(mem_val[m].ival, integer_width);
        }
    }
    else
    {
        mem_val[m].fval = bin_fop_add(mem_val[m].fval, s->fval);
    }

    /* Test
     * 10 * 2 = [20], MS, 10 * 3 = [30], MR [20], M+  ==> 40 in M
     */
}


static void enter_pi(void)
{
    if (calc_mode == calc_mode_integer)
        return;

    stackf_t fval;
    dfp_from_string(&fval, "3.1415926535897932384626433832795029", &dfp_context);
    new_arg(0, fval);
    request_display_update(stack_peek());
}

#if 0
static void enter_euler(void)
{
    if (calc_mode == calc_mode_integer)
        return;

    stackf_t fval;
    dfp_from_string(&fval, "2.7182818284590452353602874713526625", &dfp_context);
    new_arg(0, fval);
    request_display_update(stack_peek());
}
#endif

static void enter_rand(void)
{
    if (calc_mode == calc_mode_integer)
        return;

    double r = (double)rand() / (RAND_MAX + 1.0);
    /* so far, 0 <= r < 1
     * random_range == 0 is taken to mean 0 to 1, so nothing more to
     * do in that case.
     * random_range > 0 is taken to mean an integr number in range 1 to
     * random_range so in that case do the conversion.
     */
    if (random_range > 0)
    {
        r = floor(r * random_range) + 1.0;
    }

    /* convert r to decimal float */
    char buf[30];
    sprintf(buf, "%.20f", r);
    stackf_t fval;
    dfp_from_string(&fval, buf, &dfp_context);
    new_arg(0, fval);
    request_display_update(stack_peek());
}

static void enter_int_min(void)
{
    int64_t min;
    switch (integer_width)
    {
    case calc_width_8:
        min = INT8_MIN;
        break;
    case calc_width_16:
        min = INT16_MIN;
        break;
    case calc_width_32:
        min = INT32_MIN;
        break;
    default:
        min = INT64_MIN;
        break;
    }
    uint64_t ival = min;
    calc_util_mask_width(&ival, integer_width);
    stackf_t fval;
    dfp_zero(&fval);
    new_arg(ival, fval);
    request_display_update(stack_peek());
}

void calc_give_op(calc_op_enum cop)
{
    switch (cop)
    {
        case cop_peek:
            request_display_update(stack_peek());
            break;

        case cop_eq:
            op_equals();
            break;

        case cop_add:
            bin_op_common(cop, bin_iop_add, bin_fop_add, PRIORITY_ADD_SUB);
            break;
        case cop_sub:
            bin_op_common(cop, bin_iop_sub, bin_fop_sub, PRIORITY_ADD_SUB);
            break;
        case cop_and:
            bin_op_common(cop, bin_iop_and, NULL, PRIORITY_ADD_SUB);
            break;
        case cop_or:
            bin_op_common(cop, bin_iop_or, NULL, PRIORITY_ADD_SUB);
            break;
        case cop_xor:
            bin_op_common(cop, bin_iop_xor, NULL, PRIORITY_ADD_SUB);
            break;
        case cop_mul:
            bin_op_common(cop, bin_iop_mul, bin_fop_mul, PRIORITY_MUL_DIV);
            break;
        case cop_div:
            bin_op_common(cop, bin_iop_div, bin_fop_div, PRIORITY_MUL_DIV);
            break;
        case cop_mod:
            bin_op_common(cop, bin_iop_mod, bin_fop_mod, PRIORITY_MUL_DIV);
            break;
        case cop_pow:
            bin_op_common(cop, NULL, bin_fop_pow, PRIORITY_POWER_ROOT);
            break;
        case cop_root:
            bin_op_common(cop, NULL, bin_fop_root, PRIORITY_POWER_ROOT);
            break;
        case cop_gcd:
            bin_op_common(cop, bin_iop_gcd, NULL, PRIORITY_MUL_DIV);
            break;
        case cop_lsftn:
            bin_op_common(cop, bin_iop_left_shift, NULL, PRIORITY_MUL_DIV);
            break;
        case cop_rsftn:
            bin_op_common(cop, bin_iop_right_shift, NULL, PRIORITY_MUL_DIV);
            break;

        case cop_pm:
            unary_op(iop_plusminus, fop_plusminus);
            break;
        case cop_com:
            unary_op(iop_complement, NULL);
            break;
        case cop_sqr:
            unary_op(iop_square, fop_square);
            break;
        case cop_sqrt:
            unary_op(NULL, fop_square_root);
            break;
        case cop_onedx:
            unary_op(NULL, fop_one_over_x);
            break;
        case cop_lsft:
            unary_op(iop_left_shift, NULL);
            break;
        case cop_rsft:
            unary_op(iop_right_shift, NULL);
            break;
        case cop_rol:
            unary_op(iop_rol, NULL);
            break;
        case cop_ror:
            unary_op(iop_ror, NULL);
            break;
#if 0
        case cop_2powx:
            unary_op(iop_2powx, NULL);
            break;
#endif


        case cop_log:
            unary_op(NULL, fop_log);
            break;
        case cop_inv_log:
            unary_op(NULL, fop_inv_log);
            break;
        case cop_ln:
            unary_op(NULL, fop_ln);
            break;
        case cop_inv_ln:
            unary_op(NULL, fop_inv_ln);
            break;
        case cop_sin:
            unary_op(NULL, fop_sin);
            break;
        case cop_inv_sin:
            unary_op(NULL, fop_inv_sin);
            break;
        case cop_cos:
            unary_op(NULL, fop_cos);
            break;
        case cop_inv_cos:
            unary_op(NULL, fop_inv_cos);
            break;
        case cop_tan:
            unary_op(NULL, fop_tan);
            break;
        case cop_inv_tan:
            unary_op(NULL, fop_inv_tan);
            break;
        case cop_sinh:
            unary_op(NULL, fop_sinh);
            break;
        case cop_inv_sinh:
            unary_op(NULL, fop_inv_sinh);
            break;
        case cop_cosh:
            unary_op(NULL, fop_cosh);
            break;
        case cop_inv_cosh:
            unary_op(NULL, fop_inv_cosh);
            break;
        case cop_tanh:
            unary_op(NULL, fop_tanh);
            break;
        case cop_inv_tanh:
            unary_op(NULL, fop_inv_tanh);
            break;
        case cop_fact:
            unary_op(NULL, fop_fact);
            break;

        case cop_parl:
            parentheses_left();
            break;
        case cop_parr:
            parentheses_right();
            break;

        case cop_ms:
            memory_store(0);
            break;
        case cop_mr:
            memory_recall(0);
            break;
        case cop_mp:
            memory_plus(0);
            break;
        case cop_ms2:
            memory_store(1);
            break;
        case cop_mr2:
            memory_recall(1);
            break;
        case cop_mp2:
            memory_plus(1);
            break;


        case cop_pi:
            enter_pi();
            break;
#if 0
        case cop_eul:
            enter_euler();
            break;
#endif

        case cop_rand:
            enter_rand();
            break;

        case cop_int_min:
            enter_int_min();
            break;

        default:
            break;
    }
}


void calc_init(int debug_lvl,
               calc_mode_enum mode,
               int rand_range,
               bool sct_round,
               calc_width_enum width,
               bool int_unsigned,
               bool warn_signed,
               bool warn_unsigned)
{
    /* Initialise context for decimal floating point functions. */
    decContextDefault(&dfp_context, DEC_INIT_DECQUAD);

    debug_level = debug_lvl;
    calc_mode = mode;
    random_range = rand_range >= 0 ? rand_range: 0;
    use_sct_rounding = sct_round;
    integer_width = width;
    use_unsigned = int_unsigned;
    warn_on_signed_overflow = warn_signed;
    warn_on_unsigned_overflow = warn_unsigned;

    allow_repeated_equals = false;
    calc_angle = calc_angle_deg;

    for (int i = 0; i < STACK_SIZE; i++)
    {
        stack[i].ival = 0;
        dfp_zero(&stack[i].fval);
    }

    for (int i = 0; i < NUM_MEMORY; i++)
    {
        mem_val[i].ival = 0;
        dfp_zero(&mem_val[i].fval);
    }

    save_val.ival = 0;
    dfp_zero(&save_val.fval);

    /* User should do a calc_clear before starting. */
}

static const char *int_conv_warn = "Signed Integer conversion out of range";
static const char *uint_conv_warn = "Unsigned Integer conversion out of range";

void calc_set_mode(calc_mode_enum mode)
{
    /* Ignore if the same as current mode. */
    if (calc_mode == mode)
        return;

    bin_op_was_entered = false;
    op_equals();

    const stack_el_t *s = stack_peek();

    if (calc_mode == calc_mode_integer)
    {
        /* pass on the current integer value to floating mode, so update
         * save_val.fval */

        /* convert to decimal float */
        char buf[DFP_STRING_MAX];
        if (calc_get_use_unsigned())
        {
            sprintf(buf, "%"PRIu64, s->ival);
        }
        else
        {
            int64_t si = calc_util_get_signed(s->ival, integer_width);
            sprintf(buf, "%" PRId64, si);
        }
        dfp_from_string(&save_val.fval, buf, &dfp_context);
    }
    else
    {
        /* pass on the current floating value to integer mode, so update
         * save_val.ival */

        if (get_best_integer_callback == NULL)
        {
            save_val.ival = 0;
            calc_error("get_best_integer_callback NULL");
        }
        else
        {
            if (!get_best_integer_callback(&save_val.ival, integer_width, use_unsigned))
            {
                if (use_unsigned)
                    calc_warn(uint_conv_warn);
                else
                    calc_warn(int_conv_warn);
            }

        }
    }

    init_from_save_val = true;
    calc_mode = mode;

    /* User should do a calc_clear before using the new mode. */
}

calc_mode_enum calc_get_mode(void)
{
    return calc_mode;
}

void calc_set_angle(calc_angle_enum angle)
{
    calc_angle = angle;
}

calc_angle_enum calc_get_angle(void)
{
    return calc_angle;
}

void calc_set_repeated_equals(bool enable)
{
    allow_repeated_equals = enable;
}

bool calc_get_repeated_equals(void)
{
    return allow_repeated_equals;
}

void calc_clear(void)
{
    stack_index = 0;
    bop_stack_index = 0;
    bin_op_was_entered = false;
    num_parentheses = 0;
    report_num_used_parentheses();
    paren_allowed = true;

    /* Always start out with 0 on stack, unless coming from mode switch */
    if (init_from_save_val)
    {
        init_from_save_val = false;
        calc_util_mask_width(&save_val.ival, integer_width);
        stack_push(save_val.ival, save_val.fval);
    }
    else
    {
        stackf_t dzero;
        dfp_zero(&dzero);
        stack_push(0, dzero);
    }
    request_display_update(stack_peek());
}

void calc_set_result_callback(void (*fn)(uint64_t, stackf_t))
{
    result_callback = fn;
}

void calc_set_history_callback(void (*fn)(uint64_t, stackf_t))
{
    history_callback = fn;
}

void calc_set_get_best_integer_callback(bool (*fn)(uint64_t*, calc_width_enum, bool))
{
    /* For calculating the best integer from a floating point, when doing
     * mode switch from float to integer mode.
     * eg. enter 80, perform some operations like cube root, then cube back,
     * displays (rounded) as 80, but might actually be 79.99999999999999 etc.
     * So when converting to integer it would be truncated to 79, which looks
     * a bit wrong. So convert from the (rounded) value on the display rather
     * than from the underlying floating point value. This obviously means
     * the value can depend on the number of digits in use on the display. */
    get_best_integer_callback = fn;
}

void calc_set_num_paren_callback(void (*fn)(int))
{
    num_paren_callback = fn;
}

void calc_set_warn_callback(void (*fn)(const char *msg))
{
    warn_callback = fn;
}

void calc_set_error_callback(void (*fn)(const char *msg))
{
    error_callback = fn;
}

calc_op_enum calc_get_top_of_bop_stack(void)
{
    if (bop_stack_num_args() > 0)
    {
        const bop_stack_el_t *bop_info = bop_stack_peek();
        return bop_info->cop;
    }
    else
    {
        return cop_nop;
    }
}

void calc_set_random_range(int range)
{
    random_range = range;
    if (random_range < 0)
    {
        /* shouldn't happen, maybe should use unsigned! */
        random_range = 0;
    }
}

void calc_set_use_sct_rounding(bool en)
{
    use_sct_rounding = en;
}

bool calc_get_use_sct_rounding(void)
{
    return use_sct_rounding;
}

bool calc_get_mem_non_zero(unsigned int m)
{
    if (m >= NUM_MEMORY)
        return false;
    if (calc_mode == calc_mode_integer)
        return mem_val[m].ival != 0;
    else
        return !dfp_is_zero(&mem_val[m].fval);
}

stackf_t calc_get_fval_top_of_stack(void)
{
    const stack_el_t *s = stack_peek();
    return s->fval;
}

static void mask_stack_all(void)
{
    for (int i = 0; i < stack_num_args(); i++)
    {
        if (integer_width == calc_width_8)
            stack[i].ival &= 0xff;
        else if (integer_width == calc_width_16)
            stack[i].ival &= 0xffff;
        else if (integer_width == calc_width_32)
            stack[i].ival &= 0xffffffff;
    }
    history_update(stack_peek());
}

static void sign_extend_stack_all(void)
{
    for (int i = 0; i < stack_num_args(); i++)
    {
        stack[i].ival = calc_util_get_signed(stack[i].ival, integer_width);
    }
}

void calc_set_use_unsigned(bool en)
{
    if (use_unsigned == en)
        return;

    use_unsigned = en;
    mask_stack_all();
}

bool calc_get_use_unsigned(void)
{
    return use_unsigned;
}

void calc_set_integer_width(calc_width_enum width)
{
    if (width == integer_width)
        return;

    if (!use_unsigned)
    {
        /* we're in signed mode, sign extend all entries in stack, at
         * current width */
        sign_extend_stack_all();
    }
    /* update width and mask off at new width */
    integer_width = width;
    mask_stack_all();
}

calc_width_enum calc_get_integer_width(void)
{
    return integer_width;
}

void calc_set_warn_on_signed_overflow(bool en)
{
    warn_on_signed_overflow = en;
}

bool calc_get_warn_on_signed_overflow(void)
{
    return warn_on_signed_overflow;
}

void calc_set_warn_on_unsigned_overflow(bool en)
{
    warn_on_unsigned_overflow = en;
}

bool calc_get_warn_on_unsigned_overflow(void)
{
    return warn_on_unsigned_overflow;
}
