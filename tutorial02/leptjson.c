#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <math.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

// 练习一：重构合并lept_parse_true()、lept_parse_false()、lept_parse_null()为lept_parse_literal()。

static int lept_parse_literal(lept_context* c, lept_value* v) {
    char first_char = c->json[0];
    if (first_char == 't')
        EXPECT(c, 't');
    else if (first_char == 'f')
        EXPECT(c, 'f');
    else
        EXPECT(c, 'n');
    
    switch (first_char) {
    case 't':
        if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
            return LEPT_PARSE_INVALID_VALUE;
        c->json += 3;
        v->type = LEPT_TRUE;
        break;
    case 'f':
        if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
            return LEPT_PARSE_INVALID_VALUE;
        c->json += 4;
        v->type = LEPT_FALSE;
        break;
    case 'n':
        if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
            return LEPT_PARSE_INVALID_VALUE;
        c->json += 3;
        v->type = LEPT_NULL;
        break;
    }
    return LEPT_PARSE_OK;

}

/*
static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}
*/


/*
练习三：
按 JSON number 的语法在 lept_parse_number() 校验，不符合标准的程况返回 LEPT_PARSE_INVALID_VALUE 错误码。
在校验成功以后，我们不再使用 end 指针去检测 strtod() 的正确性，第二个参数可传入 NULL。(先校验，后 strtod() 转换)
*/

#define ISDIGIT(ch)       ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1To9(ch)   ((ch) >= '1' && (ch) <= '9')

int is_number_in_exp(const char* p) {    // 解析指数部分
    if (*p == '+' || *p == '-')
        p++;
    char* start = p;
    while (ISDIGIT(*p))
        p++;
    if (p - start > 0)
    {
        //long exp_ans = strtol(start, NULL, 10);
        //if (exp_ans > 308)
            //return LEPT_PARSE_NUMBER_TOO_BIG;
        return LEPT_PARSE_EXPECT_VALUE;
    }
    else
        return LEPT_PARSE_INVALID_VALUE;
}

int is_unsigned_number_in_frac(const char* p) {   // 解析小数部分及指数部分(需要的话)
    char* start = p;
    while (ISDIGIT(*p))
        p++;
    if (p - start > 0)
        if (*p == 'e' || *p == 'E')
        {
            p++;
            return is_number_in_exp(p);
        }   
        else
            return LEPT_PARSE_EXPECT_VALUE;
    else
        return LEPT_PARSE_INVALID_VALUE;
}


// 整数后面的部分
#define AFTER_INT(p_str)\
    do {\
        if (*p_str == '.')\
        {\
            p_str++;\
            return is_unsigned_number_in_frac(p_str);\
        } \
        else if (*p_str == 'e' || *p_str == 'E')\
        {\
            p_str++;\
            return is_number_in_exp(p_str);\
        }\
        else\
            return LEPT_PARSE_EXPECT_VALUE;\
    } while (0)

int is_expect_value(lept_context* c) {   // 解析整数部分、小数部分(需要的话)及指数部分(需要的话)
    const char* p_str = c->json;
//    if (*p_str == '+')  // '+'在整数部分不合法
//        return LEPT_PARSE_INVALID_VALUE;
    if (*p_str == '-')  // '-'在整数部分合法
        p_str++;

    if (ISDIGIT(*p_str))
    {
        if (ISDIGIT1To9(*p_str))  // 若整数部分以 '1 - 9' 开头
        {
            while (ISDIGIT(*p_str))
                p_str++;
            AFTER_INT(p_str);
            
        }
        else  // 若整数部分以 '0' 开头
        {
            p_str++;
            /* after zero should be '.' , 'E' , 'e' or nothing */
            if (*p_str != '.' && *p_str != 'e' && *p_str != 'E' && *p_str != '\0')
                return LEPT_PARSE_ROOT_NOT_SINGULAR;

            AFTER_INT(p_str);                
        }
    }
    else
        return LEPT_PARSE_INVALID_VALUE;
}

static int lept_parse_number(lept_context* c, lept_value* v) {  //解析出来的值符合Json-number语法就ok？

    /* \TODO validate number */
    int lept_parse_ret = is_expect_value(c);
    if (lept_parse_ret == LEPT_PARSE_EXPECT_VALUE)
    {
        char* end;
        v->n = strtod(c->json, &end);
        if (v->n == INFINITY || v->n == -INFINITY)
            return LEPT_PARSE_NUMBER_TOO_BIG;
        c->json = end;
        v->type = LEPT_NUMBER;
        return LEPT_PARSE_OK;
    }
    else
        return lept_parse_ret;
    
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v);
        case 'f':  return lept_parse_literal(c, v);
        case 'n':  return lept_parse_literal(c, v);
        case '-':  return lept_parse_number(c, v);
        case '0':  return lept_parse_number(c, v);
        case '1':  return lept_parse_number(c, v);
        case '2':  return lept_parse_number(c, v);
        case '3':  return lept_parse_number(c, v);
        case '4':  return lept_parse_number(c, v);
        case '5':  return lept_parse_number(c, v);
        case '6':  return lept_parse_number(c, v);
        case '7':  return lept_parse_number(c, v);
        case '8':  return lept_parse_number(c, v);
        case '9':  return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
        default:   return LEPT_PARSE_INVALID_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}