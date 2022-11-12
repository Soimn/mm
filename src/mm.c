#include <stdarg.h>
#include <intrin.h>

#include "mm.h"

int wvnsprintfA(char* dest, int size, const char* format, va_list args);
void OutputDebugStringA(const char* str);
void* __stdcall GetStdHandle(unsigned long handle);
int __stdcall WriteConsoleA(void*, const void*, unsigned long, unsigned long, void*);
void* VirtualAlloc(void*, unsigned __int64, unsigned long, unsigned long);
int VirtualFree(void*, unsigned __int64, unsigned long);

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

void*
MM_System_ReserveMemory(MM_umm size)
{
    return VirtualAlloc(0, size, 0x00002000 /*MEM_RESERVE*/, 0x04 /*PAGE_READWRITE*/);
}

void
MM_System_CommitMemory(void* ptr, MM_umm size)
{
    VirtualAlloc(ptr, size, 0x00001000 /*MEM_COMMIT*/, 0x04 /*PAGE_READWRITE*/);
}

void
MM_System_ReleaseMemory(void* ptr)
{
    VirtualFree(ptr, 0, 0x00008000 /*MEM_RELEASE*/);
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
    
    MM_Token token = MM_Lexer_GetToken(&lexer);
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
    for (MM_Token token = MM_Lexer_GetToken(&lexer); token.kind != MM_Token_EOF; token = MM_Lexer_NextToken(&lexer))
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
        MM_Token token = MM_Lexer_GetToken(&lexer);
        
        if (!(token.kind == MM_Token_Int && token.i128.hi == large_integer_expected.hi && token.i128.lo == large_integer_expected.lo))
        {
            Print("TestLexer_NumericLiterals -- FAILED. At large integer bin\n");
            succeeded = MM_false;
        }
    }
    { // dec
        MM_Lexer lexer = MM_Lexer_Init(large_integer_string_dec, (MM_Text_Pos){ .offset = 0, .line = 1, .col = 1 });
        MM_Token token = MM_Lexer_GetToken(&lexer);
        
        if (!(token.kind == MM_Token_Int && token.i128.hi == large_integer_expected.hi && token.i128.lo == large_integer_expected.lo))
        {
            Print("TestLexer_NumericLiterals -- FAILED. At large integer dec\n");
            succeeded = MM_false;
        }
    }
    { // hex
        MM_Lexer lexer = MM_Lexer_Init(large_integer_string_hex, (MM_Text_Pos){ .offset = 0, .line = 1, .col = 1 });
        MM_Token token = MM_Lexer_GetToken(&lexer);
        
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
    Print("TESTING LEXER\n--------------------------------------------\n");
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
    
    Print("%u out of %u succeeded\n\n", succeeded, ran);
}

