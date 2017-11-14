#include "leptjson.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h> 
#include <math.h>
#include <string.h> 

/* 堆栈 处理*/
#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

typedef struct {
    const char *json;
    char *stack;
    size_t size, top;
} lept_context;

static void* lept_context_push(lept_context *c, size_t size)
{
    assert(size > 0);

    if(c->size == 0) c->size = LEPT_PARSE_STACK_INIT_SIZE;
    while(c->top + size >= c->size)
        c->size += c->size >> 1;
    
    c->stack = (char *)realloc(c->stack, c->size);
    c->top += size;
    return c->stack + c->top;
}

static void* lept_context_pop(lept_context *c, size_t size)
{
    assert(c->top >= size);
    c->top -= size;
    return c->stack + c->top;
}
                        /*(char *)将void*转换成char指针，然后再加个*获取第一个字符 */
#define PUTC(c, ch) do{ *(char *)lept_context_push((c), (sizeof(char))) == (ch); }while(0)

#define EXPECT(c, ch) do{ assert( *c->json == (ch)); c->json++; }while(0)
#define ISDIGIT1TO9(ch) ((ch)>='1' && (ch)<='9')
#define ISDIGIT(ch) ((ch)>='0' && (ch)<='9')
#define lept_init(v) do{ (v)->type = LEPT_NULL; } while(0)

static void lept_parse_whitespace(lept_context *c)
{
    const char *p = c->json;
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context *c, lept_value *v, const char *str, lept_type t)
{
    size_t i;
    EXPECT(c, str[0]);
    for(i = 0; str[i+1]; i++)
    {
        if(c->json[i] != str[i+1]) return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += i;
    v->type = t;
    return LEPT_PARSE_OK;
}

static int lept_parse_string(lept_context* c, lept_value* v)
{
    size_t head = c->top, len;
    EXPECT(c, '\"');
    const char *p = c->json;
    char ch;
    for(;;)
    {
        ch = *p++;
        switch(ch)
        {
            case '\"':
                len = c->top - head;
                lept_set_string(v, (const char *)lept_context_pop(c, len) ,len);
                c->json = p;
                return LEPT_PARSE_OK;
            case '\0':
                c->top = head;
                return LEPT_PARSE_MISS_QUOTATION_MARK;
            default:
                PUTC(c, ch);
        }
    }
}

static int lept_parse_number(lept_context *c, lept_value *v)
{
    const char *p = c->json;
    if( *p == '-') p++;
    if( *p == '0') p++;
    else{
        if(!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
        for( p++; ISDIGIT(*p); p++);
    }
    if(*p =='.')
    {
        p++;
        if(!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for(p++; ISDIGIT(*p); p++);
    }
    if(*p =='e' || *p == 'E')
    {
        p++;
        if(*p =='+' || *p == '-') p++;
        if(!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for(p++; ISDIGIT(*p); p++);
    }
    
    // char *end;
    // v->n = strtod(c->json, &end); /*if json is 123abc  ;  json is abc123*/
    // if(*c->json == *end)     /* *c->json=a , *end=1  ok;  *c->json=a,*end=a fail */
    //     return LEPT_PARSE_INVALID_VALUE;
    errno = 0;
    v->n = strtod(c->json, NULL);
    if(errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;

    c->json = p;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context *c, lept_value *v)
{
    switch(*c->json)
    {
        case 'n':
            return lept_parse_literal(c, v, "null", LEPT_NULL);
        case 'f':
            return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 't':
            return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case '\0':
            return LEPT_PARSE_EXPECT_VALUE;
        case '"':
            return lept_parse_string(c, v);
        default:
            return lept_parse_number(c, v);
    }
}

int lept_parse(lept_value *v, const char *json)
{
    lept_context c;
    assert(v != NULL);
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;
    lept_init(v);
    lept_parse_whitespace(&c);
    int ret = lept_parse_value(&c, v);
    if(ret == LEPT_PARSE_OK)
    {
        lept_parse_whitespace(&c);
        if(*c.json != '\0')
        {
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
            lept_init(v);
        }
    }

    assert(c.top == 0);
    free(c.stack);
    return ret;
}

void lept_free(lept_value *v)
{
    assert(v != NULL);
    if(v->type == LEPT_STRING)
        free(v->u.s.s);

    v->type = LEPT_NULL;
}

/* get set*/

lept_type lept_get_type(const lept_value *v)
{
    assert(v != NULL);
    return v->type;
}

int lept_get_boolean(const lept_value *v)
{
    assert(v != NULL);
    assert(v != LEPT_FALSE || v!= LEPT_TRUE);
    return v->type == LEPT_TRUE;
}

void lept_set_boolean(lept_value *v, int b)
{
    lept_free(v);
    v->type = b ? LEPT_TRUE: LEPT_FALSE;
}

double lept_get_number(const lept_value *v)
{
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->u.n;
}

void lept_set_number(lept_value *v, double n)
{
    lept_free(v);
    v->u.n = n;
    v->type = LEPT_NUMBER;
}

const char * lept_get_string(const lept_value *v)
{
    assert(v != NULL);
    assert(v->type != LEPT_STRING);
    return v->u.s.s;
}

size_t lept_get_string_len(const lept_value *v)
{
    assert(v != NULL);
    assert(v->type != LEPT_STRING);
    return v->u.s.len;
}

void lept_set_string(lept_value *v, const char *s, size_t len)
{
    assert(v != NULL && ( s != NULL || len == 0));
    lept_free(v);
    v->u.s.s = (char *)malloc(len + 1);
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = LEPT_STRING;
}

