%{
    #include <iostream>
    #include <cstdio>
    #include <string>
    #include <memory>
    #include <vector>
    #include <fstream>
    #include <unistd.h>
    #include <cstdlib>
    #include <cstring>
    #include "../ast/ast.h"
    #include "../semantic/semantic.h"
    #include "../codegen/codegen.h"
    
    extern int yylex();
    extern int yyparse();
    extern FILE* yyin;
    extern int yylineno;

    std::unique_ptr<ProgramNode> programRoot;
    
    void yyerror(const char* s) {
        std::cerr << "Syntax error at line " << yylineno << ": " << s << std::endl;
    }
    
    Type parseType(const char* typeStr) {
        return stringToType(typeStr);
    }
%}

%union {
    char* str;
    int int_val;
    double double_val;
    float float_val;
    bool bool_val;
    void* node;
    void* expr;
    void* stmt;
    void* block;
    void* func;
    void* var_decl;
    void* param_list;
    void* expr_list;
    void* case_list;
    void* class_def;
    void* template_def;
    void* method_def;
    void* constructor_def;
    void* class_body;
}

%token <str> DIRECTIVE
%token RETURN FUNCTION IF ELSE WHILE DO FOR
%token CLASS PRIVATE PUBLIC TEMPLATE
%token BREAK CONTINUE SWITCH CASE DEFAULT
%token CONST
%token <int_val> INTEGER
%token <double_val> DOUBLE
%token <float_val> FLOAT
%token <bool_val> BOOLEAN
%token <str> STRING
%token <str> TYPE VAR
%token ASSIGN SEMICOLON LPAREN RPAREN LBRACE RBRACE COMMA
%token PLUS MINUS STAR SLASH MOD
%token PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN SLASH_ASSIGN
%token INCREMENT DECREMENT
%token EQ NE LT GT LE GE
%token AND OR NOT
%token ARROW
%token LBRACKET RBRACKET
%token COLON QUESTION

%type <node> top_level_item directive program
%type <func> function_def
%type <template_def> template_def
%type <class_def> class_def
%type <method_def> method_def
%type <constructor_def> constructor_def
%type <str> access_specifier
%type <class_body> class_body
%type <node> class_members class_member
%type <block> function_body block
%type <stmt> statement return_stmt var_decl var_assign inc_dec_stmt if_stmt while_stmt do_while_stmt for_stmt switch_stmt break_stmt continue_stmt
%type <block> default_case
%type <expr> expression
%type <param_list> param_list
%type <expr_list> expression_list
%type <case_list> case_list
%type <node> case_item
%type <str> type_spec

%left OR
%left AND
%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left STAR SLASH MOD
%right NOT NEG
%right ARROW
%right QUESTION
%left COLON

%%

program:
    {
        programRoot = std::make_unique<ProgramNode>();
        programRoot->line = 1;
    }
    program_items
    {
        $$ = programRoot.get();
    }
    ;

program_items:
    program_items top_level_item
    {
        if ($2) {
            auto* node = static_cast<ASTNode*>($2);
            if (auto* dir = dynamic_cast<DirectiveNode*>(node)) {
                programRoot->directives.push_back(std::unique_ptr<ASTNode>(dir));
            } else if (auto* func = dynamic_cast<FunctionNode*>(node)) {
                programRoot->functions.push_back(std::unique_ptr<FunctionNode>(func));
            } else if (auto* templ = dynamic_cast<TemplateNode*>(node)) {
                programRoot->templates.push_back(std::unique_ptr<TemplateNode>(templ));
            } else if (auto* cls = dynamic_cast<ClassNode*>(node)) {
                programRoot->classes.push_back(std::unique_ptr<ClassNode>(cls));
            } else if (auto* var = dynamic_cast<VarDeclNode*>(node)) {
                programRoot->globals.push_back(std::unique_ptr<VarDeclNode>(var));
            } else {
                delete node;
            }
        }
    }
    | top_level_item
    {
        if ($1) {
            auto* node = static_cast<ASTNode*>($1);
            if (auto* dir = dynamic_cast<DirectiveNode*>(node)) {
                programRoot->directives.push_back(std::unique_ptr<ASTNode>(dir));
            } else if (auto* func = dynamic_cast<FunctionNode*>(node)) {
                programRoot->functions.push_back(std::unique_ptr<FunctionNode>(func));
            } else if (auto* templ = dynamic_cast<TemplateNode*>(node)) {
                programRoot->templates.push_back(std::unique_ptr<TemplateNode>(templ));
            } else if (auto* cls = dynamic_cast<ClassNode*>(node)) {
                programRoot->classes.push_back(std::unique_ptr<ClassNode>(cls));
            } else if (auto* var = dynamic_cast<VarDeclNode*>(node)) {
                programRoot->globals.push_back(std::unique_ptr<VarDeclNode>(var));
            } else {
                delete node;
            }
        }
    }
    | /* empty */
    ;

