#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>

enum class Type {
    INT,
    DOUBLE,
    FLOAT,
    STRING,
    BOOL,
    VOID
};

enum class BinaryOp {
    ADD, SUB, MUL, DIV, MOD,
    EQ, NE, LT, GT, LE, GE,
    AND, OR,
    PLUS_ASSIGN, MINUS_ASSIGN, STAR_ASSIGN, SLASH_ASSIGN
};

enum class UnaryOp {
    NOT, NEG
};

class ASTNode;
class ProgramNode;
class DirectiveNode;
class FunctionNode;
class StatementNode;
class ExpressionNode;
class VarDeclNode;
class VarAssignNode;
class ReturnNode;
class IfNode;
class WhileNode;
class DoWhileNode;
class ForNode;
class BlockNode;
class BinaryExprNode;
class UnaryExprNode;
class CallExprNode;
class LiteralNode;
class VarNode;
class IncDecExprNode;
class BreakNode;
class ContinueNode;
class SwitchNode;
class CaseNode;
class TernaryExprNode;

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(class ASTVisitor* visitor) = 0;
    int line = 0;
};

class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> directives;
    std::vector<std::unique_ptr<FunctionNode>> functions;
    std::vector<std::unique_ptr<VarDeclNode>> globals;

    void accept(ASTVisitor* visitor) override;
};

class DirectiveNode : public ASTNode {
public:
    std::string directive;
    std::string filename;

    void accept(ASTVisitor* visitor) override;
};

class FunctionNode : public ASTNode {
public:
    std::string name;
    Type returnType;
    std::vector<std::pair<std::string, Type>> parameters;
    std::unique_ptr<BlockNode> body;

    void accept(ASTVisitor* visitor) override;
};

class BlockNode : public ASTNode {
public:
    std::vector<std::unique_ptr<StatementNode>> statements;

    void accept(ASTVisitor* visitor) override;
};

class StatementNode : public ASTNode {
public:
    void accept(ASTVisitor* visitor) override;
};

class VarDeclNode : public StatementNode {
public:
    Type type;
    std::string name;
    bool isArray;
    bool isConst;
    std::unique_ptr<ExpressionNode> arraySize;
    std::unique_ptr<ExpressionNode> initializer;

    void accept(ASTVisitor* visitor) override;
};

class VarAssignNode : public StatementNode {
public:
    std::string name;
    std::unique_ptr<ExpressionNode> value;
    BinaryOp assignOp;

    void accept(ASTVisitor* visitor) override;
};

class IncDecNode : public StatementNode {
public:
    std::string name;
    bool isIncrement;
    bool isPrefix;

    void accept(ASTVisitor* visitor) override;
};

class ReturnNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> value;

    void accept(ASTVisitor* visitor) override;
};

class IfNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BlockNode> thenBlock;
    std::unique_ptr<BlockNode> elseBlock;
    std::unique_ptr<IfNode> elseIf;

    void accept(ASTVisitor* visitor) override;
};

class WhileNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<BlockNode> body;

    void accept(ASTVisitor* visitor) override;
};

class DoWhileNode : public StatementNode {
public:
    std::unique_ptr<BlockNode> body;
    std::unique_ptr<ExpressionNode> condition;

    void accept(ASTVisitor* visitor) override;
};

class BreakNode : public StatementNode {
public:
    void accept(ASTVisitor* visitor) override;
};

class ContinueNode : public StatementNode {
public:
    void accept(ASTVisitor* visitor) override;
};

class CaseNode : public ASTNode {
public:
    std::unique_ptr<ExpressionNode> value;
    std::unique_ptr<BlockNode> block;

    void accept(ASTVisitor* visitor) override;
};

class SwitchNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> expression;
    std::vector<std::unique_ptr<CaseNode>> cases;
    std::unique_ptr<BlockNode> defaultCase;

    void accept(ASTVisitor* visitor) override;
};

class ForNode : public StatementNode {
public:
    std::unique_ptr<VarDeclNode> init;
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<ExpressionNode> increment;
    std::unique_ptr<BlockNode> body;

    void accept(ASTVisitor* visitor) override;
};

class ExpressionNode : public ASTNode {
public:
    Type type;
    void accept(ASTVisitor* visitor) override;
};

class BinaryExprNode : public ExpressionNode {
public:
    BinaryOp op;
    std::unique_ptr<ExpressionNode> left;
    std::unique_ptr<ExpressionNode> right;

    void accept(ASTVisitor* visitor) override;
};

class UnaryExprNode : public ExpressionNode {
public:
    UnaryOp op;
    std::unique_ptr<ExpressionNode> operand;

    void accept(ASTVisitor* visitor) override;
};

class CallExprNode : public ExpressionNode {
public:
    std::string functionName;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;

    void accept(ASTVisitor* visitor) override;
};

class LiteralNode : public ExpressionNode {
public:
    Type literalType;
    union {
        int intValue;
        double doubleValue;
        float floatValue;
        bool boolValue;
    };
    std::string stringValue;

    void accept(ASTVisitor* visitor) override;

    LiteralNode() : literalType(Type::INT), intValue(0) {
        new (&stringValue) std::string();
    }
    ~LiteralNode() {
        if (literalType == Type::STRING) {
            stringValue.~basic_string();
        }
    }
};

class VarNode : public ExpressionNode {
public:
    std::string name;

    void accept(ASTVisitor* visitor) override;
};

class IncDecExprNode : public ExpressionNode {
public:
    std::string name;
    bool isIncrement;
    bool isPrefix;

    void accept(ASTVisitor* visitor) override;
};

class TernaryExprNode : public ExpressionNode {
public:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<ExpressionNode> trueExpr;
    std::unique_ptr<ExpressionNode> falseExpr;

    void accept(ASTVisitor* visitor) override;
};

class ArrayAccessNode : public ExpressionNode {
public:
    std::string arrayName;
    std::unique_ptr<ExpressionNode> index;

    void accept(ASTVisitor* visitor) override;
};

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visit(ProgramNode* node) = 0;
    virtual void visit(DirectiveNode* node) = 0;
    virtual void visit(FunctionNode* node) = 0;
    virtual void visit(BlockNode* node) = 0;
    virtual void visit(VarDeclNode* node) = 0;
    virtual void visit(VarAssignNode* node) = 0;
    virtual void visit(ReturnNode* node) = 0;
    virtual void visit(IfNode* node) = 0;
    virtual void visit(WhileNode* node) = 0;
    virtual void visit(ForNode* node) = 0;
    virtual void visit(BinaryExprNode* node) = 0;
    virtual void visit(UnaryExprNode* node) = 0;
    virtual void visit(CallExprNode* node) = 0;
    virtual void visit(LiteralNode* node) = 0;
    virtual void visit(VarNode* node) = 0;
    virtual void visit(ArrayAccessNode* node) = 0;
    virtual void visit(IncDecNode* node) = 0;
    virtual void visit(IncDecExprNode* node) = 0;
    virtual void visit(DoWhileNode* node) = 0;
    virtual void visit(BreakNode* node) = 0;
    virtual void visit(ContinueNode* node) = 0;
    virtual void visit(SwitchNode* node) = 0;
    virtual void visit(CaseNode* node) = 0;
    virtual void visit(TernaryExprNode* node) = 0;
};

Type stringToType(const std::string& typeStr);
std::string typeToString(Type type);
std::string binaryOpToString(BinaryOp op);
std::string unaryOpToString(UnaryOp op);

#endif
