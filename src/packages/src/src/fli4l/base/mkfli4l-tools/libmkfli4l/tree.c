/*----------------------------------------------------------------------------
 *  tree.c - functions for creating a parse tree for extended check scripts
 *
 *  Copyright (c) 2002-2016 - fli4l-Team <team@fli4l.de>
 *
 *  This file is part of fli4l.
 *
 *  fli4l is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  fli4l is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with fli4l.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Last Update: $Id$
 *----------------------------------------------------------------------------
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <var.h>
#include <expression/expr.h>
#include "config.h"
#include "libmkfli4l/log.h"
#include "libmkfli4l/str.h"
#include "libmkfli4l/parse.h"
#include "libmkfli4l/var.h"
#include "libmkfli4l/check.h"
#include "libmkfli4l/tree_struct.h"
#include "libmkfli4l/tree.h"
#include "libmkfli4l/tree_debug.h"
#include "libmkfli4l/grammar.h"

#define ARG_STRING      1
#define ARG_NUM         2
#define ARG_VER         4
#define ARG_ID          8
#define ARG_WEAK        16

static void walk_node(elem_t *p);
static char *numeric_op(elem_t *p);
static char *orelse_op(elem_t *left, elem_t *right, int *type);
static char *cast_op(elem_t *left, elem_t *right, int *type);
static char *elem_get_arg(elem_t *p, int *type);
static char *elem_get_name_type(elem_t *p, int *type);

typedef unsigned long u32;

static elem_t parse_tree;
static elem_t *current_node = &parse_tree;

/************************************
 *
 * Functions used to build parse tree
 *
 ************************************/
elem_t *
mknode(int op, elem_t *arg1, elem_t *arg2, elem_t *arg3, int line)
{
    elem_t *p = (elem_t *) safe_malloc(sizeof(elem_t));

    p->type = NODE;
    p->OP = op;
    p->ARG[0] = arg1;
    p->ARG[1] = arg2;
    p->ARG[2] = arg3;
    p->file = strsave(parse_get_current_file());
    p->package = strsave(parse_get_current_package());
    p->line = line;
    p->next = NULL;

    log_info(LOG_TREE_BUILD, "creating node for %s (%d)\n", get_op_name(op), op);
    dump_elem(LOG_TREE_BUILD, p);

    return p;
}

elem_t *
mkleaf(int type, char const *value)
{
    elem_t *p = (elem_t *) safe_malloc(sizeof(elem_t));

    p->type = LEAF;
    p->TYPE = type;
    p->file = strsave(parse_get_current_file());
    p->package = strsave(parse_get_current_package());
    p->line = yyline;
    p->next = NULL;

    switch (type) {
    case ID:
    case IDSET:
        p->VAL = strsave(convert_to_upper(value));
        break;
    case STRING:
        p->VAL = strsave_quoted(value);
        break;
    default:
        p->VAL = strsave(value);
        break;
    }

    log_info(LOG_TREE_BUILD, "creating leaf for %s = %s\n", get_op_name(type), p->VAL);
    dump_elem(LOG_TREE_BUILD, p);
    return p;
}

elem_t *
add_node(elem_t *node1, elem_t *node2)
{
    elem_t *p = node1;

    while (p->next) {
        p = p->next;
    }

    log_info(LOG_TREE_BUILD, " appending %s to %s\n", get_op_name (node2->OP),
             get_op_name(p->OP));
    p->next = node2;
    return node1;
}

void
add_script(elem_t *node)
{
    current_node->next = mknode(SCRIPT, node, NULL, NULL, 0);
    current_node = current_node->next;
}

/****************************************************
 *
 * Helper functions used to handle function arguments
 *
 ****************************************************/

static char *
elem_get_name(elem_t *p)
{
    int dummy;
    return elem_get_name_type(p, &dummy);
}

/*
 * two examples of possible cases:
 * var_%[id] -- a node with two leafs
 *      - first leaf set var name
 *      - leaf or node representing the index
 *      for instance foo_a_%[1]:
 *              node IDSET
 *                      leaf IDSET=FOO_A_%
 *                      leaf NUM=1
 *
 * var_%_%[id][id]... a node with one or two nodes
 *      for instance foo_a_%_b_%[1][1]:
 *              node IDSET
 *                      node IDSET
 *                              leaf IDSET=FOO_A_%_B_%
 *                              leaf NUM=1
 *                      leaf NUM=1
 */
static char *
elem_get_set_name_type(elem_t *p, int *type)
{
    int   t;
    char *ret;
    char *index = elem_get_arg(p->ARG[1], &t);

    if ((t & ARG_NUM) == 0) {
        fatal_exit ("(%s:%d)You can't use a non-numeric ID "
                    "in a numeric context. Check type of '%s'.\n",
                    p->file, p->line, elem_get_name(p->ARG[1]));
    }

    if (p->ARG[0]->type == LEAF) {
        *type = ARG_ID;
        ret = get_set_var_name_string(p->ARG[0]->VAL, index, p->file, p->line);
        if (is_var_numeric(ret)) {
            log_info(LOG_TREE_EXEC, "%s is numeric\n", ret);
            *type |= ARG_NUM;
        }
    }
    else {
        /* recursively descend until we reach a leaf */
        char *set_var;
        inc_log_indent_level();
        set_var = elem_get_set_name_type(p->ARG[0], type);
        ret = get_set_var_name_string(set_var, index,
                                      p->file, p->line);
        dec_log_indent_level();
    }

    free(index);
    log_info(LOG_TREE_EXEC, "elem_get_set_name_type returning '%s' (%s)\n",
             ret, *type & ARG_NUM ? "numeric" : "non-numeric");
    return ret;
}