top_level_item:
    directive { $$ = $1; }
    | function_def { $$ = $1; }
    | template_def { $$ = $1; }
    | class_def { $$ = $1; }
    | var_decl { $$ = $1; }
    ;

directive:
    DIRECTIVE STRING
    {
        auto* dir = new DirectiveNode();
        dir->line = yylineno;
        dir->directive = $1;
        dir->filename = $2;
        $$ = dir;
        free($1);
        free($2);
    }
    | DIRECTIVE STRING SEMICOLON
    {
        auto* dir = new DirectiveNode();
        dir->line = yylineno;
        dir->directive = $1;
        dir->filename = $2;
        $$ = dir;
        free($1);
        free($2);
    }
    ;

type_spec:
    TYPE { $$ = $1; }
    ;

function_def:
    FUNCTION VAR LPAREN param_list RPAREN ARROW type_spec LBRACE function_body RBRACE
    {
        auto* func = new FunctionNode();
        func->line = yylineno;
        func->name = $2;
        func->returnType = parseType($7);
        if ($4) {
            auto* list = static_cast<std::vector<std::pair<std::string, Type>>*>($4);
            func->parameters = *list;
            delete list;
        }
        if ($9) {
            func->body = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($9));
        } else {
            func->body = std::make_unique<BlockNode>();
        }
        $$ = func;
        free($2);
        free($7);
    }
    | FUNCTION VAR LPAREN RPAREN ARROW type_spec LBRACE function_body RBRACE
    {
        auto* func = new FunctionNode();
        func->line = yylineno;
        func->name = $2;
        func->returnType = parseType($6);
        if ($8) {
            func->body = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($8));
        } else {
            func->body = std::make_unique<BlockNode>();
        }
        $$ = func;
        free($2);
        free($6);
    }
    | FUNCTION VAR LPAREN param_list RPAREN LBRACE function_body RBRACE
    {
        auto* func = new FunctionNode();
        func->line = yylineno;
        func->name = $2;
        func->returnType = Type::VOID;
        if ($4) {
            auto* list = static_cast<std::vector<std::pair<std::string, Type>>*>($4);
            func->parameters = *list;
            delete list;
        }
        if ($7) {
            func->body = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($7));
        } else {
            func->body = std::make_unique<BlockNode>();
        }
        $$ = func;
        free($2);
    }
    | FUNCTION VAR LPAREN RPAREN LBRACE function_body RBRACE
    {
        auto* func = new FunctionNode();
        func->line = yylineno;
        func->name = $2;
        func->returnType = Type::VOID;
        if ($6) {
            func->body = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($6));
        } else {
            func->body = std::make_unique<BlockNode>();
        }
        $$ = func;
        free($2);
    }
    ;

param_list:
    param_list COMMA type_spec VAR
    {
        auto* list = static_cast<std::vector<std::pair<std::string, Type>>*>($1);
        if (!list) list = new std::vector<std::pair<std::string, Type>>();
        list->push_back({$4, parseType($3)});
        $$ = list;
        free($3);
        free($4);
    }
    | type_spec VAR
    {
        auto* list = new std::vector<std::pair<std::string, Type>>();
        list->push_back({$2, parseType($1)});
        $$ = list;
        free($1);
        free($2);
    }
    ;

function_body:
    function_body statement
    {
        if (!$1) $1 = new BlockNode();
        if ($2) {
            static_cast<BlockNode*>($1)->statements.push_back(std::unique_ptr<StatementNode>(static_cast<StatementNode*>($2)));
        }
        $$ = $1;
    }
    | statement
    {
        auto* blk = new BlockNode();
        if ($1) {
            blk->statements.push_back(std::unique_ptr<StatementNode>(static_cast<StatementNode*>($1)));
        }
        $$ = blk;
    }
    | /* empty */ { $$ = NULL; }
    ;

block:
    LBRACE function_body RBRACE
    {
        if ($2) {
            $$ = $2;
        } else {
            $$ = new BlockNode();
        }
    }
    ;

statement:
    return_stmt { $$ = $1; }
    | var_decl { $$ = $1; }
    | var_assign { $$ = $1; }
    | inc_dec_stmt { $$ = $1; }
    | if_stmt { $$ = $1; }
    | while_stmt { $$ = $1; }
    | do_while_stmt { $$ = $1; }
    | for_stmt { $$ = $1; }
    | switch_stmt { $$ = $1; }
    | break_stmt { $$ = $1; }
    | continue_stmt { $$ = $1; }
    | expression SEMICOLON { $$ = nullptr; if ($1) delete static_cast<ExpressionNode*>($1); }
    ;

