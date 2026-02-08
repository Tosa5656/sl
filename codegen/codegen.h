#ifndef CODEGEN_H
#define CODEGEN_H

#include <iostream>
#include <string>
#include "../ast/ast.h"

class CodeGenerator : public ASTVisitor {
private:
    std::ostream& out;
    int indentLevel;
    std::string currentFunctionReturnType;
    bool libraryMode;

    void indent();
    void print(const std::string& str);
    void printLine(const std::string& str);
    std::string typeToCType(Type type);
    std::string escapeString(const std::string& str);

public:
    CodeGenerator(std::ostream& output) : out(output), indentLevel(0), libraryMode(false) {}
    ~CodeGenerator() = default;

    void setLibraryMode(bool mode) { libraryMode = mode; }
    void generate(ProgramNode* program);
    
    // Visitor methods
    void visit(ProgramNode* node) override;
    void visit(DirectiveNode* node) override;
    void visit(FunctionNode* node) override;
    void visit(BlockNode* node) override;
    void visit(VarDeclNode* node) override;
    void visit(VarAssignNode* node) override;
    void visit(ReturnNode* node) override;
    void visit(IfNode* node) override;
    void visit(WhileNode* node) override;
    void visit(ForNode* node) override;
    void visit(BinaryExprNode* node) override;
    void visit(UnaryExprNode* node) override;
    void visit(CallExprNode* node) override;
    void visit(LiteralNode* node) override;
    void visit(VarNode* node) override;
    void visit(ArrayAccessNode* node) override;
    void visit(IncDecNode* node) override;
    void visit(IncDecExprNode* node) override;
    void visit(DoWhileNode* node) override;
    void visit(BreakNode* node) override;
    void visit(ContinueNode* node) override;
    void visit(SwitchNode* node) override;
    void visit(CaseNode* node) override;
    void visit(TernaryExprNode* node) override;
};

#endif