MM_bool
MatchAST(void* vn0, void* vn1)
{
    MM_AST* n0 = vn0;
    MM_AST* n1 = vn1;
    
    if (n0 == 0 && n1 == 0) return MM_true;
    else if (n0 == 0 || n1 == 0) return MM_false;
    else if (n0->kind != n1->kind) return MM_false;
    else if (((n0->next == 0) ^ (n1->next == 0))) return MM_false;
    else if (n0->next != 0 && !MatchAST(n0->next, n1->next)) return MM_false;
    else
    {
        if (n0->kind >= MM_AST__FirstBinary && n0->kind <= MM_AST__LastBinary)
        {
            return (MatchAST(((MM_Binary_Expression*)n0)->left, ((MM_Binary_Expression*)n1)->left) && MatchAST(((MM_Binary_Expression*)n0)->right, ((MM_Binary_Expression*)n1)->right));
        }
        else if (n0->kind >= MM_AST__FirstPrefixUnary && n0->kind <= MM_AST__LastPrefixUnary || n0->kind >= MM_AST__FirstTypePrefix && n0->kind <= MM_AST__LastTypePrefix)
        {
            if (n0->kind == MM_AST_ArrayType)
            {
                return (MatchAST(((MM_Array_Type_Expression*)n0)->type, ((MM_Array_Type_Expression*)n1)->type) && MatchAST(((MM_Array_Type_Expression*)n0)->size, ((MM_Array_Type_Expression*)n1)->size));
            }
            else return MatchAST(((MM_Unary_Expression*)n0)->operand, ((MM_Unary_Expression*)n1)->operand);
        }
        else if (n0->kind >= MM_AST__FirstPostfixUnary && n0->kind <= MM_AST__LastPostfixUnary)
        {
            switch (n0->kind)
            {
                case MM_AST_Dereference: return MatchAST(((MM_Unary_Expression*)n0)->operand, ((MM_Unary_Expression*)n1)->operand);
                case MM_AST_Subscript:   return (MatchAST(((MM_Subscript_Expression*)n0)->array, ((MM_Subscript_Expression*)n1)->array) && MatchAST(((MM_Subscript_Expression*)n0)->index, ((MM_Subscript_Expression*)n1)->index));
                case MM_AST_Slice:   return (MatchAST(((MM_Slice_Expression*)n0)->array, ((MM_Slice_Expression*)n1)->array) && MatchAST(((MM_Slice_Expression*)n0)->start_index, ((MM_Slice_Expression*)n1)->start_index) && MatchAST(((MM_Slice_Expression*)n0)->past_end_index, ((MM_Slice_Expression*)n1)->past_end_index));
                case MM_AST_Call: return (MatchAST(((MM_Call_Expression*)n0)->proc, ((MM_Call_Expression*)n1)->proc) && MatchAST(((MM_Call_Expression*)n0)->args, ((MM_Call_Expression*)n1)->args));
                case MM_AST_Member: return (MatchAST(((MM_Member_Expression*)n0)->symbol, ((MM_Member_Expression*)n1)->symbol) && MM_String_Match(((MM_Member_Expression*)n0)->member, ((MM_Member_Expression*)n1)->member));
                case MM_AST_StructLit: return (MatchAST(((MM_Struct_Lit_Expression*)n0)->type, ((MM_Struct_Lit_Expression*)n1)->type) && MatchAST(((MM_Struct_Lit_Expression*)n0)->args, ((MM_Struct_Lit_Expression*)n1)->args));
                default:
                {
                    MM_ASSERT(n0->kind == MM_AST_ArrayLit);
                    return (MatchAST(((MM_Array_Lit_Expression*)n0)->type, ((MM_Array_Lit_Expression*)n1)->type) && MatchAST(((MM_Array_Lit_Expression*)n0)->args, ((MM_Array_Lit_Expression*)n1)->args));
                } break;
            }
        }
        else switch (n0->kind)
        {
            case MM_AST_Argument: return (MatchAST(((MM_Argument*)n0)->name, ((MM_Argument*)n1)->name) && MatchAST(((MM_Argument*)n0)->value, ((MM_Argument*)n1)->value));
            case MM_AST_Parameter: return (MatchAST(((MM_Parameter*)n0)->names, ((MM_Parameter*)n1)->names) && MatchAST(((MM_Parameter*)n0)->type, ((MM_Parameter*)n1)->type) && MatchAST(((MM_Parameter*)n0)->value, ((MM_Parameter*)n1)->value));
            case MM_AST_ReturnValue: return (MatchAST(((MM_Return_Value*)n0)->names, ((MM_Return_Value*)n1)->names) && MatchAST(((MM_Return_Value*)n0)->type, ((MM_Return_Value*)n1)->type) && MatchAST(((MM_Return_Value*)n0)->value, ((MM_Return_Value*)n1)->value));
            case MM_AST_Identifier: return MM_String_Match(((MM_Expression*)n0)->identifier_expr.value, ((MM_Expression*)n1)->identifier_expr.value);
            case MM_AST_Int: return (((MM_Expression*)n0)->int_expr.value.lo == ((MM_Expression*)n1)->int_expr.value.lo && ((MM_Expression*)n0)->int_expr.value.hi == ((MM_Expression*)n1)->int_expr.value.hi);
            case MM_AST_Float: return (((MM_Expression*)n0)->float_expr.value == ((MM_Expression*)n1)->float_expr.value);
            case MM_AST_String: return MM_String_Match(((MM_Expression*)n0)->string_expr.value, ((MM_Expression*)n1)->string_expr.value);
            case MM_AST_Bool: return (((MM_Expression*)n0)->bool_expr.value == ((MM_Expression*)n1)->bool_expr.value);
            
            case MM_AST_ProcType: return (MatchAST(((MM_Proc_Type_Expression*)n0)->params, ((MM_Proc_Type_Expression*)n1)->params) && MatchAST(((MM_Proc_Type_Expression*)n0)->return_vals, ((MM_Proc_Type_Expression*)n1)->return_vals));
            case MM_AST_ProcLit: return (MatchAST(((MM_Proc_Lit_Expression*)n0)->params, ((MM_Proc_Type_Expression*)n1)->params) && MatchAST(((MM_Proc_Lit_Expression*)n0)->return_vals, ((MM_Proc_Lit_Expression*)n1)->return_vals));
            case MM_AST_StructType: return MatchAST(((MM_Struct_Type_Expression*)n0)->members, ((MM_Struct_Type_Expression*)n1)->members);
            case MM_AST_Compound: return MatchAST(((MM_Compound_Expression*)n0)->expr, ((MM_Compound_Expression*)n1)->expr);
            case MM_AST_BuiltinCall: return (((MM_Builtin_Call_Expression*)n0)->builtin_kind == ((MM_Builtin_Call_Expression*)n1)->builtin_kind && MatchAST(((MM_Builtin_Call_Expression*)n0)->args, ((MM_Builtin_Call_Expression*)n1)->args));
            
            case MM_AST_Var:
            {
                MM_Variable_Declaration* v0 = (MM_Variable_Declaration*)n0;
                MM_Variable_Declaration* v1 = (MM_Variable_Declaration*)n1;
                
                return (MatchAST(v0->names, v1->names) && MatchAST(v0->type, v1->type) && MatchAST(v0->values, v1->values) && v0->is_uninitialized == v1->is_uninitialized);
            }
            
            case MM_AST_Const:
            {
                MM_Constant_Declaration* v0 = (MM_Constant_Declaration*)n0;
                MM_Constant_Declaration* v1 = (MM_Constant_Declaration*)n1;
                
                return (MatchAST(v0->names, v1->names) && MatchAST(v0->type, v1->type) && MatchAST(v0->values, v1->values));
            }
            
            case MM_AST_Block:
            {
                MM_Block_Statement* b0 = (MM_Block_Statement*)n0;
                MM_Block_Statement* b1 = (MM_Block_Statement*)n1;
                
                return (MM_String_Match(b0->label, b1->label) && MatchAST(b0->body, b1->body));
            }
            
            case MM_AST_If:
            {
                MM_If_Statement* i0 = (MM_If_Statement*)n0;
                MM_If_Statement* i1 = (MM_If_Statement*)n1;
                
                return (MM_String_Match(i0->label, i1->label) && MatchAST(i0->init, i1->init) && MatchAST(i0->condition, i1->condition) && MatchAST(i0->true_body, i1->true_body) && MatchAST(i0->false_body, i1->false_body));
            }
            
            case MM_AST_When:
            {
                MM_When_Declaration* i0 = (MM_When_Declaration*)n0;
                MM_When_Declaration* i1 = (MM_When_Declaration*)n1;
                
                return (MatchAST(i0->condition, i1->condition) && MatchAST(i0->true_body, i1->true_body) && MatchAST(i0->false_body, i1->false_body));
            }
            
            case MM_AST_While:
            {
                MM_While_Statement* i0 = (MM_While_Statement*)n0;
                MM_While_Statement* i1 = (MM_While_Statement*)n1;
                
                return (MM_String_Match(i0->label, i1->label) && MatchAST(i0->init, i1->init) && MatchAST(i0->condition, i1->condition) && MatchAST(i0->step, i1->step) && MatchAST(i0->body, i1->body));
            }
            
            case MM_AST_Continue: case MM_AST_Break: return MM_String_Match(((MM_Jump_Statement*)n0)->label, ((MM_Jump_Statement*)n1)->label);
            
            case MM_AST_Return: return MatchAST(((MM_Return_Statement*)n0)->args, ((MM_Return_Statement*)n1)->args);
            case MM_AST_Assignment:
            {
                MM_Assignment_Statement* a0 = (MM_Assignment_Statement*)n0;
                MM_Assignment_Statement* a1 = (MM_Assignment_Statement*)n1;
                
                return (a0->assign_op == a1->assign_op && MatchAST(a0->lhs, a1->lhs) && MatchAST(a0->rhs, a1->rhs));
            }
            
            default: MM_ASSERT(MM_false); return MM_false;
        }
    }
}

