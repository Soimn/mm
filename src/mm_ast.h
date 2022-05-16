typedef enum AST_NODE_KIND
{
    AST_Nil = 0,
    
    AST_NamedValue,
    AST_Parameter,
    AST_EnumMember,
    
    AST_FirstExpression,
    AST_Identifier = AST_FirstExpression,
    AST_String,
    AST_Char,
    AST_Int,
    AST_Float,
    AST_Bool,
    AST_Proc,
    AST_ProcLiteral,
    AST_Struct,
    AST_Union,
    AST_Enum,
    AST_Compound,
    AST_Selector,
    AST_PolymorphicName,
    AST_IntrinsicCall,
    
    AST_FirstPostfix,
    AST_Dereference = AST_FirstPostfix,
    AST_Member,
    AST_Subscript,
    AST_Slice,
    AST_Call,
    AST_StructLiteral,
    AST_ArrayLiteral,
    AST_LastPostfix = AST_Call,
    
    AST_FirstPrefix,
    AST_Reference = AST_FirstPrefix,
    AST_ArrayType,
    AST_SliceType,
    AST_Neg,
    AST_BitNot,
    AST_Not,
    AST_LastPrefix = AST_Not,
    
    AST_FirstBinary = 8*16,
    AST_FirstMulLevel = AST_FirstBinary,
    AST_Mul = AST_FirstMulLevel,
    AST_Div,
    AST_Rem,
    AST_BitAnd,
    AST_BitShl,
    AST_BitShr,
    AST_BitSar,
    AST_LastMulLevel = AST_BitSar,
    
    AST_FirstAddLevel = 9*16,
    AST_BitOr = AST_FirstAddLevel,
    AST_BitXor,
    AST_Sub,
    AST_Add,
    AST_LastAddLevel = AST_Add,
    
    AST_FirstCmpLevel = 10*16,
    AST_IsStrictlyGreater = AST_FirstCmpLevel,
    AST_IsGreater,
    AST_IsStrictlyLess,
    AST_IsLess,
    AST_IsEquals,
    AST_IsNotEquals,
    AST_LastCmpLevel = AST_IsNotEquals,
    
    AST_And = 11*16,
    
    AST_Or = 12*16,
    AST_LastBinary = AST_Or,
    
    AST_Conditional = 13*16,
    AST_LastExpression = AST_Conditional,
    
    AST_FirstStatement,
    AST_Block,
    AST_If,
    AST_While,
    AST_Break,
    AST_Continue,
    AST_Defer,
    AST_Return,
    AST_Assignment,
    AST_LastStatement = AST_Assignment,
    
    AST_FirstDeclaration,
    AST_Variable = AST_FirstDeclaration,
    AST_Constant,
    AST_Using,
    AST_LastDeclaration = AST_Using,
} AST_NODE_KIND;

