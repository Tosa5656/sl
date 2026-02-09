#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <string>
#include <map>
#include <vector>
#include "../ast/ast.h"

struct Symbol {
    std::string name;
    Type type;
    bool isFunction;
    std::vector<Type> paramTypes; // for functions
    Type returnType; // for functions
};

class SemanticAnalyzer : public ASTVisitor {
private:
    std::vector<std::map<std::string, Symbol>> scopes;
    std::map<std::string, Symbol> functions;
    std::vector<std::string> errors;
    
    void enterScope();
    void exitScope();
    void declareSymbol(const std::string& name, Type type);
    Symbol* lookupSymbol(const std::string& name);
    Symbol* lookupFunction(const std::string& name);
    Type inferType(ExpressionNode* expr);
    bool checkType(Type expected, Type actual, const std::string& context);
    
public:
    SemanticAnalyzer() { enterScope(); }
    ~SemanticAnalyzer() = default;
    
    bool analyze(ProgramNode* program);
    const std::vector<std::string>& getErrors() const { return errors; }
    
    // Visitor methods
    void visit(ProgramNode* node) override;
    void visit(DirectiveNode* node) override;
    void visit(FunctionNode* node) override;
    void visit(TemplateNode* node) override;
    void visit(ClassNode* node) override;
    void visit(MethodNode* node) override;
    void visit(ConstructorNode* node) override;
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

#endif // SEMANTIC_H