static char *
elem_get_name_type(elem_t *p, int *type)
{
    char *ret;

    *type = 0;
    if (p->type == LEAF) {
        switch (p->TYPE) {
        case ID:
            ret = strsave(p->VAL);
            *type = ARG_ID | (is_var_numeric(p->VAL) ? ARG_NUM : 0);
            break;
        case STRING:
            ret =  parse_rewrite_string(p->VAL, p->file, p->line);
            break;
        default:
            fatal_exit("(%s:%d): invalid argument type %s\n",
                       __FILE__, __LINE__, get_op_name(p->TYPE));
        }
    }
    else if (p->OP == IDSET) {
        ret = elem_get_set_name_type(p, type);
    }
    else {
        fatal_exit("(%s:%d): invalid argument type %s\n",
                   __FILE__, __LINE__, get_op_name(p->TYPE));
    }

    log_info(LOG_TREE_EXEC, "(%s:%d) returns '%s' (%s)\n",
             __FILE__, __LINE__, ret,
             *type & ARG_NUM ? "numeric" : "non-numeric");
    return ret;
}

static char *
elem_get_arg(elem_t *p, int *type)
{
    char *name = NULL;
    if (p->type == LEAF) {
        switch (p->TYPE) {
        case STRING:
            *type = ARG_STRING;
            return parse_rewrite_string(p->VAL, p->file, p->line);
        case NUM:
            *type = ARG_NUM;
            return strsave(p->VAL);
        case VER:
            *type = ARG_VER;
            return strsave(p->VAL);
        case ID:
            name = strsave(p->VAL);
            break;
        }
    }
    else {
        switch (p->OP) {
        case IDSET:
            name = elem_get_name(p);
            break;
        case ADD:
        case SUB:
        case MULT:
        case DIV:
        case MOD:
            *type = ARG_NUM;
            return numeric_op(p);
        case ORELSE:
            return orelse_op(p->ARG[0], p->ARG[1], type);
        case CAST:
            return cast_op(p->ARG[0], p->ARG[1], type);
        default:
            fatal_exit("(%s:%d) unexpected operator '%s' in elem_get_arg\n",
                       __FILE__, __LINE__, get_op_name(p->OP));
        }
    }

    if (name) {
        char const *ret = parse_get_variable(name, p->file, p->line);
        if (ret) {
            *type = ARG_ID;
            if (is_var_numeric(name)) {
                log_info(LOG_TREE_EXEC, "(%s:%d): numeric ID %s found\n",
                         __FILE__, __LINE__, name);
                *type |= ARG_NUM;
            }
            else {
                *type |= ARG_STRING;
            }
            free(name);
            return strsave(ret);
        }
        else {
            struct var_instance_t *inst
                = var_instance_create_from_string(var_get_global_scope(), name);

            if (!inst || !var_get_properties(inst->var)->optional) {
                fatal_exit("(%s:%d): unknown variable %s\n",
                           __FILE__, __LINE__, name);
            }
            else {
                free(name);
                *type = ARG_ID;
                if (var_get_type(inst->var)
                        == vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER)) {
                    *type |= ARG_NUM;
                    var_instance_destroy(inst);
                    return strsave("0");
                }
                else {
                    *type |= ARG_STRING;
                    var_instance_destroy(inst);
                    return strsave("");
                }
            }
        }
    }

    fatal_exit("(%s:%d): unexpected argument %s\n",
               __FILE__, __LINE__, get_op_name(p->OP));
}

static void
assert_numeric(elem_t *p, char const *file, int line)
{
    BOOL numeric;
    int type;

    char *name = elem_get_name_type(p, &type);
    numeric = (type & ARG_NUM) != 0;

    if (!numeric) {
        dump_elem(LOG_TREE_EXEC|LOG_INFO, p);
        if (is_var_weak(name)) {
            fatal_exit("(%s:%d) You can't use a non-numeric ID "
                       "in a numeric context.\n"
                       "\tIf the temporary variable %s "
                       "should be a numeric ID,\n"
                       "\tassign a numeric value or a numeric ID to it.\n",
                       file, line, name);

        }
        else {
            char *location = get_variable_src(name);
            fatal_exit("(%s:%d) You can't use a non-numeric ID "
                       "in a numeric context.\n"
                       "\tIf this should be a numeric ID, specify a numeric "
                       "type \n\tfor variable '%s' in %s\n",
                       file, line, name, location);
        }
    }
}

/****************************************
 *
 * Functions used to implement iterations
 *
 ****************************************/