// NOTE: Every member that can legally be 0 is marked with the value means when it is
typedef struct AST_Node
{
    AST_NODE_KIND kind;
    
    struct AST_Node* next; // NOTE: 0 means no next
    File_ID file_id;
    u32 offset;
    u32 size;
    struct AST_Node_Info* info;
    
    union
    {
        struct
        {
            struct AST_Node* name;
            struct AST_Node* value;
        } named_value;
        
        struct
        {
            struct AST_Node* name;  // NOTE: 0 means no name
            struct AST_Node* type;  // NOTE: 0 means infered type
            struct AST_Node* value; // NOTE: 0 means no value
            bool is_using;
        } parameter;
        
        struct
        {
            struct AST_Node* name;
            struct AST_Node* value; // NOTE: 0 means no value
        } enum_member;
        
        Interned_String string;  // NOTE: 0 means ""
        u32 character;           // NOTE: 0 means '\x00'
        bool boolean;            // NOTE: 0 means false
        
        struct
        {
            Big_Int big_int;     // NOTE: 0 means 0
            u8 base;             // NOTE: 0 means implicit base 10, else explicit base (e.g. 0x0 or 0b0)
        } integer;
        
        struct
        {
            Big_Float big_float; // NOTE: 0 means 0
            u8 byte_size;        // NOTE: 0 means normal float, else it is the size of the hex float in bytes
        } floating;
        
        struct Proc_Header
        {
            struct AST_Node* params;       // NOTE: 0 means no parameters
            struct AST_Node* return_types; // NOTE: 0 means no return values
            struct AST_Node* where_clause; // NOTE: 0 means no clause
        } proc_type;
        
        struct
        {
            struct Proc_Header;
            struct AST_Node* body; // NOTE: 0 means declaration, will otherwise point to a block_statement
        } proc_literal;
        
        struct
        {
            struct AST_Node* params; // NOTE: 0 means no parameters
            struct AST_Node* body;   // NOTE: 0 means empty body
        } struct_type;
        
        struct
        {
            struct AST_Node* params; // NOTE: 0 means no parameters
            struct AST_Node* body;   // NOTE: 0 means empty body
        } union_type;
        
        struct
        {
            struct AST_Node* type; // NOTE: 0 means default enum backing type
            struct AST_Node* body; // NOTE: 0 means empty body
        } enum_type;
        
        struct AST_Node* compound_expr;
        
        Interned_String selector_expr;
        
        Interned_String poly_name;
        
        struct
        {
            Interned_String proc;
            struct AST_Node* args; // NOTE: 0 means no arguments
        } intrinsic_call_expr;
        
        struct
        {
            struct AST_Node* expr;
            Interned_String member;
        } member_expr;
        
        struct
        {
            struct AST_Node* array;
            struct AST_Node* index;
        } subscript_expr;
        
        struct
        {
            struct AST_Node* array;
            struct AST_Node* start;    // NOTE: 0 means the start of the array
            struct AST_Node* past_end; // NOTE: 0 means one past the the of the array
        } slice_expr;
        
        struct
        {
            struct AST_Node* proc;
            struct AST_Node* args; // NOTE: 0 means no arguments
        } call_expr;
        
        struct
        {
            struct AST_Node* size;
            struct AST_Node* elem_type;
        } array_type;
        
        struct
        {
            struct AST_Node* elem_type;
        } slice_type;
        
        struct
        {
            struct AST_Node* type; // NOTE: 0 means infered type
            struct AST_Node* args; // NOTE: 0 means all default values
        } struct_literal;
        
        struct
        {
            struct AST_Node* type; // NOTE: 0 means infered type
            struct AST_Node* args; // NOTE: 0 means all default values
        } array_literal;
        
        struct AST_Node* unary_expr;
        
        struct
        {
            struct AST_Node* left;
            struct AST_Node* right;
        } binary_expr;
        
        struct
        {
            struct AST_Node* condition;
            struct AST_Node* true_expr;
            struct AST_Node* false_expr;
        } conditional_expr;
        
        struct
        {
            struct AST_Node* body; // NOTE: 0 means empty block
            Interned_String label; // NOTE: 0 means no label
        } block_statement;
        
        struct
        {
            struct AST_Node* init;       // NOTE: 0 means no init, can be either an expression with side effects, an assignment or a declaration
            struct AST_Node* condition;  // NOTE: can only be an expression with a boolean type
            struct AST_Node* true_body;  // NOTE: 0 means no body
            struct AST_Node* false_body; // NOTE: 0 means no body
            Interned_String label;       // NOTE: 0 means no label
        } if_statement;
        
        struct
        {
            struct AST_Node* init;      // NOTE: 0 means no init, can be either an expression with side effects, an assignment or a declaration
            struct AST_Node* condition; // NOTE: can only be an expression with a boolean type
            struct AST_Node* step;      // NOTE: 0 means no step
            struct AST_Node* body;      // NOTE: 0 means no body
            Interned_String label;      // NOTE: 0 means no label
        } while_statement;
        
        Interned_String jump_label;    // NOTE: 0 means no label
        
        struct AST_Node* defer_statement; // NOTE: can only be a statement, or an expression with side effects
        
        struct AST_Node* return_values; // NOTE: 0 means no return values
        
        struct
        {
            struct AST_Node* lhs;
            struct AST_Node* rhs;
            AST_NODE_KIND op; // NOTE: 0 means regular assignment and not e.g. +=
        } assignment_statement;
        
        struct
        {
            struct AST_Node* names;
            struct AST_Node* type;   // NOTE: 0 means infered type
            struct AST_Node* values; // NOTE: 0 means default initialized
            bool is_using;
            bool is_uninitialized;
        } variable_decl;
        
        struct
        {
            struct AST_Node* names;
            struct AST_Node* type;  // NOTE: 0 means infered type
            struct AST_Node* values;
            bool is_using;
        } constant_decl;
        
        struct
        {
            struct AST_Node* symbol_paths;
            Interned_String alias; // NOTE: 0 means no alias
        } using_decl;
    };
} AST_Node;

typedef struct Whitespace_Info
{
    File_ID file_id;
    u32 offset;
    u32 size;
} Whitespace_Info;

