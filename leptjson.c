#include "leptjson.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h> 
#include <math.h>

typedef struct {
    const char *json;
} lept_context;

#define EXPECT(c, ch) do{ assert( *c->json == (ch)); c->json++; }while(0)
#define ISDIGIT1TO9(ch) ((ch)>='1' && (ch)<='9')
#define ISDIGIT(ch) ((ch)>='0' && (ch)<='9')

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

static int lept_parse_number(lept_context *c, lept_value *v)
{
    const char *p = c->json;
    if( *p == '-') p++;
    if( *p == '0') p++;
    else{
        if(!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
        for( p++; ISDIGIT(*p); p++)
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
    if(errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;

    v->n = strtod(c->json, NULL);
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
        default:
            return lept_parse_number(c, v);
    }
}

int lept_parse(lept_value *v, const char *json)
{
    lept_context c;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    int ret = lept_parse_value(&c, v);
    if(ret == LEPT_PARSE_OK)
    {
        lept_parse_whitespace(&c);
        if(*c.json != '\0')
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    return ret;
}

lept_type lept_get_type(const lept_value *v)
{
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value *v)
{
    assert(v != NULL);
    assert(v->type != LEPT_NUMBER);
    return v->n;
}