static void
foreach_idset(elem_t *p)
{
    char index_buf[32];
    char *set_var;
    struct iter_t *iter;

    log_info(LOG_TREE_EXEC, "%s: foreach_idset %s in %s (line: %d)\n",
             get_op_name(p->OP),  p->ARG[0]->VAL, p->ARG[1]->VAL, p->line);

    /* kristov TODO: check whether "BASE" may be replaced by NULL */
    if (!(iter = init_set_var_iteration(p->ARG[1]->VAL))) {
        fatal_exit ("%s:%d: Unable to get var_n for variable %s\n", p->file, p->line, p->ARG[1]->VAL);
    }

    while ((set_var = get_next_set_var(iter)) != NULL) {
        char const *val = get_variable(set_var);
        if (!val) {
            if (check_var_optional(p->ARG[1]->VAL)) {
                log_info (LOG_TREE_EXEC, "%s: skipping '%s', "
                          "'%s' optional and undefined\n",
                          get_op_name (p->OP),  set_var, p->ARG[1]->VAL);
                free(set_var);
                continue;
            }
            else {
                fatal_exit ("unknown variable '%s' in %s, line %d\n",
                            set_var, p->file, p->line);
            }
        }

        sprintf(index_buf, "%d", get_last_index(iter));
        log_info(LOG_TREE_EXEC, "%s: executing loop using %s='%s'\n",
                 get_op_name(p->OP), p->ARG[0]->VAL, val);
        inc_log_indent_level();
        /* TODO: add slot extra data */
        var_add_weak_declaration(p->package, p->ARG[0]->VAL,
                                 val,
                                 index_buf,
                                 set_var,
                                 is_var_numeric(set_var) ?
                                 TYPE_NUMERIC : TYPE_UNKNOWN,
                                 p->file, p->line, LOG_TREE_EXEC,
                                 PACKAGE_FILE_TYPE_EXTCHECK);
        free(set_var);
        walk_node(p->ARG[2]);
        dec_log_indent_level();
    }

    end_set_var_iteration(iter);
}

static void
foreach_idset_set(elem_t *p)
{
    elem_t e = *p;
    p = p->ARG[1];
    while (p) {
        e.ARG[1] = p->ARG[0];
        foreach_idset(&e);
        p = p->ARG[2];
    }
}

static void
foreach_id (elem_t *p)
{
    int index;
    int max_index;
    char index_buf[32];

    /* kristov TODO: parse_get_variable may return NULL */
    char const *val = parse_get_variable (p->ARG[1]->VAL, p->file, p->line);

    log_info(LOG_TREE_EXEC, "%s: foreach_id %s in %s (line: %d)\n",
             get_op_name(p->OP),  p->ARG[0]->VAL, p->ARG[1]->VAL, p->line);

    assert_numeric(p->ARG[1], p->file, p->line);

    max_index = convert_to_long(val, p->file, p->line);
    for(index = 1; index <= max_index; ++index)
    {
        int current_index;
        sprintf(index_buf, "%d", index);
        log_info(LOG_TREE_EXEC, "%s: executing loop using %s='%s'\n",
                 get_op_name(p->OP),  p->ARG[0]->VAL, index_buf);

        inc_log_indent_level();
        var_add_weak_declaration(p->package, p->ARG[0]->VAL,
                                 index_buf, index_buf, NULL, TYPE_NUMERIC,
                                 p->file, p->line, LOG_TREE_EXEC,
                                 PACKAGE_FILE_TYPE_EXTCHECK);
        walk_node(p->ARG[2]);
        dec_log_indent_level();

        /* kristov TODO: parse_get_variable may return NULL */
        val = parse_get_variable(p->ARG[0]->VAL, p->file, p->line);
        current_index = convert_to_long(val, p->file, p->line);

        if (index != current_index) {
            log_info(LOG_TREE_EXEC, "changing loop variable from %d to %d\n",
                     index, current_index);
            index = current_index;
        }
    }
}

/***************************
 *
 * numeric_op
 *
 ***************************/
static unsigned long
get_numeric_value(elem_t *p)
{
    int type;
    char *val = elem_get_arg (p, &type);
    if (!(type & ARG_NUM)) {
        fatal_exit ("(%s:%d) You can't use a non-numeric ID "
                    "in a numeric context. Check type of operand.\n",
                    p->file, p->line);
    }
    return convert_to_long(val, p->file, p->line);
}

char *
numeric_op(elem_t *p)
{
    unsigned long x, y, res;
    char buf[32];

    x = get_numeric_value(p->ARG[0]);
    y = get_numeric_value(p->ARG[1]);
    switch (p->OP) {
    case ADD:
        res = x + y;
        break;
    case SUB:
        res = x - y;
        break;
    case MULT:
        res = x * y;
        break;
    case DIV:
        if (y == 0) {
            fatal_exit("(%s:%d) divide by zero\n", p->file, p->line);
        }
        res = x / y;
        break;
    case MOD:
        if (y == 0) {
            fatal_exit("(%s:%d) divide by zero\n", p->file, p->line);
        }
        res = x % y;
        break;
    default:
        fatal_exit("(%s:%d) unknown operation\n", p->file, p->line);
    }

    sprintf(buf, "%lu", res);
    return strsave (buf);
}

/***************************
 *
 * orelse_op
 *
 ***************************/
char *
orelse_op(elem_t *left, elem_t *right, int *type)
{
    char *name;
    char const *value;
    int left_type;

    if ((left->type == LEAF && left->TYPE == ID) || left->OP == IDSET) {
        name = elem_get_name_type(left, &left_type);
    }
    else {
        fatal_exit("(%s:%d): unexpected argument %s\n",
                   __FILE__, __LINE__, get_op_name(left->OP));
    }

    value = try_get_variable(name);
    free(name);
    if (value) {
        *type = left_type;
        return strsave(value);
    }
    else {
        return elem_get_arg(right, type);
    }
}

/***************************
 *
 * cast_op
 *
 ***************************/
char *
cast_op(elem_t *left, elem_t *right, int *type)
{
    char *value = elem_get_arg(left, type);
    struct vartype_t *right_type = vartype_get(convert_to_upper(right->VAL));

    if (!right_type) {
        fatal_exit("cast to invalid type '%s' in %s, line %d\n",
                   right->VAL, left->package, left->line);
    }

    if (vartype_match(right_type, value)) {
        *type = *type & ~(ARG_NUM | ARG_STRING);
        switch (vartype_get_expression_type(right_type)) {
        case EXPR_TYPE_INTEGER :
            *type |= ARG_NUM;
            break;
        default :
            /* no libmkfli4l type for boolean */
            *type = ARG_STRING;
            break;
        }
        return value;
    }
    else {
        fatal_exit("casting '%s' to type '%s' failed in %s, line %d\n",
                   value, right->VAL, left->package, left->line);
    }
}