return_stmt:
    RETURN expression SEMICOLON
    {
        auto* ret = new ReturnNode();
        ret->line = yylineno;
        if ($2) {
            ret->value = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($2));
        }
        $$ = ret;
    }
    | RETURN SEMICOLON
    {
        auto* ret = new ReturnNode();
        ret->line = yylineno;
        ret->value = nullptr;
        $$ = ret;
    }
    ;

template_def:
    TEMPLATE LT VAR GT function_def
    {
        auto* templ = new TemplateNode();
        templ->line = yylineno;
        templ->templateParam = $3;
        templ->function = std::unique_ptr<FunctionNode>(static_cast<FunctionNode*>($5));
        $$ = templ;
        free($3);
    }
    ;

class_def:
    CLASS VAR LBRACE class_body RBRACE
    {
        auto* cls = new ClassNode();
        cls->line = yylineno;
        cls->name = $2;
        if ($4) {
            auto* body = static_cast<std::pair<ConstructorNode*, std::vector<MethodNode*>*>*>($4);
            if (body->first) {
                cls->constructor = std::unique_ptr<ConstructorNode>(body->first);
            }
            if (body->second) {
                for (auto* method : *body->second) {
                    cls->methods.push_back(std::unique_ptr<MethodNode>(method));
                }
                delete body->second;
            }
            delete body;
        }
        $$ = cls;
        free($2);
    }
    ;

class_body:
    class_members
    {
        $$ = $1;
    }
    | /* empty */
    {
        $$ = nullptr;
    }
    ;

class_members:
    class_members class_member
    {
        if ($1 && $2) {
            auto* result = static_cast<std::pair<ConstructorNode*, std::vector<MethodNode*>*>*>($1);
            auto* member = static_cast<std::pair<ConstructorNode*, std::vector<MethodNode*>*>*>($2);

            if (member->first) {
                result->first = member->first;
            }
            if (member->second) {
                for (auto* method : *member->second) {
                    result->second->push_back(method);
                }
                delete member->second;
            }
            delete member;
            $$ = result;
        } else {
            $$ = $1 ? $1 : $2;
        }
    }
    | class_member
    {
        $$ = $1;
    }
    ;

class_member:
    constructor_def
    {
        auto* result = new std::pair<ConstructorNode*, std::vector<MethodNode*>*>();
        result->first = static_cast<ConstructorNode*>($1);
        result->second = nullptr;
        $$ = result;
    }
    | method_def
    {
        auto* result = new std::pair<ConstructorNode*, std::vector<MethodNode*>*>();
        result->first = nullptr;
        result->second = new std::vector<MethodNode*>();
        result->second->push_back(static_cast<MethodNode*>($1));
        $$ = result;
    }
    ;

constructor_def:
    VAR LPAREN param_list RPAREN LBRACE function_body RBRACE
    {
        auto* ctor = new ConstructorNode();
        ctor->line = yylineno;
        ctor->className = $1;
        if ($3) {
            auto* list = static_cast<std::vector<std::pair<std::string, Type>>*>($3);
            ctor->parameters = *list;
            delete list;
        }
        if ($6) {
            ctor->body = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($6));
        } else {
            ctor->body = std::make_unique<BlockNode>();
        }
        $$ = ctor;
        free($1);
    }
    | VAR LPAREN RPAREN LBRACE function_body RBRACE
    {
        auto* ctor = new ConstructorNode();
        ctor->line = yylineno;
        ctor->className = $1;
        if ($5) {
            ctor->body = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($5));
        } else {
            ctor->body = std::make_unique<BlockNode>();
        }
        $$ = ctor;
        free($1);
    }
    ;

method_def:
    access_specifier type_spec VAR LPAREN param_list RPAREN LBRACE function_body RBRACE
    {
        auto* method = new MethodNode();
        method->line = yylineno;
        method->name = $3;
        method->returnType = parseType($2);
        method->isPrivate = ($1 && strcmp($1, "private") == 0);
        if ($5) {
            auto* list = static_cast<std::vector<std::pair<std::string, Type>>*>($5);
            method->parameters = *list;
            delete list;
        }
        if ($8) {
            method->body = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($8));
        } else {
            method->body = std::make_unique<BlockNode>();
        }
        $$ = method;
        if ($1) free($1);
        free($2);
        free($3);
    }
    | access_specifier type_spec VAR LPAREN RPAREN LBRACE function_body RBRACE
    {
        auto* method = new MethodNode();
        method->line = yylineno;
        method->name = $3;
        method->returnType = parseType($2);
        method->isPrivate = ($1 && strcmp($1, "private") == 0);
        if ($7) {
            method->body = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($7));
        } else {
            method->body = std::make_unique<BlockNode>();
        }
        $$ = method;
        if ($1) free($1);
        free($2);
        free($3);
    }
    | access_specifier VAR LPAREN param_list RPAREN LBRACE function_body RBRACE
    {
        auto* method = new MethodNode();
        method->line = yylineno;
        method->name = $2;
        method->returnType = Type::VOID;
        method->isPrivate = ($1 && strcmp($1, "private") == 0);
        if ($4) {
            auto* list = static_cast<std::vector<std::pair<std::string, Type>>*>($4);
            method->parameters = *list;
            delete list;
        }
        if ($7) {
            method->body = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($7));
        } else {
            method->body = std::make_unique<BlockNode>();
        }
        $$ = method;
        if ($1) free($1);
        free($2);
    }
    | access_specifier VAR LPAREN RPAREN LBRACE function_body RBRACE
    {
        auto* method = new MethodNode();
        method->line = yylineno;
        method->name = $2;
        method->returnType = Type::VOID;
        method->isPrivate = ($1 && strcmp($1, "private") == 0);
        if ($6) {
            method->body = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($6));
        } else {
            method->body = std::make_unique<BlockNode>();
        }
        $$ = method;
        if ($1) free($1);
        free($2);
    }
    ;

