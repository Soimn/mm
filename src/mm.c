#include <stdarg.h>
#include <intrin.h>

#include "mm.h"
#include "mm_int.h"
#include "mm_string.h"
#include "mm_tokens.h"
#include "mm_lexer.h"

int wvnsprintfA(char* dest, int size, const char* format, va_list args);
void OutputDebugStringA(const char* str);
void* __stdcall GetStdHandle(unsigned long handle);
int __stdcall WriteConsoleA(void*, const void*, unsigned long, unsigned long, void*);

int
Sprint(char* buffer, MM_u32 size, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    int result = wvnsprintfA(buffer, size, msg, args);
    va_end(args);
    
    return result;
}

void
Print(const char* msg, ...)
{
    char buffer[1024];
    
    va_list args;
    va_start(args, msg);
    wvnsprintfA(buffer, sizeof(buffer), msg, args);
    va_end(args);
    
    OutputDebugStringA(buffer);
    WriteConsoleA(GetStdHandle(((unsigned long)-11)), buffer, (unsigned long)MM_Cstring_Length(buffer), 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPORTANT NOTE: THESE ARE NOT TESTS IN THE FORMAL SENSE, THEY ARE "TEST IF THIS WORKS", which is
//                 kind of everything a test is when you remove the important methodology around it.
//                 Additionally, the code is shit, it will definitely be replaced with something
//                 more proper when there is actually something interesting to test

MM_bool
TestLexer_TokenKinds(MM_String string, MM_Token_Kind* expected_kinds, MM_umm expected_kind_count)
{
    MM_Lexer lexer = MM_Lexer_Init(string, (MM_Text_Pos){ .offset = 0, .line = 1, .col = 1 });
    
    Print("TestLexer_TokenKinds -- ");
    
    MM_Token token = MM_Lexer_NextToken(&lexer);
    MM_umm i       = 0;
    for (; i < expected_kind_count && token.kind != MM_Token_EOF; token = MM_Lexer_NextToken(&lexer), ++i)
    {
        if (token.kind != expected_kinds[i])
        {
            MM_String expected = MM_Token_NameList[expected_kinds[i]];
            MM_String got      = MM_Token_NameList[token.kind];
            
            Print("FAILED. (%u,%u): expected %.*s got %.*s\n", token.line, token.col, expected.size, expected.data, got.size, got.data);
            return MM_false;
        }
    }
    
    if (i < expected_kind_count)
    {
        MM_String expected = MM_Token_NameList[expected_kinds[i]];
        Print("FAILED. (%u,%u): expected %.*s got MM_Token_EOF\n", token.line, token.col, expected.size, expected.data);
        return MM_false;
    }
    else if (token.kind != MM_Token_EOF)
    {
        MM_String got = MM_Token_NameList[token.kind];
        Print("FAILED. (%u,%u): expected MM_Token_EOF got %.*s\n", token.line, token.col, got.size, got.data);
        return MM_false;
    }
    else
    {
        Print("SUCCEEDED.\n");
        return MM_true;
    }
}

MM_bool
TestLexer_Reconstruction(MM_String string, MM_bool print)
{
    MM_Lexer lexer = MM_Lexer_Init(string, (MM_Text_Pos){ .offset = 0, .line = 1, .col = 1 });
    
    char buffer[4096];
    int buffer_space = MM_ARRAY_SIZE(buffer);
    
    MM_Text_Pos prev = { .offset = 0, .line = 1, .col = 1 };
    for (MM_Token token = MM_Lexer_NextToken(&lexer); token.kind != MM_Token_EOF; token = MM_Lexer_NextToken(&lexer))
    {
        MM_u8 token_string_buffer[42];
        MM_String token_string = MM_STRING("(null)");
        
        if (token.kind == MM_Token_Identifier                                           ||
            token.kind >= MM_Token__FirstKeyword && token.kind <= MM_Token__LastKeyword ||
            token.kind >= MM_Token__FirstBuiltin && token.kind <= MM_Token__LastBuiltin)
        {
            token_string = token.string;
        }
        else if (token.kind == MM_Token_String) MM_NOT_IMPLEMENTED;
        else if (token.kind == MM_Token_Float)  MM_NOT_IMPLEMENTED;
        else if (token.kind == MM_Token_Int)
        {
            if (token.i128.hi != 0) MM_NOT_IMPLEMENTED;
            else
            {
                token_string.data = token_string_buffer;
                token_string.size = Sprint((char*)token_string_buffer, MM_ARRAY_SIZE(token_string_buffer), "%llu", token.i128.lo);
                MM_ASSERT(token_string.size > 0 && token_string.size < MM_ARRAY_SIZE(token_string_buffer));
            }
        }
        else token_string = MM_Token_SymbolStringList[token.kind];
        
        for (MM_umm i = 0; i < token.line - prev.line; ++i)
        {
            int result = Sprint(buffer + (MM_ARRAY_SIZE(buffer) - buffer_space), buffer_space, "\n");
            MM_ASSERT(result == 1);
            buffer_space -= result;
        }
        
        int result = Sprint(buffer + (MM_ARRAY_SIZE(buffer) - buffer_space), buffer_space,
                            "%*s%.*s", (token.line != prev.line ? token.col - 1 : token.col - prev.col), "", token_string.size, token_string.data);
        
        MM_ASSERT(result > 0 && result < buffer_space);
        buffer_space -= result;
        
        prev      = token.text_pos;
        prev.col += token.size;
    }
    
    if (print) Print("%s\n\n", buffer);
    
    MM_bool succeeded = MM_String_Cstring_Match(string, buffer);
    
    Print("TestLexer_Reconstruction -- %s.\n", (succeeded ? "SUCCEEDED" : "FAILED"));
    
    return succeeded;
}

MM_bool
TestLexer_NumericLiterals(MM_bool print)
{
    MM_String large_integer_string_bin = MM_STRING("0b_10001011100011010000001100001101001111100000100011000111__101_001011111000100111_011100111110100_0011001101111001100011111100____");
    MM_String large_integer_string_dec = MM_STRING("0000__00000000_07245893248_______7324345897__34589013___00123900_______");
    MM_String large_integer_string_hex = MM_STRING("0x__8b8d0___30d3e08__c7_a5f13b9f4__33798fc_______________________");
    MM_i128   large_integer_expected   = (MM_i128){ .lo = 0xa5f13b9f433798fc, .hi = 0x8b8d030d3e08c7 };
    
    MM_bool succeeded = MM_true;
    { // bin
        MM_Lexer lexer = MM_Lexer_Init(large_integer_string_bin, (MM_Text_Pos){ .offset = 0, .line = 1, .col = 1 });
        MM_Token token = MM_Lexer_NextToken(&lexer);
        
        if (!(token.kind == MM_Token_Int && token.i128.hi == large_integer_expected.hi && token.i128.lo == large_integer_expected.lo))
        {
            Print("TestLexer_NumericLiterals -- FAILED. At large integer bin\n");
            succeeded = MM_false;
        }
    }
    { // dec
        MM_Lexer lexer = MM_Lexer_Init(large_integer_string_dec, (MM_Text_Pos){ .offset = 0, .line = 1, .col = 1 });
        MM_Token token = MM_Lexer_NextToken(&lexer);
        
        if (!(token.kind == MM_Token_Int && token.i128.hi == large_integer_expected.hi && token.i128.lo == large_integer_expected.lo))
        {
            Print("TestLexer_NumericLiterals -- FAILED. At large integer dec\n");
            succeeded = MM_false;
        }
    }
    { // hex
        MM_Lexer lexer = MM_Lexer_Init(large_integer_string_hex, (MM_Text_Pos){ .offset = 0, .line = 1, .col = 1 });
        MM_Token token = MM_Lexer_NextToken(&lexer);
        
        if (!(token.kind == MM_Token_Int && token.i128.hi == large_integer_expected.hi && token.i128.lo == large_integer_expected.lo))
        {
            Print("TestLexer_NumericLiterals -- FAILED. At large integer hex\n");
            succeeded = MM_false;
        }
    }
    
    if (succeeded) Print("TestLexer_NumericLiterals -- SUCCEEDED.\n");
    
    return succeeded;
}

void
TestLexer(MM_bool print)
{
    MM_String string = MM_STRING("factorial :: proc(n: int) -> int\n{\n    if (n <= 2) return n;\n    else return n*factorial(n-1);\n}\n\nmain :: proc\n{\n    x: int = factorial(5);\n}");
    MM_Token_Kind expected_kinds[] = {
        MM_Token_Identifier, MM_Token_Colon, MM_Token_Colon, MM_Token_Proc, MM_Token_OpenParen, MM_Token_Identifier, MM_Token_Colon, MM_Token_Identifier,
        MM_Token_CloseParen, MM_Token_Arrow, MM_Token_Identifier, MM_Token_OpenBrace, MM_Token_If, MM_Token_OpenParen, MM_Token_Identifier, MM_Token_LessEQ, MM_Token_Int,
        MM_Token_CloseParen, MM_Token_Return, MM_Token_Identifier, MM_Token_Semicolon, MM_Token_Else, MM_Token_Return, MM_Token_Identifier, MM_Token_Star,
        MM_Token_Identifier, MM_Token_OpenParen, MM_Token_Identifier, MM_Token_Minus, MM_Token_Int, MM_Token_CloseParen, MM_Token_Semicolon, MM_Token_CloseBrace,
        MM_Token_Identifier, MM_Token_Colon, MM_Token_Colon, MM_Token_Proc, MM_Token_OpenBrace, MM_Token_Identifier, MM_Token_Colon, MM_Token_Identifier, MM_Token_Equals,
        MM_Token_Identifier, MM_Token_OpenParen, MM_Token_Int, MM_Token_CloseParen, MM_Token_Semicolon, MM_Token_CloseBrace
    };
    
    MM_umm ran       = 0;
    MM_umm succeeded = 0;
    ++ran, succeeded += (MM_umm)TestLexer_TokenKinds(string, expected_kinds, MM_ARRAY_SIZE(expected_kinds));
    ++ran, succeeded += (MM_umm)TestLexer_Reconstruction(string, print);
    ++ran, succeeded += (MM_umm)TestLexer_NumericLiterals(print);
    
    Print("%u out of %u succeeded\n", succeeded, ran);
}

int
mainCRTStartup()
{
    TestLexer(MM_true);
    
    return 0;
}