#ifndef LEPTJSON_H__
#define LEPTJSON_H__

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
    LEPT_PARSE_ROOT_NOT_SINGULAR    /*其他多余字符 */
};

typedef struct{
    lept_type type;
} lept_value;

/*字符串 解析成json格式*/
int lept_parse(lept_value *v, const char *json);

lept_type lept_get_type(const lept_value *v);

#endif