access_specifier:
    PRIVATE { $$ = strdup("private"); }
    | PUBLIC { $$ = strdup("public"); }
    | /* empty */ { $$ = nullptr; }
    ;

var_decl:
    CONST type_spec VAR ASSIGN expression SEMICOLON
    {
        auto* var = new VarDeclNode();
        var->line = yylineno;
        var->type = parseType($2);
        var->name = $3;
        var->isArray = false;
        var->isConst = true;
        var->arraySize = nullptr;
        if ($5) {
            var->initializer = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($5));
        }
        $$ = var;
        free($2);
        free($3);
    }
    | CONST type_spec VAR SEMICOLON
    {
        auto* var = new VarDeclNode();
        var->line = yylineno;
        var->type = parseType($2);
        var->name = $3;
        var->isArray = false;
        var->isConst = true;
        var->arraySize = nullptr;
        var->initializer = nullptr;
        $$ = var;
        free($2);
        free($3);
    }
    | type_spec VAR ASSIGN expression SEMICOLON
    {
        auto* var = new VarDeclNode();
        var->line = yylineno;
        var->type = parseType($1);
        var->name = $2;
        var->isArray = false;
        var->isConst = false;
        var->arraySize = nullptr;
        if ($4) {
            var->initializer = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($4));
        }
        $$ = var;
        free($1);
        free($2);
    }
    | type_spec VAR LBRACKET expression RBRACKET SEMICOLON
    {
        auto* var = new VarDeclNode();
        var->line = yylineno;
        var->type = parseType($1);
        var->name = $2;
        var->isArray = true;
        var->isConst = false;
        if ($4) {
            var->arraySize = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($4));
        }
        var->initializer = nullptr;
        $$ = var;
        free($1);
        free($2);
    }
    | type_spec VAR SEMICOLON
    {
        auto* var = new VarDeclNode();
        var->line = yylineno;
        var->type = parseType($1);
        var->name = $2;
        var->isArray = false;
        var->isConst = false;
        var->arraySize = nullptr;
        var->initializer = nullptr;
        $$ = var;
        free($1);
        free($2);
    }
    ;

var_assign:
    VAR LBRACKET expression RBRACKET ASSIGN expression SEMICOLON
    {
        auto* assign = new VarAssignNode();
        assign->line = yylineno;
        assign->name = $1;
        if ($6) {
            assign->value = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($6));
        }
        assign->assignOp = BinaryOp::ADD;
        $$ = assign;
        free($1);
        if ($3) delete static_cast<ExpressionNode*>($3);
    }
    | VAR ASSIGN expression SEMICOLON
    {
        auto* assign = new VarAssignNode();
        assign->line = yylineno;
        assign->name = $1;
        if ($3) {
            assign->value = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        }
        assign->assignOp = BinaryOp::ADD;
        $$ = assign;
        free($1);
    }
    | VAR PLUS_ASSIGN expression SEMICOLON
    {
        auto* assign = new VarAssignNode();
        assign->line = yylineno;
        assign->name = $1;
        if ($3) {
            assign->value = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        }
        assign->assignOp = BinaryOp::PLUS_ASSIGN;
        $$ = assign;
        free($1);
    }
    | VAR MINUS_ASSIGN expression SEMICOLON
    {
        auto* assign = new VarAssignNode();
        assign->line = yylineno;
        assign->name = $1;
        if ($3) {
            assign->value = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        }
        assign->assignOp = BinaryOp::MINUS_ASSIGN;
        $$ = assign;
        free($1);
    }
    | VAR STAR_ASSIGN expression SEMICOLON
    {
        auto* assign = new VarAssignNode();
        assign->line = yylineno;
        assign->name = $1;
        if ($3) {
            assign->value = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        }
        assign->assignOp = BinaryOp::STAR_ASSIGN;
        $$ = assign;
        free($1);
    }
    | VAR SLASH_ASSIGN expression SEMICOLON
    {
        auto* assign = new VarAssignNode();
        assign->line = yylineno;
        assign->name = $1;
        if ($3) {
            assign->value = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        }
        assign->assignOp = BinaryOp::SLASH_ASSIGN;
        $$ = assign;
        free($1);
    }
    ;

