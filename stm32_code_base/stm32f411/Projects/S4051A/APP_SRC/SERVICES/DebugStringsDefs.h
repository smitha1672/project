/** This should be used in case there is no associated String ID*/
#define STR_ID_NULL     0

/** This should be used in case there is no string after the string ID */
#define STR_EMPTY_STRING     ""

#define STR_ID_START_GROUP(index) \
typedef enum\
{\
STR_ID_START_GROUP_ ##index = index - 1,

#define STR_ID_ITEM(str, id) \
id,

#define STR_ID_END_GROUP(name) \
} STR_ID_GROUP_ ##name;