typedef union AST_Whitespace_Info
{
    Whitespace_Info ws[10];
    
    struct
    {
        Whitespace_Info ws_before;
        
        union
        {
            struct
            {
                Whitespace_Info ws_equals;
            } named_value_info;
            
            struct
            {
                Whitespace_Info ws_colon;
                Whitespace_Info ws_equals;
            } parameter_info;
            struct
            {
                Whitespace_Info ws_equals;
            } enum_member_info;
            
            struct
            {
                Whitespace_Info ws_open_paren;
                Whitespace_Info ws_close_paren;
                Whitespace_Info ws_arrow;
                Whitespace_Info ws_open_paren_ret;
                Whitespace_Info ws_close_paren_ret;
                Whitespace_Info ws_where;
                Whitespace_Info ws_uninit;
            } proc_info;
            
            struct
            {
                Whitespace_Info ws_open_paren;
                Whitespace_Info ws_close_paren;
                Whitespace_Info ws_open_brace;
                Whitespace_Info ws_close_brace;
            } struct_info;
            
            struct
            {
                Whitespace_Info ws_open_brace;
                Whitespace_Info ws_close_brace;
            } enum_info;
            
            struct
            {
                Whitespace_Info ws_close_paren;
            } compound_info;
            
            struct
            {
                Whitespace_Info ws_name;
            } selector_info;
            
            struct
            {
                Whitespace_Info ws_name;
            } poly_name_info;
            
            struct
            {
                Whitespace_Info ws_hat;
            } deref_info;
            
            struct
            {
                Whitespace_Info ws_period;
                Whitespace_Info ws_member;
            } member_info;
            
            struct
            {
                Whitespace_Info ws_open_bracket;
                Whitespace_Info ws_close_bracket;
            } subscript_info;
            
            struct
            {
                Whitespace_Info ws_open_bracket;
                Whitespace_Info ws_colon;
                Whitespace_Info ws_close_bracket;
            } slice_info;
            
            struct
            {
                Whitespace_Info ws_open_paren;
                Whitespace_Info ws_close_paren;
            } call_info;
            
            struct
            {
                Whitespace_Info ws_close_bracket;
            } array_type_info;
            
            struct
            {
                Whitespace_Info ws_close_bracket;
            } slice_type_info;
            
            struct
            {
                Whitespace_Info ws_period_brace;
                Whitespace_Info ws_close_brace;
            } struct_literal_info;
            
            struct
            {
                Whitespace_Info ws_period_bracket;
                Whitespace_Info ws_close_bracket;
            } array_literal_info;
            
            struct
            {
                Whitespace_Info ws_op;
            } binary_info;
            
            struct
            {
                Whitespace_Info ws_qmark;
                Whitespace_Info ws_colon;
            } conditional_info;
            
            struct
            {
                Whitespace_Info ws_colon;
                Whitespace_Info ws_open_brace;
                Whitespace_Info ws_close_brace;
            } block_info;
            
            struct
            {
                Whitespace_Info ws_colon;
                Whitespace_Info ws_if;
                Whitespace_Info ws_open_paren;
                Whitespace_Info ws_init_semi;
                Whitespace_Info ws_close_paren;
                Whitespace_Info ws_if_body_semi;
                Whitespace_Info ws_else;
                Whitespace_Info ws_else_body_semi;
            } if_info;
            
            struct
            {
                Whitespace_Info ws_colon;
                Whitespace_Info ws_while;
                Whitespace_Info ws_open_paren;
                Whitespace_Info ws_init_semi;
                Whitespace_Info ws_cond_semi;
                Whitespace_Info ws_close_paren;
                Whitespace_Info ws_body_semi;
            } while_info;
            
            struct
            {
                Whitespace_Info ws_label;
            } jump_info;
            
            struct
            {
                Whitespace_Info ws_op;
            } assignment_info;
            
            struct
            {
                Whitespace_Info ws_colon;
                Whitespace_Info ws_equals;
                Whitespace_Info ws_uninit;
            } variable_info;
            
            struct
            {
                Whitespace_Info ws_colon_0;
                Whitespace_Info ws_colon_1;
            } constant_info;
            
            struct
            {
                Whitespace_Info ws_as;
                Whitespace_Info ws_alias;
            } using_info;
        };
        
        Whitespace_Info ws_terminator;
    };
} AST_Whitespace_Info;

typedef struct AST_Node_Info
{
    u32 line;
    u32 col;
    
    union AST_Whitespace_Info;
} AST_Node_Info;

internal Whitespace_Info
WhitespaceInfo_FromToken(File_ID file_id, Token token)
{
    return (Whitespace_Info){ .offset = token.offset_raw, .size = token.text.offset - token.offset_raw, .file_id = file_id, };
}

internal String
WhitespaceInfo_ToString(File* file, Whitespace_Info ws)
{
    return (String){ .data = file->content.data + ws.offset, .size = ws.size };
}

internal bool
AST_StatementNeedsSemicolon(AST_Node* node)
{
    bool needs_semicolon = true;
    
    if (node->kind == AST_Block || node->kind == AST_If || node->kind == AST_While ||
        node->kind == AST_Defer && node->defer_statement->kind == AST_Block        ||
        node->kind == AST_Constant && node->constant_decl.values->next == 0 && (node->constant_decl.values->kind == AST_ProcLiteral && node->constant_decl.values->proc_literal.body != 0 ||
                                                                                node->constant_decl.values->kind == AST_Struct || node->constant_decl.values->kind == AST_Union           ||
                                                                                node->constant_decl.values->kind == AST_Enum))
    {
        needs_semicolon = false;
    }
    
    return needs_semicolon;
}