/***************************
 *
 * assign
 *
 ***************************/
static void
assign(elem_t *p)
{
    char *var = p->ARG[0]->VAL;
    int type = 0;

    if (p->ARG[0]->TYPE == ID) {
        if (p->ARG[1] == NULL) {
            var_add_weak_declaration(p->package, p->ARG[0]->VAL, "yes", NULL, NULL,
                                     TYPE_UNKNOWN, p->file, p->line, LOG_TREE_EXEC,
                                     PACKAGE_FILE_TYPE_EXTCHECK);
        }
        else {
            char *val = elem_get_arg(p->ARG[1], &type);
            var_add_weak_declaration(p->package, p->ARG[0]->VAL, val, NULL, NULL,
                                     type & ARG_NUM ? TYPE_NUMERIC : TYPE_UNKNOWN,
                                     p->file, p->line, LOG_TREE_EXEC,
                                     PACKAGE_FILE_TYPE_EXTCHECK);
            free(val);
        }
    }
    else {
        struct var_t *v;
        struct var_instance_t *inst;
        unsigned num_index;
        char *val;

        val = elem_get_arg(p->ARG[2], &type);
        v = var_try_get(var);
        if (!v) {
            v = check_add_weak_declaration(p->package, var,
                                           type & ARG_NUM ? TYPE_NUMERIC : TYPE_UNKNOWN,
                                           FALSE, p->file, p->line, LOG_TREE_EXEC);
        }
        if (!v) {
            fatal_exit("(%s:%d) cannot create variable %s\n",
                       p->file, p->line, var);
        }

        num_index = (unsigned) atoi(elem_get_arg(p->ARG[1], &type));
        if (!(type & ARG_NUM)) {
            fatal_exit("(%s:%d) non-numeric index\n",
                       p->file, p->line);
        }

        /* core arrays are zero-based, but mkfli4l's ones are one-based */
        --num_index;
        inst = var_instance_create(v, 1, &num_index);
        var_instance_set_and_check_value(inst, val, NULL, VAR_ACCESS_WEAK,
                                         VAR_PRIORITY_SCRIPT_TRANSIENT);
        var_instance_destroy(inst);
    }
}

/***************************
 *
 * split
 *
 ***************************/
static void
split(elem_t *e, int type)
{
    char *var;
    char  c;

    char buf[32];
    char *p;
    int dummy;
    struct var_t *v;
    struct var_instance_t *inst;

    p = elem_get_arg(e->ARG[0], &dummy);
    var = e->ARG[1]->VAL;
    c = e->ARG[2]->VAL[1];
    strcpy(buf, "0");

    v = check_add_weak_declaration(e->package, var, type, TRUE,
                                   e->file, e->line, LOG_TREE_EXEC);
    if (!v) {
        fatal_exit("(%s:%d) cannot create variable %s\n",
                   e->file, e->line, var);
    }

    inst = var_instance_create(v, 1, NULL);
    while (*p) {
        char *q = p;
        char *sub = p;

        if (c == ' ') {
            while (*q && !isspace((int) *q)) {
                ++q;
            }
            if (*q && *(q + 1) && isspace((int) *(q + 1))) {
                *q = '\0';
                while (*(q + 1) && isspace((int) *(q + 1))) {
                    ++q;
                }
            }
        }
        else {
            while (*q && *q != c) {
                ++q;
            }
        }

        if (*q) {
            p = q + 1;
        }
        else {
            p = q;
        }
        *q = '\0';

        var_instance_set_and_check_value(inst, sub, NULL, VAR_ACCESS_WEAK,
                                         VAR_PRIORITY_SCRIPT_TRANSIENT);
        ++inst->indices[0];
    }
    var_instance_destroy(inst);
}

/***************************
 *
 * condition implementations
 *
 ***************************/
static BOOL
cond_id(elem_t *p)
{
    BOOL ret;
    char const *val;
    char *name;

    if (p->TYPE != ID && p->TYPE != IDSET) {
        fatal_exit("(%s:%d) impossible boolean expression: "
                   "if ( %s ) then...\n",
                   p->file, p->line,
                   get_op_name (p->TYPE));
    }

    name = elem_get_name(p);
    log_info(LOG_TREE_EXEC, "%s: %s == '%s'\n",
             get_op_name(p->OP), get_op_name(p->TYPE), name);

    val = get_variable (name);
    ret = val == NULL ? FALSE : strcmp(val, "yes") == 0;
    log_info(LOG_TREE_EXEC, "%s: returning %s (%s = '%s')\n",
             get_op_name(p->OP), ret ? "true" : "false",
             name, val ? val : "undefined");

    free(name);
    return ret;
}

static BOOL
cond_copy_pending(elem_t *p)
{
    BOOL ret;
    char *name;

    expect_types(p, SOMETHING, NONE, NONE, __LINE__);

    name = elem_get_name(p->ARG[0]);
    log_info(LOG_TREE_EXEC, "%s: %s == '%s'\n",
             get_op_name(p->OP), get_op_name(p->ARG[0]->TYPE), name);

    ret = is_var_copy_pending(name);
    log_info(LOG_TREE_EXEC, "%s: returning %s\n",
             get_op_name(p->OP), ret ? "true" : "false");

    free(name);
    return ret;
}