inc_dec_stmt:
    INCREMENT VAR SEMICOLON
    {
        auto* inc = new IncDecNode();
        inc->line = yylineno;
        inc->name = $2;
        inc->isIncrement = true;
        inc->isPrefix = true;
        $$ = inc;
        free($2);
    }
    | VAR INCREMENT SEMICOLON
    {
        auto* inc = new IncDecNode();
        inc->line = yylineno;
        inc->name = $1;
        inc->isIncrement = true;
        inc->isPrefix = false;
        $$ = inc;
        free($1);
    }
    | DECREMENT VAR SEMICOLON
    {
        auto* inc = new IncDecNode();
        inc->line = yylineno;
        inc->name = $2;
        inc->isIncrement = false;
        inc->isPrefix = true;
        $$ = inc;
        free($2);
    }
    | VAR DECREMENT SEMICOLON
    {
        auto* inc = new IncDecNode();
        inc->line = yylineno;
        inc->name = $1;
        inc->isIncrement = false;
        inc->isPrefix = false;
        $$ = inc;
        free($1);
    }
    ;

if_stmt:
    IF LPAREN expression RPAREN block
    {
        auto* ifNode = new IfNode();
        ifNode->line = yylineno;
        if ($3) {
            ifNode->condition = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        }
        if ($5) {
            ifNode->thenBlock = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($5));
        }
        ifNode->elseBlock = nullptr;
        ifNode->elseIf = nullptr;
        $$ = ifNode;
    }
    | IF LPAREN expression RPAREN block ELSE if_stmt
    {
        auto* ifNode = new IfNode();
        ifNode->line = yylineno;
        if ($3) {
            ifNode->condition = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        }
        if ($5) {
            ifNode->thenBlock = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($5));
        }
        ifNode->elseBlock = nullptr;
        if ($7) {
            auto* elseStmt = static_cast<StatementNode*>($7);
            if (auto* elseIf = dynamic_cast<IfNode*>(elseStmt)) {
                ifNode->elseIf = std::unique_ptr<IfNode>(elseIf);
            } else {
                delete elseStmt;
            }
        }
        $$ = ifNode;
    }
    | IF LPAREN expression RPAREN block ELSE block
    {
        auto* ifNode = new IfNode();
        ifNode->line = yylineno;
        if ($3) {
            ifNode->condition = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        }
        if ($5) {
            ifNode->thenBlock = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($5));
        }
        if ($7) {
            ifNode->elseBlock = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($7));
        }
        ifNode->elseIf = nullptr;
        $$ = ifNode;
    }
    ;


while_stmt:
    WHILE LPAREN expression RPAREN block
    {
        auto* whileNode = new WhileNode();
        whileNode->line = yylineno;
        if ($3) {
            whileNode->condition = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        }
        if ($5) {
            whileNode->body = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($5));
        }
        $$ = whileNode;
    }
    ;

do_while_stmt:
    DO block WHILE LPAREN expression RPAREN SEMICOLON
    {
        auto* doWhileNode = new DoWhileNode();
        doWhileNode->line = yylineno;
        if ($2) {
            doWhileNode->body = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($2));
        }
        if ($5) {
            doWhileNode->condition = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($5));
        }
        $$ = doWhileNode;
    }
    ;

break_stmt:
    BREAK SEMICOLON
    {
        auto* breakNode = new BreakNode();
        breakNode->line = yylineno;
        $$ = breakNode;
    }
    ;

continue_stmt:
    CONTINUE SEMICOLON
    {
        auto* continueNode = new ContinueNode();
        continueNode->line = yylineno;
        $$ = continueNode;
    }
    ;

switch_stmt:
    SWITCH LPAREN expression RPAREN LBRACE case_list RBRACE
    {
        auto* switchNode = new SwitchNode();
        switchNode->line = yylineno;
        if ($3) {
            switchNode->expression = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        }
        if ($6) {
            auto* cases = static_cast<std::vector<CaseNode*>*>($6);
            if (cases) {
                for (auto* c : *cases) {
                    switchNode->cases.push_back(std::unique_ptr<CaseNode>(c));
                }
                delete cases;
            }
        }
        switchNode->defaultCase = nullptr;
        $$ = switchNode;
    }
    | SWITCH LPAREN expression RPAREN LBRACE case_list default_case RBRACE
    {
        auto* switchNode = new SwitchNode();
        switchNode->line = yylineno;
        if ($3) {
            switchNode->expression = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        }
        if ($6) {
            auto* cases = static_cast<std::vector<CaseNode*>*>($6);
            if (cases) {
                for (auto* c : *cases) {
                    switchNode->cases.push_back(std::unique_ptr<CaseNode>(c));
                }
                delete cases;
            }
        }
        if ($7) {
            switchNode->defaultCase = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($7));
        }
        $$ = switchNode;
    }
    ;

