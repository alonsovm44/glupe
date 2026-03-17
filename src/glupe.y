%{
#include "ast.hpp"
#include <iostream>
#include <vector>
#include <string>

extern int yylex();
extern int yylineno;
void yyerror(ProgramNode** root, const char* s) {
    std::cerr << "Parse error at line " << yylineno << ": " << s << std::endl;
}
%}

%parse-param { ProgramNode** root }

%union {
    std::string* str;
    ASTNode* node;
    ProgramNode* prog;
    std::vector<std::string>* str_list;
    bool boolean;
}

%destructor { delete $$; } <str> <node> <str_list>

%token <str> T_RAW_CODE T_IDENTIFIER T_VAR_VALUE T_INTENT_TEXT T_STRING_LITERAL T_NUMBER
%token T_PERSISTENT_VAR_START T_CONST_VAR_START T_EPHEMERAL_VAR_START
%token T_BLOCK_START T_INLINE_START T_ABSTRACT T_ARROW T_COMMA T_LPAREN T_RPAREN T_LBRACE T_BLOCK_END T_INLINE_END T_NEWLINE
%token T_PLUS T_MINUS T_STAR T_SLASH T_CARET

%type <prog> program
%type <node> element var_decl container_decl
%type <str_list> expr_list optional_params optional_parents
%type <str> intent_body string_or_id opt_var_value algebra_expr algebra_term
%type <boolean> optional_abstract

%%

program:
    /* empty */
    {
        *root = new ProgramNode();
        $$ = *root;
    }
    | program element
    {
        if ($2) {
            $1->addElement(std::unique_ptr<ASTNode>($2));
        }
        $$ = $1;
    }
    ;

element:
    T_RAW_CODE
    {
        $$ = new RawCodeNode(*$1);
        delete $1;
    }
    | var_decl { $$ = $1; }
    | container_decl { $$ = $1; }
    | T_NEWLINE { $$ = nullptr; }
    ;

opt_var_value:
    /* empty */ { $$ = new std::string(""); }
    | T_VAR_VALUE { $$ = $1; }
    ;

var_decl:
    T_PERSISTENT_VAR_START T_IDENTIFIER opt_var_value T_NEWLINE
    {
        $$ = new VariableNode(*$2, *$3, VarType::PERSISTENT);
        delete $2; delete $3;
    }
    | T_CONST_VAR_START T_IDENTIFIER opt_var_value T_NEWLINE
    {
        $$ = new VariableNode(*$2, *$3, VarType::CONSTANT);
        delete $2; delete $3;
    }
    | T_EPHEMERAL_VAR_START T_IDENTIFIER opt_var_value T_NEWLINE
    {
        $$ = new VariableNode(*$2, *$3, VarType::EPHEMERAL);
        delete $2; delete $3;
    }
    ;

optional_abstract:
    /* empty */ { $$ = false; }
    | T_ABSTRACT { $$ = true; }
    ;

optional_params:
    /* empty */ { $$ = new std::vector<std::string>(); }
    | T_LPAREN expr_list T_RPAREN { $$ = $2; }
    ;

optional_parents:
    /* empty */ { $$ = new std::vector<std::string>(); }
    | T_ARROW expr_list { $$ = $2; }
    ;

expr_list:
    algebra_expr
    {
        $$ = new std::vector<std::string>();
        $$->push_back(*$1);
        delete $1;
    }
    | expr_list T_COMMA algebra_expr
    {
        $1->push_back(*$3);
        delete $3;
        $$ = $1;
    }
    ;

algebra_expr:
    algebra_term { $$ = $1; }
    | algebra_expr T_PLUS algebra_term { $$ = new std::string(*$1 + " + " + *$3); delete $1; delete $3; }
    | algebra_expr T_MINUS algebra_term { $$ = new std::string(*$1 + " - " + *$3); delete $1; delete $3; }
    | algebra_expr T_STAR algebra_term { $$ = new std::string(*$1 + " * " + *$3); delete $1; delete $3; }
    | algebra_expr T_SLASH algebra_term { $$ = new std::string(*$1 + " / " + *$3); delete $1; delete $3; }
    | algebra_expr T_CARET algebra_term { $$ = new std::string(*$1 + " ^ " + *$3); delete $1; delete $3; }
    ;

algebra_term:
    T_IDENTIFIER { $$ = new std::string("$" + *$1); delete $1; }
    | T_STRING_LITERAL { $$ = new std::string("\"" + *$1 + "\""); delete $1; }
    | T_NUMBER { $$ = $1; }
    ;

string_or_id:
    T_IDENTIFIER { $$ = $1; }
    | T_STRING_LITERAL { $$ = $1; }
    ;

intent_body:
    /* empty */ { $$ = new std::string(""); }
    | intent_body T_INTENT_TEXT
    {
        $$ = new std::string(*$1 + *$2);
        delete $1; delete $2;
    }
    ;

container_decl:
    T_BLOCK_START optional_abstract string_or_id optional_params optional_parents T_LBRACE intent_body T_BLOCK_END
    {
        auto* cont = new ContainerNode(*$3, *$7, true, $2);
        cont->params = *$4;
        cont->parents = *$5;
        $$ = cont;
        delete $3; delete $4; delete $5; delete $7;
    }
    | T_BLOCK_START optional_abstract T_LBRACE intent_body T_BLOCK_END
    {
        auto* cont = new ContainerNode("", *$4, true, $2);
        $$ = cont;
        delete $4;
    }
    | T_INLINE_START optional_abstract string_or_id optional_params optional_parents T_LBRACE intent_body T_INLINE_END
    {
        auto* cont = new ContainerNode(*$3, *$7, false, $2);
        cont->params = *$4;
        cont->parents = *$5;
        $$ = cont;
        delete $3; delete $4; delete $5; delete $7;
    }
    | T_INLINE_START optional_abstract T_LBRACE intent_body T_INLINE_END
    {
        auto* cont = new ContainerNode("", *$4, false, $2);
        $$ = cont;
        delete $4;
    }
    ;

%%