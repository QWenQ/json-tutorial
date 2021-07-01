#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL */

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

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}


// 练习三：参考lept_parse_null()的实现和调用方，解析true和false值。

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 'n':  return lept_parse_null(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
        default:   return LEPT_PARSE_INVALID_VALUE;
    }
}

// 练习一：此处修正LEPT_PARSE_ROOT_NOT_SINGULAR，若json在一个值之后，空白之后还有其他字符，则要返回LEPT_PARSE_NOT_SINGULAR

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    // JSON-text = ws value ws, 其中ws可以是零个或者多个
    lept_parse_whitespace(&c);  
    lept_type ret = lept_parse_value(&c, v);
    lept_parse_whitespace(&c);
    if (ret == LEPT_PARSE_OK && lept_parse_value(&c, v) == LEPT_PARSE_OK)  // 若value输入有效，并且 ws value ws 之后，还有其他字符，则返回LEPT_PARSE_SINGULAR
        return LEPT_PARSE_SINGULAR;
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}