case_list:
    case_list case_item
    {
        auto* list = static_cast<std::vector<CaseNode*>*>($1);
        if (!list) list = new std::vector<CaseNode*>();
        auto* caseNode = static_cast<CaseNode*>($2);
        if (caseNode) {
            list->push_back(caseNode);
        }
        $$ = list;
    }
    | case_item
    {
        auto* list = new std::vector<CaseNode*>();
        auto* caseNode = static_cast<CaseNode*>($1);
        if (caseNode) {
            list->push_back(caseNode);
        }
        $$ = list;
    }
    | /* empty */ { $$ = NULL; }
    ;

case_item:
    CASE expression COLON function_body
    {
        auto* caseNode = new CaseNode();
        caseNode->line = yylineno;
        if ($2) {
            caseNode->value = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($2));
        }
        if ($4) {
            caseNode->block = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($4));
        } else {
            caseNode->block = std::make_unique<BlockNode>();
        }
        $$ = caseNode;
    }
    ;

default_case:
    DEFAULT COLON function_body
    {
        if ($3) {
            $$ = static_cast<BlockNode*>($3);
        } else {
            $$ = new BlockNode();
        }
    }
    ;

for_stmt:
    FOR LPAREN var_decl expression SEMICOLON expression RPAREN block
    {
        auto* forNode = new ForNode();
        forNode->line = yylineno;
        if ($3) {
            forNode->init = std::unique_ptr<VarDeclNode>(static_cast<VarDeclNode*>($3));
        }
        if ($4) {
            forNode->condition = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($4));
        }
        if ($6) {
            forNode->increment = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($6));
        }
        if ($8) {
            forNode->body = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($8));
        }
        $$ = forNode;
    }
    | FOR LPAREN SEMICOLON expression SEMICOLON expression RPAREN block
    {
        auto* forNode = new ForNode();
        forNode->line = yylineno;
        forNode->init = nullptr;
        if ($4) {
            forNode->condition = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($4));
        }
        if ($6) {
            forNode->increment = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($6));
        }
        if ($8) {
            forNode->body = std::unique_ptr<BlockNode>(static_cast<BlockNode*>($8));
        }
        $$ = forNode;
    }
    ;

