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

typedef enum OperandKind_{
    OP_VARIABLE,
    OP_CONSTANT,
    OP_ADDRESS,
    OP_LABEL,
    OP_FUNCTION,
}OperandKind;

typedef enum CodeKind_{
    IC_LABEL, // LABEL x :
    IC_FUNCTION, // FUNCTION f :
    IC_ASSIGN, // x := y
    IC_ADD, // x := y + z
    IC_SUB, // x := y - z
    IC_MUL, // x := y * z
    IC_DIV, // x := y / z
    IC_GET_ADDR, // x := &y
    IC_READ_ADDR, // x := *y
    IC_WRITE_ADDR, // *x = y
    IC_GOTO, // GOTO x 10
    IC_IF_GOTO, // IF x [relop] y GOTO z
    IC_RETURN, // RETURN x
    IC_DEC, // DEC x [size]
    IC_ARG, // ARG x
    IC_CALL, // x := CALL f
    IC_PARAM, // PARAM x
    IC_READ, // READ x
    IC_WRITE,  // WRITE x
}CodeKind;