static BOOL
cond_unique(elem_t *p, char const *file, int line)
{
    BOOL ret;
    char *name;

    expect_types(p, SOMETHING, NONE, NONE, __LINE__);

    name = elem_get_name(p->ARG[0]);
    log_info (LOG_TREE_EXEC, "%s: %s == '%s'\n",
              get_op_name(p->OP), get_op_name(p->ARG[0]->TYPE), name);

    log_error("(%s:%d) \"unique\" is not supported anymore\n",
              file, line);

    ret = FALSE;
    log_info (LOG_TREE_EXEC, "%s: returning %s\n",
              get_op_name (p->OP), ret ? "true" : "false");

    free(name);
    return ret;
}

static BOOL
my_inet_aton(char const *cp, u32 *s_addr)
{
    unsigned long addr;
    int part;

    addr = 0;
    for (part = 1; part <= 4; ++part) {
        int value;
        if (!isdigit((int) *cp)) {
            return FALSE;
        }

        value = 0;
        while (isdigit((int) *cp)) {
            value *= 10;
            value += *cp++ - '0';
            if (value > 255) {
                return FALSE;
            }
        }

        if (part < 4) {
            if (*cp++ != '.') {
                return FALSE;
            }
        } else {
            char c = *cp++;
            if (c != '\0' && !isspace((int) c)) {
                return FALSE;
            }
        }

        addr <<= 8;
        addr |= value;
    }

    *s_addr = addr;
    return TRUE;
}

static int
get_netmask_bits(u32 mask)
{
    u32 n = ~mask + 1; /* equivalent to ~(mask - 1) */
    if ((mask & n) != n) {
        /* not a 1*0* mask */
        return 0;
    }
    else {
        /*
         * no efficient inverse function f with f(1 << y) == y available
         * (without resorting to binary logarithm ld)
         */
        int bits = 0;
        while (n != 0) {
            ++bits;
            n <<= 1;
        }
        return bits;
    }
}

static u32
get_netmask(int netmask_bits)
{
    if (netmask_bits > 0 && netmask_bits <= 32) {
        return (~(1uL << (32 - netmask_bits)) + 1uL) & 0xFFFFFFFFuL;
    }
    else {
        return 0;
    }
}

static void
normalize_network(char *network, u32 *ip, int *netmask_bits,
                  char const *file, int line)
{
    const char delim[3] = {'/', ':', ' '};
    u32 netmask = 0;

    char * nm = 0;
    int i;

    *netmask_bits = 32;

    for(i = 0; i < 3 && !nm; ++i) {
        nm = strchr(network, delim[i]);
    }

    if (nm) {
        *nm++ = '\0';
        if (strchr(nm, '.') != NULL) {
            if (!my_inet_aton(nm, &netmask)) {
                fatal_exit ("%s:%d : Invalid IPv4 netmask %s\n", file, line, nm);
            }
            *netmask_bits = get_netmask_bits(netmask);
        }
        else {
            sscanf (nm, "%d", netmask_bits);
            if (*netmask_bits < 0 || *netmask_bits > 32) {
                fatal_exit ("%s:%d : Invalid IPv4 netmask '/%s'\n", file, line,
                            network);
            }
        }
    }

    netmask = get_netmask(*netmask_bits);

    if (!my_inet_aton(network, ip)) {
        fatal_exit ("%s:%d : Invalid IPv4 address %s\n", file, line, network);
    }

    *ip &= netmask;
}

static BOOL
cond_same_net(elem_t *p, char const *file, int line)
{
    BOOL ret;
    int dummy;
    char *left_net;
    char *right_net;
    u32 left_ip;
    u32 right_ip;
    int left_netmask_bits;
    int right_netmask_bits;

    expect_types(p, SOMETHING, SOMETHING, NONE, __LINE__);

    left_net = parse_rewrite_string(elem_get_arg(p->ARG[0], &dummy), file, line);
    right_net = parse_rewrite_string(elem_get_arg (p->ARG[1], &dummy), file, line);
    log_info(LOG_TREE_EXEC, "%s: '%s' == '%s'\n",
             get_op_name(p->OP), left_net, right_net);

    normalize_network(left_net, &left_ip, &left_netmask_bits, file, line);
    normalize_network(right_net, &right_ip, &right_netmask_bits, file, line);
    ret = left_ip == right_ip && left_netmask_bits == right_netmask_bits;
    log_info(LOG_TREE_EXEC, "%s: returning %s\n",
             get_op_name(p->OP), ret ? "true" : "false");

    free(left_net);
    free(right_net);
    return ret;
}

static BOOL
cond_sub_net(elem_t *p, char const *file, int line)
{
    BOOL ret;
    int dummy;
    char *left_net;
    char *right_net;
    u32 left_ip;
    u32 right_ip;
    int left_netmask_bits;
    int right_netmask_bits;

    expect_types(p, SOMETHING, SOMETHING, NONE, __LINE__);

    left_net = parse_rewrite_string(elem_get_arg(p->ARG[0], &dummy), file, line);
    right_net = parse_rewrite_string(elem_get_arg(p->ARG[1], &dummy), file, line);
    log_info(LOG_TREE_EXEC, "%s: '%s' subnet of '%s'\n",
              get_op_name(p->OP), right_net, left_net);

    normalize_network(left_net, &left_ip, &left_netmask_bits, file, line);
    normalize_network(right_net, &right_ip, &right_netmask_bits, file, line);
    if (right_netmask_bits <= left_netmask_bits) {
        u32 nm = get_netmask(right_netmask_bits);
        left_ip &= nm;
        ret = left_ip == right_ip;
    }
    else {
        ret = FALSE;
    }
    log_info(LOG_TREE_EXEC, "%s: returning %s\n",
             get_op_name(p->OP), ret ? "true" : "false");

    free(left_net);
    free(right_net);
    return ret;
}

