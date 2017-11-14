#ifndef LEPTJSON_H__
#define LEPTJSON_H__
#include <stddef.h> /* size_t */

typedef enum {
    LEPT_NULL, 
    LEPT_STRING,
    LEPT_FALSE,
    LEPT_TRUE,
    LEPT_NUMBER,
    LEPT_ARRAY,
    LEPT_OBJECT
} lept_type;

enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,    /*只有空白 */
    LEPT_PARSE_INVALID_VALUE,   /*无效 */
    LEPT_PARSE_ROOT_NOT_SINGULAR,   /*其他多余字符 */
    LEPT_PARSE_NUMBER_TOO_BIG, /* 数字过大*/
    LEPT_PARSE_MISS_QUOTATION_MARK, /*丢失标记，字符串引号结尾， 结尾却没有*/
    LEPT_PARSE_INVALID_STRING_ESCAPE,
    LEPT_PARSE_INVALID_STRING_CHAR
};

typedef struct{
    union{
        struct { size_t len; char *s;}s; /*字符串：长度和字符指针*/
        double n; /*number值*/
    } u;
    lept_type type;
} lept_value;

void lept_free(lept_value *v);

/*字符串 解析成json格式*/
int lept_parse(lept_value *v, const char *json);

lept_type lept_get_type(const lept_value *v);

/*类型处理 set get*/
int lept_get_boolean(const lept_value *v);
void lept_set_boolean(lept_value *v, int b);

double lept_get_number(const lept_value *v);
void lept_set_number(lept_value *v, double n);

const char * lept_get_string(const lept_value *v);
size_t lept_get_string_len(const lept_value *v);
void lept_set_string(lept_value *v, const char *s, size_t len);

#define lept_set_null(v) lept_free(v)
#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)

#endif