MM_bool
TestParser_BasicPrecedence(MM_Arena* arena)
{
    MM_String string = MM_STRING("1 + 2 * add(5, name: 6) / 5 == 3 << (4 + -2)");
    
    MM_AST nodes[] = {
        [0]  = { .expression.binary_expr     = { .kind = MM_AST_CmpEqual,   .left    = (MM_Expression*)&nodes[1],  .right = (MM_Expression*)&nodes[14]                                        } },
        [1]  = { .expression.binary_expr     = { .kind = MM_AST_Add,        .left    = (MM_Expression*)&nodes[2],  .right = (MM_Expression*)&nodes[3]                                         } },
        [2]  = { .expression.int_expr        = { .kind = MM_AST_Int,        .value   = MM_i128_FromU64(1)                                                                                     } },
        [3]  = { .expression.binary_expr     = { .kind = MM_AST_Div,        .left    = (MM_Expression*)&nodes[4],  .right = (MM_Expression*)&nodes[13]                                        } },
        [4]  = { .expression.binary_expr     = { .kind = MM_AST_Mul,        .left    = (MM_Expression*)&nodes[5],  .right = (MM_Expression*)&nodes[6]                                         } },
        [5]  = { .expression.int_expr        = { .kind = MM_AST_Int,        .value   = MM_i128_FromU64(2)                                                                                     } },
        [6]  = { .expression.call_expr       = { .kind = MM_AST_Call,       .proc    = (MM_Expression*)&nodes[7],  .args  = (MM_Argument*)&nodes[8]                                           } },
        [7]  = { .expression.identifier_expr = { .kind = MM_AST_Identifier, .value   = MM_STRING("add")                                                                                       } },
        [8]  = { .argument                   = { .kind = MM_AST_Argument,   .next    = (MM_Argument*)&nodes[10],   .name = 0, .value = (MM_Expression*)&nodes[9]                              } },
        [9]  = { .expression.int_expr        = { .kind = MM_AST_Int,        .value   = MM_i128_FromU64(5)                                                                                     } },
        [10] = { .argument                   = { .kind = MM_AST_Argument,   .next    = 0,                          .name  = (MM_Expression*)&nodes[11], .value = (MM_Expression*)&nodes[12]   } },
        [11] = { .expression.identifier_expr = { .kind = MM_AST_Identifier, .value   = MM_STRING("name")                                                                                      } },
        [12] = { .expression.int_expr        = { .kind = MM_AST_Int,        .value   = MM_i128_FromU64(6)                                                                                     } },
        [13] = { .expression.int_expr        = { .kind = MM_AST_Int,        .value   = MM_i128_FromU64(5)                                                                                     } },
        [14] = { .expression.binary_expr     = { .kind = MM_AST_BitShl,     .left    = (MM_Expression*)&nodes[15], .right = (MM_Expression*)&nodes[16]                                        } },
        [15] = { .expression.int_expr        = { .kind = MM_AST_Int,        .value   = MM_i128_FromU64(3)                                                                                     } },
        [16] = { .expression.compound_expr   = { .kind = MM_AST_Compound,   .expr    = (MM_Expression*)&nodes[17]                                                                             } },
        [17] = { .expression.binary_expr     = { .kind = MM_AST_Add,        .left    = (MM_Expression*)&nodes[18], .right = (MM_Expression*)&nodes[19]                                        } },
        [18] = { .expression.int_expr        = { .kind = MM_AST_Int,        .value   = MM_i128_FromU64(4)                                                                                     } },
        [19] = { .expression.unary_expr      = { .kind = MM_AST_Neg,        .operand = (MM_Expression*)&nodes[20]                                                                             } },
        [20] = { .expression.int_expr        = { .kind = MM_AST_Int,        .value   = MM_i128_FromU64(2)                                                                                     } },
    };
    
    MM_Expression* expected = (MM_Expression*)&nodes[0];
    
    MM_Parser parser = {
        .lexer     = MM_Lexer_Init(string, (MM_Text_Pos){ .offset = 0, .line = 1, .col = 1 }),
        .ast_arena = arena,
    };
    
    MM_Expression* got;
    if (!MM_Parser__ParseExpression(&parser, &got))
    {
        Print("TestParser_BasicPrecedence -- FAILED. Failed to parse\n");
        return MM_false;
    }
    else if (!MatchAST(got, expected))
    {
        Print("TestParser_BasicPrecedence -- FAILED. Parse trees don't match\n");
        return MM_false;
    }
    else
    {
        Print("TestParser_BasicPrecedence -- SUCCEEDED.\n");
        return MM_true;
    }
}

void
TestParser()
{
    Print("TESTING PARSER\n--------------------------------------------\n");
    
    MM_Arena* arena = MM_Arena_Create(4ULL*1024*1024*1024);
    
    MM_umm ran       = 0;
    MM_umm succeeded = 0;
    ++ran, succeeded += (MM_umm)TestParser_BasicPrecedence(arena);
    
    Print("%u out of %u succeeded\n\n", succeeded, ran);
}

int
mainCRTStartup()
{
    TestLexer(MM_false);
    TestParser();
    
    return 0;
}