expression:
    expression PLUS expression
    {
        auto* bin = new BinaryExprNode();
        bin->op = BinaryOp::ADD;
        bin->left = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($1));
        bin->right = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        $$ = bin;
    }
    | expression MINUS expression
    {
        auto* bin = new BinaryExprNode();
        bin->op = BinaryOp::SUB;
        bin->left = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($1));
        bin->right = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        $$ = bin;
    }
    | expression STAR expression
    {
        auto* bin = new BinaryExprNode();
        bin->op = BinaryOp::MUL;
        bin->left = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($1));
        bin->right = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        $$ = bin;
    }
    | expression SLASH expression
    {
        auto* bin = new BinaryExprNode();
        bin->op = BinaryOp::DIV;
        bin->left = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($1));
        bin->right = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        $$ = bin;
    }
    | expression MOD expression
    {
        auto* bin = new BinaryExprNode();
        bin->op = BinaryOp::MOD;
        bin->left = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($1));
        bin->right = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        $$ = bin;
    }
    | expression EQ expression
    {
        auto* bin = new BinaryExprNode();
        bin->op = BinaryOp::EQ;
        bin->left = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($1));
        bin->right = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        $$ = bin;
    }
    | expression NE expression
    {
        auto* bin = new BinaryExprNode();
        bin->op = BinaryOp::NE;
        bin->left = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($1));
        bin->right = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        $$ = bin;
    }
    | expression LT expression
    {
        auto* bin = new BinaryExprNode();
        bin->op = BinaryOp::LT;
        bin->left = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($1));
        bin->right = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        $$ = bin;
    }
    | expression GT expression
    {
        auto* bin = new BinaryExprNode();
        bin->op = BinaryOp::GT;
        bin->left = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($1));
        bin->right = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        $$ = bin;
    }
    | expression LE expression
    {
        auto* bin = new BinaryExprNode();
        bin->op = BinaryOp::LE;
        bin->left = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($1));
        bin->right = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        $$ = bin;
    }
    | expression GE expression
    {
        auto* bin = new BinaryExprNode();
        bin->op = BinaryOp::GE;
        bin->left = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($1));
        bin->right = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        $$ = bin;
    }
    | expression AND expression
    {
        auto* bin = new BinaryExprNode();
        bin->op = BinaryOp::AND;
        bin->left = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($1));
        bin->right = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        $$ = bin;
    }
    | expression OR expression
    {
        auto* bin = new BinaryExprNode();
        bin->op = BinaryOp::OR;
        bin->left = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($1));
        bin->right = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        $$ = bin;
    }
    | NOT expression %prec NOT
    {
        auto* unary = new UnaryExprNode();
        unary->op = UnaryOp::NOT;
        unary->operand = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($2));
        $$ = unary;
    }
    | MINUS expression %prec NEG
    {
        auto* unary = new UnaryExprNode();
        unary->op = UnaryOp::NEG;
        unary->operand = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($2));
        $$ = unary;
    }
    | VAR LPAREN RPAREN
    {
        auto* call = new CallExprNode();
        call->line = yylineno;
        call->functionName = $1;
        $$ = call;
        free($1);
    }
    | VAR LPAREN expression_list RPAREN
    {
        auto* call = new CallExprNode();
        call->line = yylineno;
        call->functionName = $1;
        if ($3) {
            auto* list = static_cast<std::vector<ExpressionNode*>*>($3);
            for (auto* expr : *list) {
                call->arguments.push_back(std::unique_ptr<ExpressionNode>(expr));
            }
            delete list;
        }
        $$ = call;
        free($1);
    }
    | INTEGER
    {
        auto* lit = new LiteralNode();
        lit->line = yylineno;
        lit->literalType = Type::INT;
        lit->intValue = $1;
        $$ = lit;
    }
    | DOUBLE
    {
        auto* lit = new LiteralNode();
        lit->line = yylineno;
        lit->literalType = Type::DOUBLE;
        lit->doubleValue = $1;
        $$ = lit;
    }
    | FLOAT
    {
        auto* lit = new LiteralNode();
        lit->line = yylineno;
        lit->literalType = Type::FLOAT;
        lit->floatValue = $1;
        $$ = lit;
    }
    | BOOLEAN
    {
        auto* lit = new LiteralNode();
        lit->line = yylineno;
        lit->literalType = Type::BOOL;
        lit->boolValue = $1;
        $$ = lit;
    }
    | STRING
    {
        auto* lit = new LiteralNode();
        lit->line = yylineno;
        lit->literalType = Type::STRING;
        new (&lit->stringValue) std::string($1);
        $$ = lit;
        free($1);
    }
    | VAR
    {
        auto* var = new VarNode();
        var->line = yylineno;
        var->name = $1;
        $$ = var;
        free($1);
    }
    | VAR LBRACKET expression RBRACKET
    {
        auto* arr = new ArrayAccessNode();
        arr->line = yylineno;
        arr->arrayName = $1;
        if ($3) {
            arr->index = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        }
        $$ = arr;
        free($1);
    }
    | INCREMENT VAR
    {
        auto* inc = new IncDecExprNode();
        inc->line = yylineno;
        inc->name = $2;
        inc->isIncrement = true;
        inc->isPrefix = true;
        $$ = inc;
        free($2);
    }
    | VAR INCREMENT
    {
        auto* inc = new IncDecExprNode();
        inc->line = yylineno;
        inc->name = $1;
        inc->isIncrement = true;
        inc->isPrefix = false;
        $$ = inc;
        free($1);
    }
    | DECREMENT VAR
    {
        auto* inc = new IncDecExprNode();
        inc->line = yylineno;
        inc->name = $2;
        inc->isIncrement = false;
        inc->isPrefix = true;
        $$ = inc;
        free($2);
    }
    | VAR DECREMENT
    {
        auto* inc = new IncDecExprNode();
        inc->line = yylineno;
        inc->name = $1;
        inc->isIncrement = false;
        inc->isPrefix = false;
        $$ = inc;
        free($1);
    }
    | expression QUESTION expression COLON expression
    {
        auto* ternary = new TernaryExprNode();
        ternary->line = yylineno;
        if ($1) {
            ternary->condition = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($1));
        }
        if ($3) {
            ternary->trueExpr = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($3));
        }
        if ($5) {
            ternary->falseExpr = std::unique_ptr<ExpressionNode>(static_cast<ExpressionNode*>($5));
        }
        $$ = ternary;
    }
    | LPAREN expression RPAREN { $$ = $2; }
    ;

expression_list:
    expression_list COMMA expression
    {
        auto* list = static_cast<std::vector<ExpressionNode*>*>($1);
        if (!list) list = new std::vector<ExpressionNode*>();
        if ($3) {
            list->push_back(static_cast<ExpressionNode*>($3));
        }
        $$ = list;
    }
    | expression
    {
        auto* list = new std::vector<ExpressionNode*>();
        if ($1) {
            list->push_back(static_cast<ExpressionNode*>($1));
        }
        $$ = list;
    }
    ;

