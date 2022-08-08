#define MM_ERROR_FIRST_OF_CLASS(c) ((MM_u32)(c) << 16)

typedef enum MM_ERROR_CLASS
{
    MM_ErrorClass_None = 0,
    MM_ErrorClass_Lexer,
    MM_ErrorClass_TokenParsing,
    MM_ErrorClass_Parser,
} MM_ERROR_CLASS;

typedef enum MM_ERROR_CODE
{
    MM_Error_None = MM_ERROR_FIRST_OF_CLASS(MM_ErrorClass_None),
    MM_Error_UnknownLabel,
} MM_ERROR_CODE;

MM_STATIC_ASSERT(MM_Error_None == 0);

typedef enum MM_LEXER_ERROR_CODE
{
    MM_LexerError_UnterminatedMultiLineComment = MM_ERROR_FIRST_OF_CLASS(MM_ErrorClass_Lexer),
    MM_LexerError_MissingFractionalPart,
    MM_LexerError_MissingExponent,
    MM_LexerError_DigitTooLarge,
    MM_LexerError_DigitCountNoValidHexFloat,
    MM_LexerError_UnterminatedStringLit,
    MM_LexerError_UnterminatedCharLit,
    MM_LexerError_BackslashWithoutEscSeq,
    MM_LexerError_UnknownSymbol,
} MM_LEXER_ERROR_CODE;

typedef enum MM_TOKEN_PARSE_ERROR_CODE
{
    MM_TokenParseError_NoCodepointInCodepointLiteral = MM_ERROR_FIRST_OF_CLASS(MM_ErrorClass_TokenParsing),
    MM_TokenParseError_MissingDigitsInEscSeq,
    MM_TokenParseError_InvalidFirstByteOfUTF8,
    MM_TokenParseError_MissingTrailingBytesUTF8,
    MM_TokenParseError_MoreThanOneCodepoint,
    MM_TokenParseError_CodepointOutOfRange,
    MM_TokenParseError_IntegerLiteralTooLarge,
} MM_TOKEN_PARSE_ERROR_CODE;

typedef enum MM_PARSER_ERROR_CODE
{
    MM_ParserError_MissingColonBetweenParamNamesAndType,
    MM_ParserError_MissingOpenParenAfterBuiltin,
    MM_ParserError_MissingOpenParenAfterIf,
    MM_ParserError_MissingOpenParenAfterWhen,
    MM_ParserError_MissingOpenParenAfterWhile,
    MM_ParserError_MissingCloseParen,
    MM_ParserError_MissingCloseBrace,
    MM_ParserError_MissingCloseBracket,
    MM_ParserError_MissingSemicolon,
    MM_ParserError_StraySemicolon,
    MM_ParserError_MissingColonBetweenRetValNamesAndType,
    MM_ParserError_MissingStructBody,
    MM_ParserError_MissingUnionBody,
    MM_ParserError_MissingEnumBody,
    MM_ParserError_InvalidUseOfKeywordInExpression,
    MM_ParserError_MissingPrimaryExpression,
    MM_ParserError_MissingNameOfMember,
    MM_ParserError_MissingFalseClause,
    MM_ParserError_DuplicateUsing,
    MM_ParserError_DuplicateDistinct,
    MM_ParserError_UsingOnAssignment,
    MM_ParserError_UsingMultipleSymbols,
    MM_ParserError_MissingAliasName,
    MM_ParserError_StrayExpressionList,
    MM_ParserError_ElseWithoutIf,
    MM_ParserError_MissingLabelName,
    MM_ParserError_BlankLabelName,
    MM_ParserError_InvalidUseOfLabel,
    MM_ParserError_BlankJumpLabel,
    MM_ParserError_MissingJumpLabel,
    MM_ParserError_MissingPathAfterInclude,
    
} MM_PARSER_ERROR_CODE;

typedef struct MM_Error
{
    union
    {
        struct
        {
            MM_u16 _;
            MM_u16 error_class;
        };
        
        MM_u32 code;
    };
    
    MM_String text;
} MM_Error;

MM_Error MM_ErrorNone = { .code = MM_Error_None };

#undef MM_ERROR_FIRST_OF_CLASS