static BOOL
cond_defined(elem_t *p, char const *file, int line)
{
    char *name = NULL;
    BOOL ret;

    expect_types(p, SOMETHING, NONE, NONE, __LINE__);
    p = p->ARG[0];

    dump_elem(LOG_TREE_EXEC, p);
    if (p->type == LEAF) {
        switch (p->TYPE) {
        case STRING:
            name = parse_rewrite_string(p->VAL, file, line);
            break;
        case ID:
            name = strsave(p->VAL);
            break;
        default :
            fatal_exit("(%s:%d) unexpected argument %s '%s'\n", __FILE__, __LINE__, get_op_name(p->TYPE), p->VAL);
            break;
        }
    }
    else if (p->OP == IDSET) {
        name = elem_get_name(p);
    }
    else {
        fatal_exit("(%s:%d) unexpected argument %s\n", __FILE__, __LINE__, get_op_name(p->OP));
    }

    log_info(LOG_TREE_EXEC, "DEFINED: %s '%s'\n", get_op_name(p->TYPE), name);

    ret = try_get_variable(name) && !is_var_generated(name);
    log_info(LOG_TREE_EXEC, "%s: returning %s\n", get_op_name(p->OP),
             ret ? "true" : "false");

    free(name);
    return ret;
}

static BOOL
cond_match(elem_t *p)
{
    BOOL ret;
    char *name;
    char const *val;
    char *regexp;

    expect_types(p, SOMETHING, LEAF, NONE, __LINE__);

    name = elem_get_name(p->ARG[0]);
    val = parse_get_variable(name, p->file, p->line);
    if (val == NULL) {
        fatal_exit("(%s:%d) unknown variable %s\n", __FILE__, __LINE__, name);
    }

    regexp = elem_get_name(p->ARG[1]);

    log_info(LOG_TREE_EXEC, "%s: left hand %s == '%s', right hand %s == '%s'\n",
             get_op_name(p->OP),
             get_op_name(p->ARG[0]->TYPE), val,
             get_op_name(p->ARG[1]->TYPE), regexp);

    ret = str_strmatch(val, regexp, p->package, p->file, p->line);

    log_info(LOG_TREE_EXEC, "%s: returning %s\n",
             get_op_name(p->OP),
             ret ? "true" : "false");

    free(name);
    free(regexp);
    return ret;
}

/* ID|IDSET rel ID|IDSET
   ID|IDSET rel STRING
   ID|IDSET rel NUM
   ID|IDSET rel VER
*/
static BOOL
cond_relop(elem_t *p, char const *file, int line)
{
    char *left_val;
    char *right_val;
    int left_type;
    int right_type;
    int arg_types;
    int cmpret;
    BOOL ret;

    expect_types(p, SOMETHING, SOMETHING, NONE, __LINE__);

    left_val  = elem_get_arg(p->ARG[0], &left_type);
    right_val = elem_get_arg(p->ARG[1], &right_type);
    arg_types = (left_type | right_type) & (ARG_NUM | ARG_STRING | ARG_VER);

    log_info(LOG_TREE_EXEC, "%s: left hand %s == '%s', right hand %s == '%s'\n",
             get_op_name(p->OP),
             get_op_name(p->ARG[0]->TYPE), left_val,
             get_op_name(p->ARG[1]->TYPE), right_val);

    switch (arg_types) {
    case ARG_STRING :
        cmpret = strcmp(left_val, right_val);
        if (cmpret < 0) {
            cmpret = CMP_LESS;
        }
        else if (cmpret > 0) {
            cmpret = CMP_GREATER;
        }
        else {
            cmpret = CMP_EQUAL;
        }

        log_info(LOG_TREE_EXEC, "strcmp (%s, %s) returned %d\n",
                 left_val, right_val, cmpret);
        break;

    case ARG_STRING | ARG_NUM :
        if ((left_type & ARG_ID) != 0) {
            assert_numeric(p->ARG[0], file, line);
        }
        if ((right_type & ARG_ID) != 0) {
            assert_numeric(p->ARG[0], file, line);
        }
        /* fall through */

    case ARG_NUM :
        cmpret = val_numcmp(left_val, right_val, file, line);
        log_info(LOG_TREE_EXEC, "val_numcmp  (%s, %s) returned %d\n",
                 left_val, right_val, cmpret);
        break;

    case ARG_VER :
    case ARG_VER | ARG_STRING :
        cmpret = val_vercmp(left_val, right_val, file, line);
        log_info(LOG_TREE_EXEC, "val_numcmp (%s, %s) returned %d\n",
                 left_val, right_val, cmpret);
        break;

    case ARG_VER | ARG_NUM :
        fatal_exit("(%s:%d) you can't compare a version with "
                   "a plain number\n", __FILE__, __LINE__);

    default:
        fatal_exit ("(%s:%d) unknown arguments for comparison\n",
                    __FILE__, __LINE__);
    }

    switch (p->OP) {
    case EQUAL:
        ret = (cmpret == CMP_EQUAL);
        break;
    case NOT_EQUAL:
        ret = (cmpret != CMP_EQUAL);
        break;
    case LESS:
        ret = (cmpret == CMP_LESS);
        break;
    case GREATER:
        ret = (cmpret == CMP_GREATER);
        break;
    case GE:
        ret = (cmpret != CMP_LESS);
        break;
    case LE:
        ret = (cmpret != CMP_GREATER);
        break;
    default:
        fatal_exit("(%s:%d) unknown/wrong relational operation: '%s'\n",
                   __FILE__, __LINE__, get_op_name(p->OP));
    }

    log_info(LOG_TREE_EXEC, "%s: returning %s\n",
             get_op_name(p->OP),
             ret ? "true" : "false");

    return ret;
}