internal void
DEBUG_AST_Print(Workspace* workspace, Arena* arena, AST_Node* node)
{
    File* file          = File_FromFileID(workspace, node->file_id);
    AST_Node_Info* info = node->info;
    
    String_Printf(arena, "%S", WhitespaceInfo_ToString(file, info->ws_before));
    
    if (node->kind >= AST_FirstStatement && node->kind <= AST_LastStatement)
    {
        if (node->kind == AST_Block)
        {
            if (node->block_statement.label != INTERNED_STRING_NIL)
            {
                String_Printf(arena, "%S%S:%S", InternedString_ToString(workspace, node->block_statement.label), WhitespaceInfo_ToString(file, info->block_info.ws_colon),
                              WhitespaceInfo_ToString(file, info->block_info.ws_open_brace));
            }
            
            String_Printf(arena, "{");
            
            for (AST_Node* scan = node->block_statement.body; scan != 0; scan = scan->next)
            {
                DEBUG_AST_Print(workspace, arena, scan);
                if (AST_StatementNeedsSemicolon(scan)) String_Printf(arena, ";");
            }
            
            String_Printf(arena, "%S}", WhitespaceInfo_ToString(file, info->block_info.ws_close_brace));
        }
        else if (node->kind == AST_If)
        {
            if (node->if_statement.label != INTERNED_STRING_NIL)
            {
                String_Printf(arena, "%S%S:%S", InternedString_ToString(workspace, node->if_statement.label), WhitespaceInfo_ToString(file, info->if_info.ws_colon),
                              WhitespaceInfo_ToString(file, info->if_info.ws_if));
            }
            
            String_Printf(arena, "if%S(", WhitespaceInfo_ToString(file, info->if_info.ws_open_paren));
            
            if (node->if_statement.init != 0)
            {
                String_Printf(arena, "%S;", WhitespaceInfo_ToString(file, info->if_info.ws_init_semi));
                DEBUG_AST_Print(workspace, arena, node->if_statement.init);
            }
            
            DEBUG_AST_Print(workspace, arena, node->if_statement.condition);
            
            String_Printf(arena, "%S)", WhitespaceInfo_ToString(file, info->if_info.ws_close_paren));
            
            if (node->if_statement.true_body != 0) DEBUG_AST_Print(workspace, arena, node->if_statement.true_body);
            else                                   String_Printf(arena, "%S;", WhitespaceInfo_ToString(file, info->if_info.ws_if_body_semi));
            
            if (node->if_statement.false_body != 0)
            {
                String_Printf(arena, "%Selse", WhitespaceInfo_ToString(file, info->if_info.ws_else));
                
                if (node->if_statement.false_body != 0) DEBUG_AST_Print(workspace, arena, node->if_statement.false_body);
                else                                    String_Printf(arena, "%S;", WhitespaceInfo_ToString(file, info->if_info.ws_else_body_semi));
            }
        }
        else if (node->kind == AST_While)
        {
            if (node->while_statement.label != INTERNED_STRING_NIL)
            {
                String_Printf(arena, "%S%S:%S", InternedString_ToString(workspace, node->while_statement.label), WhitespaceInfo_ToString(file, info->while_info.ws_colon),
                              WhitespaceInfo_ToString(file, info->while_info.ws_while));
            }
            
            String_Printf(arena, "while%S(", WhitespaceInfo_ToString(file, info->while_info.ws_open_paren));
            
            if (node->while_statement.init != 0)
            {
                String_Printf(arena, "%S;", WhitespaceInfo_ToString(file, info->while_info.ws_init_semi));
                DEBUG_AST_Print(workspace, arena, node->while_statement.init);
            }
            
            DEBUG_AST_Print(workspace, arena, node->while_statement.condition);
            
            if (node->while_statement.step != 0)
            {
                String_Printf(arena, "%S;", WhitespaceInfo_ToString(file, info->while_info.ws_cond_semi));
                DEBUG_AST_Print(workspace, arena, node->while_statement.step);
            }
            
            String_Printf(arena, "%S)", WhitespaceInfo_ToString(file, info->while_info.ws_close_paren));
            
            if (node->while_statement.body != 0) DEBUG_AST_Print(workspace, arena, node->while_statement.body);
            else                                 String_Printf(arena, "%S;", WhitespaceInfo_ToString(file, info->while_info.ws_body_semi));
        }
        else if (node->kind == AST_Break || node->kind == AST_Continue)
        {
            String_Printf(arena, (node->kind == AST_Break ? "break" : "continue"));
            
            if (node->jump_label != INTERNED_STRING_NIL)
            {
                String_Printf(arena, "%S%S", WhitespaceInfo_ToString(file, info->jump_info.ws_label), InternedString_ToString(workspace, node->jump_label));
            }
        }
        else if (node->kind == AST_Defer)
        {
            String_Printf(arena, "defer");
            DEBUG_AST_Print(workspace, arena, node->defer_statement);
        }
        else if (node->kind == AST_Return)
        {
            String_Printf(arena, "return");
            
            for (AST_Node* scan = node->return_values; scan != 0; scan = scan->next)
            {
                DEBUG_AST_Print(workspace, arena, scan);
                if (scan->next != 0) String_Printf(arena, ",");
            }
        }
        else if (node->kind == AST_Assignment)
        {
            for (AST_Node* scan = node->assignment_statement.lhs; scan != 0; scan = scan->next)
            {
                DEBUG_AST_Print(workspace, arena, scan);
                if (scan->next != 0) String_Printf(arena, ",");
            }
            
            String ws_op = WhitespaceInfo_ToString(file, info->assignment_info.ws_op);
            switch (node->assignment_statement.op)
            {
                case AST_Nil:    String_Printf(arena, "%S=",    ws_op); break;
                case AST_Mul:    String_Printf(arena, "%S*=",   ws_op); break;
                case AST_Div:    String_Printf(arena, "%S/=",   ws_op); break;
                case AST_Rem:    String_Printf(arena, "%S%=",   ws_op); break;
                case AST_BitAnd: String_Printf(arena, "%S&=",   ws_op); break;
                case AST_BitShl: String_Printf(arena, "%S<<=",  ws_op); break;
                case AST_BitShr: String_Printf(arena, "%S>>=",  ws_op); break;
                case AST_BitSar: String_Printf(arena, "%S>>>=", ws_op); break;
                case AST_BitXor: String_Printf(arena, "%S~=",   ws_op); break;
                case AST_BitOr:  String_Printf(arena, "%S|=",   ws_op); break;
                case AST_Sub:    String_Printf(arena, "%S-=",   ws_op); break;
                case AST_Add:    String_Printf(arena, "%S+=",   ws_op); break;
                case AST_And:    String_Printf(arena, "%S&&=",  ws_op); break;
                case AST_Or:     String_Printf(arena, "%S||=",  ws_op); break;
                INVALID_DEFAULT_CASE;
            }
            
            for (AST_Node* scan = node->assignment_statement.rhs; scan != 0; scan = scan->next)
            {
                DEBUG_AST_Print(workspace, arena, scan);
                if (scan->next != 0) String_Printf(arena, ",");
            }
        }
        else INVALID_CODE_PATH;
    }
    else if (node->kind >= AST_FirstDeclaration && node->kind <= AST_LastDeclaration)
    {
        if (node->kind == AST_Variable)
        {
            for (AST_Node* scan = node->variable_decl.names; scan != 0; scan = scan->next)
            {
                DEBUG_AST_Print(workspace, arena, scan);
                if (scan->next != 0) String_Printf(arena, ",");
            }
            
            String_Printf(arena, "%S:", WhitespaceInfo_ToString(file, info->variable_info.ws_colon));
            
            if (node->variable_decl.type != 0) DEBUG_AST_Print(workspace, arena, node->variable_decl.type);
            
            if (node->variable_decl.is_uninitialized)
            {
                String_Printf(arena, "%S=%S---", WhitespaceInfo_ToString(file, info->variable_info.ws_equals), WhitespaceInfo_ToString(file, info->variable_info.ws_uninit));
            }
            else if (node->variable_decl.values != 0)
            {
                String_Printf(arena, "%S=", WhitespaceInfo_ToString(file, info->variable_info.ws_equals));
                
                for (AST_Node* scan = node->variable_decl.values; scan != 0; scan = scan->next)
                {
                    DEBUG_AST_Print(workspace, arena, scan);
                    if (scan->next != 0) String_Printf(arena, ",");
                }
            }
        }
        else if (node->kind == AST_Constant)
        {
            for (AST_Node* scan = node->variable_decl.names; scan != 0; scan = scan->next)
            {
                DEBUG_AST_Print(workspace, arena, scan);
                if (scan->next != 0) String_Printf(arena, ",");
            }
            
            String_Printf(arena, "%S:", WhitespaceInfo_ToString(file, info->variable_info.ws_colon));
            
            if (node->variable_decl.type != 0) DEBUG_AST_Print(workspace, arena, node->variable_decl.type);
            
            String_Printf(arena, "%S=", WhitespaceInfo_ToString(file, info->variable_info.ws_equals));
            
            for (AST_Node* scan = node->variable_decl.values; scan != 0; scan = scan->next)
            {
                DEBUG_AST_Print(workspace, arena, scan);
                if (scan->next != 0) String_Printf(arena, ",");
            }
        }
        else if (node->kind == AST_Using)
        {
            String_Printf(arena, "using");
            
            for (AST_Node* scan = node->using_decl.symbol_paths; scan != 0; scan = scan->next)
            {
                DEBUG_AST_Print(workspace, arena, scan);
                if (scan->next != 0) String_Printf(arena, ",");
            }
            
            if (node->using_decl.alias != INTERNED_STRING_NIL)
            {
                String_Printf(arena, "%Sas%S%S", WhitespaceInfo_ToString(file, info->using_info.ws_as), WhitespaceInfo_ToString(file, info->using_info.ws_alias),
                              InternedString_ToString(workspace, node->using_decl.alias));
            }
        }
        else INVALID_CODE_PATH;
    }
    else if (node->kind == AST_NamedValue)
    {
        if (node->named_value.name != 0)
        {
            DEBUG_AST_Print(workspace, arena, node->named_value.name);
            String_Printf(arena, "%S=", WhitespaceInfo_ToString(file, info->named_value_info.ws_equals));
        }
        
        DEBUG_AST_Print(workspace, arena, node->named_value.value);
    }
    else if (node->kind == AST_Parameter)
    {
        if (node->parameter.name != 0)
        {
            DEBUG_AST_Print(workspace, arena, node->parameter.name);
            String_Printf(arena, "%S:", WhitespaceInfo_ToString(file, info->parameter_info.ws_colon));
        }
        
        if (node->parameter.type != 0) DEBUG_AST_Print(workspace, arena, node->parameter.type);
        
        if (node->parameter.value != 0)
        {
            String_Printf(arena, "%S=", WhitespaceInfo_ToString(file, info->parameter_info.ws_equals));
            DEBUG_AST_Print(workspace, arena, node->parameter.value);
        }
    }
    else if (node->kind == AST_EnumMember)
    {
        DEBUG_AST_Print(workspace, arena, node->enum_member.name);
        
        if (node->enum_member.value != 0)
        {
            String_Printf(arena, "%S=", WhitespaceInfo_ToString(file, info->enum_member_info.ws_equals));
            DEBUG_AST_Print(workspace, arena, node->enum_member.value);
        }
    }
    else if (node->kind >= AST_FirstBinary && node->kind <= AST_LastBinary)
    {
        DEBUG_AST_Print(workspace, arena, node->binary_expr.left);
        
        String ws_op = WhitespaceInfo_ToString(file, info->binary_info.ws_op);
        switch (node->kind)
        {
            case AST_Mul:               String_Printf(arena, "%S*",   ws_op); break;
            case AST_Div:               String_Printf(arena, "%S/",   ws_op); break;
            case AST_Rem:               String_Printf(arena, "%S%",   ws_op); break;
            case AST_BitAnd:            String_Printf(arena, "%S&",   ws_op); break;
            case AST_BitShl:            String_Printf(arena, "%S<<",  ws_op); break;
            case AST_BitShr:            String_Printf(arena, "%S>>",  ws_op); break;
            case AST_BitSar:            String_Printf(arena, "%S>>>", ws_op); break;
            case AST_BitOr:             String_Printf(arena, "%S|",   ws_op); break;
            case AST_BitXor:            String_Printf(arena, "%S~",   ws_op); break;
            case AST_Sub:               String_Printf(arena, "%S-",   ws_op); break;
            case AST_Add:               String_Printf(arena, "%S+",   ws_op); break;
            case AST_IsStrictlyGreater: String_Printf(arena, "%S>",   ws_op); break;
            case AST_IsGreater:         String_Printf(arena, "%S>=",  ws_op); break;
            case AST_IsStrictlyLess:    String_Printf(arena, "%S<",   ws_op); break;
            case AST_IsLess:            String_Printf(arena, "%S<=",  ws_op); break;
            case AST_IsEquals:          String_Printf(arena, "%S==",  ws_op); break;
            case AST_IsNotEquals:       String_Printf(arena, "%S!=",  ws_op); break;
            case AST_And:               String_Printf(arena, "%S&&",  ws_op); break;
            case AST_Or:                String_Printf(arena, "%S||",  ws_op); break;
            INVALID_DEFAULT_CASE;
        }
        
        DEBUG_AST_Print(workspace, arena, node->binary_expr.right);
    }
    else if (node->kind == AST_Conditional)
    {
        DEBUG_AST_Print(workspace, arena, node->conditional_expr.condition);
        String_Printf(arena, "%S?", WhitespaceInfo_ToString(file, info->conditional_info.ws_qmark));
        DEBUG_AST_Print(workspace, arena, node->conditional_expr.true_expr);
        String_Printf(arena, "%S?", WhitespaceInfo_ToString(file, info->conditional_info.ws_colon));
        DEBUG_AST_Print(workspace, arena, node->conditional_expr.false_expr);
    }
    else if (node->kind == AST_Selector || node->kind == AST_PolymorphicName)
    {
        if (node->kind == AST_Selector) String_Printf(arena, ".%S%S", WhitespaceInfo_ToString(file, info->selector_info.ws_name), InternedString_ToString(workspace, node->selector_expr));
        else                            String_Printf(arena, "$%S%S", WhitespaceInfo_ToString(file, info->poly_name_info.ws_name), InternedString_ToString(workspace, node->poly_name));
    }
    else if (node->kind == AST_ArrayType)
    {
        String_Printf(arena, "[");
        DEBUG_AST_Print(workspace, arena, node->array_type.size);
        String_Printf(arena, "%S]", WhitespaceInfo_ToString(file, info->array_type_info.ws_close_bracket));
        DEBUG_AST_Print(workspace, arena, node->array_type.elem_type);
    }
    else if (node->kind >= AST_FirstPrefix && node->kind <= AST_LastPrefix)
    {
        switch (node->kind)
        {
            case AST_Reference:       String_Printf(arena, "^");                                                                      break;
            case AST_SliceType:       String_Printf(arena, "[%S]", WhitespaceInfo_ToString(file, info->slice_info.ws_close_bracket)); break;
            case AST_Neg:             String_Printf(arena, "-");                                                                      break;
            case AST_BitNot:          String_Printf(arena, "~");                                                                      break;
            case AST_Not:             String_Printf(arena, "!");                                                                      break;
        }
        
        DEBUG_AST_Print(workspace, arena, node->unary_expr);
    }
    else if (node->kind == AST_Dereference)
    {
        DEBUG_AST_Print(workspace, arena, node->unary_expr);
        String_Printf(arena, "%S^", WhitespaceInfo_ToString(file, info->deref_info.ws_hat));
    }
    else if (node->kind == AST_Member)
    {
        DEBUG_AST_Print(workspace, arena, node->member_expr.expr);
        String_Printf(arena, "%S.%S%S", WhitespaceInfo_ToString(file, info->member_info.ws_period),
                      WhitespaceInfo_ToString(file, info->member_info.ws_member), InternedString_ToString(workspace, node->member_expr.member));
    }
    else if (node->kind == AST_StructLiteral)
    {
        if (node->struct_literal.type != 0) DEBUG_AST_Print(workspace, arena, node->struct_literal.type);
        String_Printf(arena, "%S.{", WhitespaceInfo_ToString(file, info->struct_literal_info.ws_period_brace));
        
        for (AST_Node* scan = node->struct_literal.args; scan != 0; scan = scan->next)
        {
            DEBUG_AST_Print(workspace, arena, scan);
            if (scan->next != 0) String_Printf(arena, ",");
        }
        
        String_Printf(arena, "%S}", WhitespaceInfo_ToString(file, info->struct_literal_info.ws_close_brace));
    }
    else if (node->kind == AST_ArrayLiteral)
    {
        if (node->array_literal.type != 0) DEBUG_AST_Print(workspace, arena, node->array_literal.type);
        String_Printf(arena, "%S.[", WhitespaceInfo_ToString(file, info->array_literal_info.ws_period_bracket));
        
        for (AST_Node* scan = node->array_literal.args; scan != 0; scan = scan->next)
        {
            DEBUG_AST_Print(workspace, arena, scan);
            if (scan->next != 0) String_Printf(arena, ",");
        }
        
        String_Printf(arena, "%S]", WhitespaceInfo_ToString(file, info->array_literal_info.ws_close_bracket));
    }
    else if (node->kind == AST_Subscript)
    {
        DEBUG_AST_Print(workspace, arena, node->subscript_expr.array);
        
        String_Printf(arena, "%S[", WhitespaceInfo_ToString(file, info->subscript_info.ws_open_bracket));
        DEBUG_AST_Print(workspace, arena, node->subscript_expr.index);
        String_Printf(arena, "%S]", WhitespaceInfo_ToString(file, info->subscript_info.ws_close_bracket));
    }
    else if (node->kind == AST_Slice)
    {
        DEBUG_AST_Print(workspace, arena, node->slice_expr.array);
        
        String_Printf(arena, "%S[", WhitespaceInfo_ToString(file, info->slice_info.ws_open_bracket));
        if (node->slice_expr.start) DEBUG_AST_Print(workspace, arena, node->slice_expr.start);
        String_Printf(arena, "%S:", WhitespaceInfo_ToString(file, info->slice_info.ws_colon));
        if (node->slice_expr.past_end) DEBUG_AST_Print(workspace, arena, node->slice_expr.past_end);
        String_Printf(arena, "%S]", WhitespaceInfo_ToString(file, info->slice_info.ws_close_bracket));
    }
    else if (node->kind == AST_Call || node->kind == AST_IntrinsicCall)
    {
        if (node->kind == AST_Call) DEBUG_AST_Print(workspace, arena, node->call_expr.proc);
        else                        String_Printf(arena, "%S", InternedString_ToString(workspace, node->intrinsic_call_expr.proc));
        
        String_Printf(arena, "%S(", WhitespaceInfo_ToString(file, info->call_info.ws_open_paren));
        
        for (AST_Node* scan = (node->kind == AST_Call ? node->call_expr.args : node->intrinsic_call_expr.args); scan != 0; scan = scan->next)
        {
            DEBUG_AST_Print(workspace, arena, scan);
            if (scan->next != 0) String_Printf(arena, ",");
        }
        
        String_Printf(arena, "%S)", WhitespaceInfo_ToString(file, info->call_info.ws_close_paren));
    }
    else if (node->kind == AST_Proc || node->kind == AST_ProcLiteral)
    {
        String_Printf(arena, "proc");
        
        // NOTE: proc_literal is a superset of proc_type
        if (node->proc_literal.params != 0)
        {
            String_Printf(arena, "%S(", WhitespaceInfo_ToString(file, info->proc_info.ws_open_paren));
            
            for (AST_Node* scan = node->proc_literal.params; scan != 0; scan = scan->next)
            {
                DEBUG_AST_Print(workspace, arena, scan);
                if (scan->next != 0) String_Printf(arena, ",");
            }
            
            String_Printf(arena, "%S)", WhitespaceInfo_ToString(file, info->proc_info.ws_close_paren));
        }
        
        if (node->proc_literal.return_types != 0)
        {
            String_Printf(arena, "%S->", WhitespaceInfo_ToString(file, info->proc_info.ws_arrow));
            
            if (node->proc_literal.return_types->next == 0) DEBUG_AST_Print(workspace, arena, node->proc_literal.return_types);
            else
            {
                String_Printf(arena, "%S(", WhitespaceInfo_ToString(file, info->proc_info.ws_open_paren_ret));
                
                for (AST_Node* scan = node->proc_literal.return_types; scan != 0; scan = scan->next)
                {
                    DEBUG_AST_Print(workspace, arena, scan);
                    if (scan->next != 0) String_Printf(arena, ",");
                }
                
                String_Printf(arena, "%S)", WhitespaceInfo_ToString(file, info->proc_info.ws_close_paren_ret));
            }
        }
        
        if (node->proc_literal.where_clause != 0)
        {
            String_Printf(arena, "%S", WhitespaceInfo_ToString(file, info->proc_info.ws_where));
            DEBUG_AST_Print(workspace, arena, node->proc_literal.where_clause);
        }
        
        if (node->kind == AST_ProcLiteral)
        {
            if (node->proc_literal.body == 0) String_Printf(arena, "%S---", WhitespaceInfo_ToString(file, info->proc_info.ws_uninit));
            else                              DEBUG_AST_Print(workspace, arena, node->proc_literal.body);
        }
    }
    else if (node->kind == AST_Struct || node->kind == AST_Union)
    {
        String_Printf(arena, (node->kind == AST_Struct ? "struct" : "union"));
        
        if (node->struct_type.params != 0)
        {
            String_Printf(arena, "%S(", WhitespaceInfo_ToString(file, info->struct_info.ws_open_paren));
            
            for (AST_Node* scan = node->struct_type.params; scan != 0; scan = scan->next)
            {
                DEBUG_AST_Print(workspace, arena, scan);
                if (scan->next) String_Printf(arena, ",");
            }
            
            String_Printf(arena, "%S)", WhitespaceInfo_ToString(file, info->struct_info.ws_close_paren));
        }
        
        String_Printf(arena, "%S{", WhitespaceInfo_ToString(file, info->struct_info.ws_open_brace));
        
        for (AST_Node* scan = node->struct_type.body; scan != 0; scan = scan->next)
        {
            DEBUG_AST_Print(workspace, arena, scan);
            if (scan->next) String_Printf(arena, ",");
        }
        
        String_Printf(arena, "%S}", WhitespaceInfo_ToString(file, info->struct_info.ws_close_brace));
    }
    else if (node->kind == AST_Enum)
    {
        String_Printf(arena, "enum");
        if (node->enum_type.type != 0) DEBUG_AST_Print(workspace, arena, node->enum_type.type);
        
        String_Printf(arena, "%S{", WhitespaceInfo_ToString(file, info->enum_info.ws_open_brace));
        
        for (AST_Node* scan = node->enum_type.body; scan != 0; scan = scan->next)
        {
            DEBUG_AST_Print(workspace, arena, scan);
            if (scan->next) String_Printf(arena, ",");
        }
        
        String_Printf(arena, "%S}", WhitespaceInfo_ToString(file, info->enum_info.ws_close_brace));
    }
    else if (node->kind == AST_Compound)
    {
        String_Printf(arena, "(");
        DEBUG_AST_Print(workspace, arena, node->compound_expr);
        String_Printf(arena, "%S)", WhitespaceInfo_ToString(file, info->compound_info.ws_close_paren));
    }
    else
    {
        switch (node->kind)
        {
            case AST_Identifier: String_Printf(arena, "%S", InternedString_ToString(workspace, node->string));     break;
            case AST_String:     String_Printf(arena, "\"%S\"", InternedString_ToString(workspace, node->string)); break;
            case AST_Char:       String_Printf(arena, "'%c'", node->character);                                    break;
            case AST_Bool:       String_Printf(arena, "%S", (node->boolean ? STRING("true") : STRING("false")));   break;
            case AST_Int:        String_Printf(arena, "%BX", node->integer.big_int, node->integer.base);           break;
            case AST_Float:      String_Printf(arena, "%BF", node->floating.big_float, node->floating.byte_size);  break;
            INVALID_DEFAULT_CASE;
        }
    }
    
    String_Printf(arena, "%S", WhitespaceInfo_ToString(file, info->ws_terminator));
}