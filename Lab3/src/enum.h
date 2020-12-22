typedef enum BasicType_ { TYPE_INT, TYPE_FLOAT } BasicType;

typedef enum Kind_{BASIC, ARRAY,STRUCTURE, FUNCTION} Kind;

typedef enum ErrorType_{
    UNDEF_VAR = 1,         // Undefined Variable
    UNDEF_FUNC,            // Undefined Function
    REDEF_VAR,             // Redefined Variable
    REDEF_FUNC,            // Redefined Function
    MISMATCH_ASSIGN,       // Mismatched Types Assigned
    EXP_ASSIGN,            // Right-Value Expression Assigned
    MISMATCH_OPRAND,       // Mismatched Types for Operands.
    MISMATCH_RETURN,       // Mismatched Return Type
    MISMATCH_PARAM,        // Mismatched Parameters for Called Function
    NON_ARRAY,             // Calling Non-Array
    NON_FUNC,              // Calling Non-Function
    NON_INT,               // Access Array with Non-Integer
    ILLEGAL_DOT,           // Illegal Dot Used
    UNDEF_FIEED,           // Undefined Field
    REDEF_FEILD,           // Redefined Field
    DUPLIC_STRUCT,         // Duplicate Structure Name
    UNDEF_STRUCT           // Undefined Structure
}ErrorType;