%%

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input.sl> <output> [options]" << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "  -o <file>           Output executable" << std::endl;
        std::cerr << "  -shared             Generate shared library (.so)" << std::endl;
        std::cerr << "  -static             Generate static library (.a)" << std::endl;
        std::cerr << "  -c <file>           Keep intermediate C file" << std::endl;
        return 1;
    }

    enum class OutputType { EXECUTABLE, SHARED_LIB, STATIC_LIB };
    OutputType outputType = OutputType::EXECUTABLE;
    std::string inputFile;
    std::string outputFile;
    std::string intermediateCFile;
    bool keepIntermediate = false;

    int i = 1;
    inputFile = argv[i++];

    while (i < argc) {
        std::string arg = argv[i++];
        if (arg == "-o" && i < argc) {
            outputFile = argv[i++];
            outputType = OutputType::EXECUTABLE;
        } else if (arg == "-shared") {
            outputType = OutputType::SHARED_LIB;
        } else if (arg == "-static") {
            outputType = OutputType::STATIC_LIB;
        } else if (arg == "-c" && i < argc) {
            intermediateCFile = argv[i++];
            keepIntermediate = true;
        } else if (outputFile.empty()) {
            outputFile = arg;
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }

    if (outputFile.empty()) {
        std::cerr << "Output file not specified" << std::endl;
        return 1;
    }

    if (intermediateCFile.empty()) {
        intermediateCFile = "/tmp/sl_temp_" + std::to_string(getpid()) + ".c";
    }

    inputFile = argv[1];
    std::string outputExecutable;
    std::string tempCFile;

    if (argc == 3) {
        outputExecutable = argv[2];
        tempCFile = "/tmp/sl_temp_" + std::to_string(getpid()) + ".c";
    } else if (argc == 4) {
        if (std::string(argv[2]) == "-o") {
            outputExecutable = argv[3];
            tempCFile = "/tmp/sl_temp_" + std::to_string(getpid()) + ".c";
        } else {
            tempCFile = argv[2];
            outputExecutable = argv[3];
        }
    }

    FILE* file = fopen(inputFile.c_str(), "r");
    if (!file) {
        std::cerr << "Cannot open file: " << inputFile << std::endl;
        return 1;
    }
    yyin = file;

    yyparse();
    fclose(yyin);

    if (!programRoot) {
        std::cerr << "Parse failed" << std::endl;
        return 1;
    }

    SemanticAnalyzer semantic;
    if (!semantic.analyze(programRoot.get())) {
        std::cerr << "Semantic errors:" << std::endl;
        for (const auto& error : semantic.getErrors()) {
            std::cerr << "  " << error << std::endl;
        }
        return 1;
    }

    std::ofstream cFileOutput(intermediateCFile);
    if (!cFileOutput) {
        std::cerr << "Cannot create intermediate C file: " << intermediateCFile << std::endl;
        return 1;
    }

    CodeGenerator generator(cFileOutput);
    if (outputType == OutputType::SHARED_LIB || outputType == OutputType::STATIC_LIB) {
        generator.setLibraryMode(true);
    }
    generator.generate(programRoot.get());
    cFileOutput.close();

    std::string gccCommand;
    std::string finalOutput = outputFile;

    switch (outputType) {
        case OutputType::EXECUTABLE:
            gccCommand = "gcc " + intermediateCFile + " -o " + finalOutput;
            break;
        case OutputType::SHARED_LIB:
            if (finalOutput.find(".so") == std::string::npos) {
                finalOutput += ".so";
            }
            gccCommand = "gcc -shared -fPIC " + intermediateCFile + " -o " + finalOutput;
            break;
        case OutputType::STATIC_LIB:
            std::string objFile = intermediateCFile.substr(0, intermediateCFile.find_last_of('.')) + ".o";
            gccCommand = "gcc -c " + intermediateCFile + " -o " + objFile +
                        " && ar rcs " + finalOutput + " " + objFile;
            break;
    }

    int gccResult = system(gccCommand.c_str());

    if (!keepIntermediate) {
        remove(intermediateCFile.c_str());
        if (outputType == OutputType::STATIC_LIB) {
            std::string objFile = intermediateCFile.substr(0, intermediateCFile.find_last_of('.')) + ".o";
            remove(objFile.c_str());
        }
    }

    if (gccResult != 0) {
        std::cerr << "GCC compilation failed" << std::endl;
        return 1;
    }

    std::string outputTypeStr;
    switch (outputType) {
        case OutputType::EXECUTABLE: outputTypeStr = "executable"; break;
        case OutputType::SHARED_LIB: outputTypeStr = "shared library"; break;
        case OutputType::STATIC_LIB: outputTypeStr = "static library"; break;
    }

    std::cout << "Successfully compiled " << inputFile << " to " << outputTypeStr << " " << finalOutput << std::endl;
    return 0;
}