/********************************************************************
 *
 * get the result of a condition by simply walking the condition tree
 *
 ********************************************************************/

static BOOL
walk_condition(elem_t *p, char const *file, int line)
{
    BOOL ret;
    int type;

    log_info(LOG_TREE_EXEC, "condition: %s (%d)\n", get_op_name(p->OP), p->OP);
    inc_log_indent_level();

    switch (p->OP) {
    case ID:
        ret = cond_id(p);
        break;
    case COPY_PENDING:
        ret = cond_copy_pending(p);
        break;
    case UNIQUE:
        ret = cond_unique(p, file, line);
        break;
    case DEFINED:
        ret = cond_defined(p, file, line);
        break;
    case MATCH:
        ret = cond_match(p);
        break;
    case SAMENET:
        ret = cond_same_net(p, file, line);
        break;
    case SUBNET:
        ret = cond_sub_net(p, file, line);
        break;
    case EQUAL:
    case NOT_EQUAL:
    case LESS:
    case GREATER:
    case LE:
    case GE:
        ret = cond_relop(p, file, line);
        break;
    case NOT:
        expect_types(p, SOMETHING, NONE, NONE, __LINE__);
        ret = !walk_condition(p->ARG[0], file, line);
        break;
    case AND:
        expect_types(p, SOMETHING, SOMETHING, NONE, __LINE__);
        ret = walk_condition(p->ARG[0], file, line) &&
            walk_condition(p->ARG[1], file, line);
        break;
    case OR:
        expect_types(p, SOMETHING, SOMETHING, NONE, __LINE__);
        ret = walk_condition(p->ARG[0], file, line) ||
            walk_condition(p->ARG[1], file, line);
        break;
    case ORELSE:
        expect_types(p, SOMETHING, SOMETHING, NONE, __LINE__);
        ret = strcmp(orelse_op(p->ARG[0], p->ARG[1], &type), "yes") == 0;
        break;
    default:
        fatal_exit("(%s:%d) impossible boolean expression: %s\n",
                   p->file, p->line, get_op_name (p->OP));
    }

    dec_log_indent_level();
    log_info(LOG_TREE_EXEC, "condition: '%s' returns '%s'\n",
             get_op_name(p->OP), ret ? "true" : "false");
    return ret;
}

/****************************************************
 *
 * Execute a script by recursively walking the parse tree
 *
 ****************************************************/

