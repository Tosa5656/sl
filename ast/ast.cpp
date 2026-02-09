#include "ast.h"
#include "visitors.h"

void ProgramNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void DirectiveNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void FunctionNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void TemplateNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void ClassNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void MethodNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void ConstructorNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void BlockNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void StatementNode::accept(ASTVisitor* visitor) {
}

void VarDeclNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void VarAssignNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void ReturnNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void IfNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void WhileNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void ForNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void ExpressionNode::accept(ASTVisitor* visitor) {
}

void BinaryExprNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void UnaryExprNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void CallExprNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void LiteralNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void VarNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void ArrayAccessNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void IncDecNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void DoWhileNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void BreakNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void ContinueNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void SwitchNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void CaseNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void IncDecExprNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

void TernaryExprNode::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}
Type stringToType(const std::string& typeStr) {
    if (typeStr == "int") return Type::INT;
    if (typeStr == "double") return Type::DOUBLE;
    if (typeStr == "float") return Type::FLOAT;
    if (typeStr == "string") return Type::STRING;
    if (typeStr == "bool") return Type::BOOL;
    if (typeStr == "void") return Type::VOID;
    return Type::VOID;
}

std::string typeToString(Type type) {
    switch (type) {
        case Type::INT: return "int";
        case Type::DOUBLE: return "double";
        case Type::FLOAT: return "float";
        case Type::STRING: return "string";
        case Type::BOOL: return "bool";
        case Type::VOID: return "void";
        default: return "void";
    }
}

std::string binaryOpToString(BinaryOp op) {
    switch (op) {
        case BinaryOp::ADD: return "+";
        case BinaryOp::SUB: return "-";
        case BinaryOp::MUL: return "*";
        case BinaryOp::DIV: return "/";
        case BinaryOp::MOD: return "%";
        case BinaryOp::EQ: return "==";
        case BinaryOp::NE: return "!=";
        case BinaryOp::LT: return "<";
        case BinaryOp::GT: return ">";
        case BinaryOp::LE: return "<=";
        case BinaryOp::GE: return ">=";
        case BinaryOp::AND: return "&&";
        case BinaryOp::OR: return "||";
        case BinaryOp::PLUS_ASSIGN: return "+=";
        case BinaryOp::MINUS_ASSIGN: return "-=";
        case BinaryOp::STAR_ASSIGN: return "*=";
        case BinaryOp::SLASH_ASSIGN: return "/=";
        default: return "?";
    }
}

std::string unaryOpToString(UnaryOp op) {
    switch (op) {
        case UnaryOp::NOT: return "!";
        case UnaryOp::NEG: return "-";
        default: return "?";
    }
}