static void
walk_node(elem_t *p)
{
    if (!p) {
        log_info (LOG_TREE_EXEC, "empty tree\n");
    }

    while (p) {
        BOOL cond_res;
        expect_node(p, __LINE__, -1, 0);

        switch (p->OP) {
        case WARNING :
            expect_types(p, LEAF, NONE, NONE, __LINE__);
            log_info(LOG_TREE_EXEC, "%s: \"%s\" (line: %d)\n",
                     get_op_name(p->OP), p->ARG[0]->VAL, p->line);
            parse_warning (p->ARG[0]->VAL, p->file, p->line);
            break;

        case ERROR :
            expect_types(p, LEAF, NONE, NONE, __LINE__);
            log_info(LOG_TREE_EXEC, "%s: \"%s\" (line: %d)\n",
                     get_op_name(p->OP), p->ARG[0]->VAL, p->line);
            parse_error(p->ARG[0]->VAL, p->file, p->line);
            break;

        case FATAL_ERROR :
            expect_types(p, LEAF, NONE, NONE, __LINE__);
            log_info(LOG_TREE_EXEC, "%s: \"%s\" (line: %d)\n",
                     get_op_name(p->OP), p->ARG[0]->VAL, p->line);
            parse_fatal_error(p->ARG[0]->VAL, p->file, p->line);
            break;

        case ADD_TO_OPT :
            expect_types(p, LEAF, LEAF|OPT, NONE, __LINE__);
            if (p->ARG[1]) {
                log_info(LOG_TREE_EXEC, "%s: \"%s\" \"%s\" (line: %d)\n",
                         get_op_name(p->OP), p->ARG[0]->VAL,
                         p->ARG[1]->VAL, p->line);
                parse_add_to_opt(p->ARG[0]->VAL, p->ARG[1]->VAL, p->file,
                                 p->line);
            }
            else {
                log_info(LOG_TREE_EXEC, "%s: \"%s\" (line: %d)\n",
                         get_op_name(p->OP), p->ARG[0]->VAL, p->line);
                parse_add_to_opt(p->ARG[0]->VAL, NULL, p->file, p->line);
            }
            break;

        case FGREP :
            expect_types(p, LEAF, LEAF, NONE, __LINE__);
            log_info(LOG_TREE_EXEC, "%s: \"%s\" (line: %d)\n",
                     get_op_name(p->OP), p->ARG[0]->VAL, p->line);
            parse_fgrep(p->ARG[0]->VAL, p->ARG[1]->VAL, p->package, p->file, p->line);
            break;

        case STAT :
            expect_types(p, LEAF, LEAF, NONE, __LINE__);
            log_info(LOG_TREE_EXEC, "%s: \"%s\" (line: %d)\n",
                     get_op_name(p->OP), p->ARG[0]->VAL, p->line);
            parse_stat(p->ARG[0]->VAL, p->ARG[1]->VAL, p->package, p->file, p->line);
            break;

        case CRYPT :
            expect_types(p, SOMETHING, NONE, NONE, __LINE__);
            log_info(LOG_TREE_EXEC, "%s: \"%s\" (line: %d)\n",
                     get_op_name(p->OP), p->ARG[0]->VAL, p->line);
            parse_crypt(elem_get_name(p->ARG[0]), p->package, p->file, p->line);
            break;

        case SPLIT :
        case SPLIT | SPLIT_NUMERIC :
            expect_types(p, SOMETHING, LEAF, LEAF, __LINE__);
            log_info(LOG_TREE_EXEC, "%s: \"%s\" (line: %d)\n",
                     get_op_name(p->OP), p->ARG[0]->VAL, p->line);
            split(p, p->OP & SPLIT_NUMERIC ? TYPE_NUMERIC : TYPE_UNKNOWN);
            break;

        case PROVIDES :
            expect_types(p, LEAF, LEAF, NONE, __LINE__);
            log_info(LOG_TREE_EXEC, "%s: '%s' version '%s' (line: %d)\n",
                     get_op_name(p->OP), p->ARG[0]->VAL, p->ARG[1]->VAL,
                     p->line);
            parse_provides(p->ARG[0]->VAL,  p->ARG[1]->VAL, p->package, p->file,
                           p->line);
            break;

        case DEPENDS :
            expect_types(p, LEAF, ANYTHING, NONE, __LINE__);
            log_info(LOG_TREE_EXEC, "%s: on '%s' (line: %d)\n",
                     get_op_name(p->OP), p->ARG[0]->VAL,
                     p->line);
            parse_depends(p->ARG[0]->VAL,  p->ARG[1], p->package,
                          p->file, p->line);
            break;

        case IF :
            expect_types(p, SOMETHING, NODE, NODE|OPT, __LINE__);
            log_info(LOG_TREE_EXEC, "IF: checking condition... (line: %d)\n",
                     p->line);

            inc_log_indent_level();
            cond_res = walk_condition(p->ARG[0], p->file, p->line);
            dec_log_indent_level();

            if (cond_res) {
                log_info(LOG_TREE_EXEC, "IF: executing then statements (line: %d)\n",
                         p->ARG[1]->line);
                inc_log_indent_level();
                walk_node(p->ARG[1]);
                dec_log_indent_level();
            }
            else if (p->ARG[2]) {
                log_info(LOG_TREE_EXEC, "IF: executing else statements"
                         "(line: %d)\n", p->ARG[2]->line);
                inc_log_indent_level();
                walk_node (p->ARG[2]);
                dec_log_indent_level();
            }
            else {
                log_info(LOG_TREE_EXEC, "IF: empty else statement\n");
            }
            break;

        case FOREACH :
            expect_types(p, LEAF, SOMETHING, NODE, __LINE__);
            if (p->ARG[1]->type == LEAF) {
                if (p->ARG[1]->TYPE == IDSET) {
                    foreach_idset(p);
                }
                else {
                    foreach_id(p);
                }
            }
            else {
                foreach_idset_set(p);
            }
            break;

        case ASSIGN :
            expect_types(p, SOMETHING, SOMETHING|OPT, SOMETHING|OPT, __LINE__);
            log_info(LOG_TREE_EXEC, "%s: '%s' (line: %d)\n",
                     get_op_name(p->OP), p->ARG[0]->VAL, p->line);
            assign(p);
            break;

        case SCRIPT :
            expect_types(p, NODE, NONE, NONE, __LINE__);
            log_info(LOG_INFO|LOG_TREE_EXEC, "executing extended checks in %s\n",
                     p->file);
            inc_log_indent_level();
            walk_node(p->ARG[0]);
            dec_log_indent_level();
            break;

        default:
            log_error("(%s:%d) unknown case: %d:  (%s)\n",
                      __FILE__, __LINE__, p->OP, get_op_name(p->OP));
            break;
        }

        p = p->next;
        if (p && p->OP != SCRIPT) {
            log_info(LOG_TREE_EXEC, "choosing next statement\n");
        }
    }
}

void
walk_tree(void)
{
    walk_node(parse_tree.next);
}

void
reorder_check_scripts(void)
{
    /* "base" must be the first one... */
    elem_t *first = &parse_tree;
    elem_t *previous = first;
    elem_t *last = NULL;
    elem_t *node = first;
    while ((node = node->next) != NULL) {
        if (node->OP == SCRIPT && strcmp(node->package, PKG_BASE) == 0) {
            previous->next = node->next;
            node->next = first->next;
            first->next = node;
            break;
        }
        else {
            previous = node;
        }
    }

    /* search for last script */
    previous = first;
    node = first;
    while ((node = node->next) != NULL) {
        previous = node;
    }
    last = previous;

    /* ...and "circuits" must be the last one */
    previous = first;
    node = first;
    while ((node = node->next) != NULL) {
        if (node->OP == SCRIPT && strcmp(node->package, PKG_CIRCUITS) == 0) {
            previous->next = node->next;
            if (previous->next == NULL) {
                last = previous;
            }
            node->next = NULL;
            last->next = node;
            break;
        }
        else {
            previous = node;
        }
    }
}
