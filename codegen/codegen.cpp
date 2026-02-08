#include "codegen.h"
#include <sstream>
#include <iomanip>

void CodeGenerator::indent() {
    for (int i = 0; i < indentLevel; ++i) {
        out << "    ";
    }
}

void CodeGenerator::print(const std::string& str) {
    out << str;
}

void CodeGenerator::printLine(const std::string& str) {
    indent();
    out << str << "\n";
}

std::string CodeGenerator::typeToCType(Type type) {
    switch (type) {
        case Type::INT: return "int";
        case Type::DOUBLE: return "double";
        case Type::FLOAT: return "float";
        case Type::STRING: return "char*";
        case Type::BOOL: return "int";
        case Type::VOID: return "void";
        default: return "void";
    }
}

std::string CodeGenerator::escapeString(const std::string& str) {
    std::stringstream ss;
    for (char c : str) {
        if (c == '"') ss << "\\\"";
        else if (c == '\\') ss << "\\\\";
        else if (c == '\n') ss << "\\n";
        else if (c == '\t') ss << "\\t";
        else ss << c;
    }
    return ss.str();
}

void CodeGenerator::generate(ProgramNode* program) {
    printLine("#include <stdio.h>");
    printLine("#include <stdlib.h>");
    printLine("#include <string.h>");
    print("");

    program->accept(this);
}

void CodeGenerator::visit(ProgramNode* node) {
    for (auto& global : node->globals) {
        global->accept(this);
    }

    for (auto& func : node->functions) {
        func->accept(this);
    }
}

void CodeGenerator::visit(DirectiveNode* node) {
}

void CodeGenerator::visit(FunctionNode* node) {
    std::stringstream ss;
    ss << typeToCType(node->returnType) << " " << node->name << "(";
    
    for (size_t i = 0; i < node->parameters.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << typeToCType(node->parameters[i].second) << " " << node->parameters[i].first;
    }
    
    ss << ") {";
    printLine(ss.str());
    
    currentFunctionReturnType = typeToString(node->returnType);
    indentLevel++;
    
    if (node->body) {
        node->body->accept(this);
    }
    
    indentLevel--;
    printLine("}");
    print("");
}

void CodeGenerator::visit(BlockNode* node) {
    for (auto& stmt : node->statements) {
        stmt->accept(this);
    }
}

void CodeGenerator::visit(VarDeclNode* node) {
    indent();
    std::stringstream ss;
    if (node->isConst) {
        ss << "const ";
    }
    ss << typeToCType(node->type) << " " << node->name;
    
    if (node->isArray) {
        ss << "[";
        if (node->arraySize) {
            print(ss.str());
            node->arraySize->accept(this);
            print("]");
        } else {
            ss << "]";
            print(ss.str());
        }
    }
    
    if (!node->isArray) {
        print(ss.str());
    }
    
    if (node->initializer) {
        if (!node->isArray) {
            print(" = ");
        } else {
            print(" = ");
        }
        node->initializer->accept(this);
        print(";\n");
    } else {
        if (!node->isArray) {
            print(";\n");
        } else {
            print(";\n");
        }
    }
}

void CodeGenerator::visit(VarAssignNode* node) {
    indent();
    std::stringstream ss;
    
    if (node->assignOp == BinaryOp::PLUS_ASSIGN) {
        ss << node->name << " += ";
    } else if (node->assignOp == BinaryOp::MINUS_ASSIGN) {
        ss << node->name << " -= ";
    } else if (node->assignOp == BinaryOp::STAR_ASSIGN) {
        ss << node->name << " *= ";
    } else if (node->assignOp == BinaryOp::SLASH_ASSIGN) {
        ss << node->name << " /= ";
    } else {
        ss << node->name << " = ";
    }
    
    print(ss.str());
    
    if (node->value) {
        node->value->accept(this);
    }
    
    print(";\n");
}

void CodeGenerator::visit(IncDecNode* node) {
    indent();
    std::stringstream ss;
    
    if (node->isPrefix) {
        ss << (node->isIncrement ? "++" : "--") << node->name;
    } else {
        ss << node->name << (node->isIncrement ? "++" : "--");
    }
    ss << ";";
    printLine(ss.str());
}

void CodeGenerator::visit(ReturnNode* node) {
    indent();
    std::stringstream ss;
    ss << "return";
    
    if (node->value) {
        ss << " ";
        print(ss.str());
        node->value->accept(this);
        print(";\n");
    } else {
        ss << ";\n";
        printLine(ss.str());
    }
}

void CodeGenerator::visit(IfNode* node) {
    indent();
    print("if (");
    if (node->condition) {
        node->condition->accept(this);
    }
    print(") {\n");
    
    indentLevel++;
    if (node->thenBlock) {
        node->thenBlock->accept(this);
    }
    indentLevel--;
    
    indent();
    print("}");
    
    if (node->elseIf) {
        print(" else ");
        node->elseIf->accept(this);
    } else if (node->elseBlock) {
        print(" else {\n");
        indentLevel++;
        node->elseBlock->accept(this);
        indentLevel--;
        indent();
        print("}");
    }
    print("\n");
}

void CodeGenerator::visit(WhileNode* node) {
    indent();
    print("while (");
    if (node->condition) {
        node->condition->accept(this);
    }
    print(") {\n");
    
    indentLevel++;
    if (node->body) {
        node->body->accept(this);
    }
    indentLevel--;
    
    indent();
    print("}\n");
}

void CodeGenerator::visit(ForNode* node) {
    indent();
    print("for (");
    
    if (node->init) {
        std::stringstream ss;
        ss << typeToCType(node->init->type) << " " << node->init->name;
        if (node->init->initializer) {
            ss << " = ";
            print(ss.str());
            node->init->initializer->accept(this);
        } else {
            print(ss.str());
        }
    }
    print("; ");
    
    if (node->condition) {
        node->condition->accept(this);
    }
    print("; ");
    
    if (node->increment) {
        node->increment->accept(this);
    }
    
    print(") {\n");
    
    indentLevel++;
    if (node->body) {
        node->body->accept(this);
    }
    indentLevel--;
    
    indent();
    print("}\n");
}

void CodeGenerator::visit(BinaryExprNode* node) {
    bool needParens = false;
    
    if (node->left) {
        node->left->accept(this);
    }
    
    std::string op;
    switch (node->op) {
        case BinaryOp::ADD: op = " + "; break;
        case BinaryOp::SUB: op = " - "; break;
        case BinaryOp::MUL: op = " * "; break;
        case BinaryOp::DIV: op = " / "; break;
        case BinaryOp::MOD: op = " % "; break;
        case BinaryOp::EQ: op = " == "; break;
        case BinaryOp::NE: op = " != "; break;
        case BinaryOp::LT: op = " < "; break;
        case BinaryOp::GT: op = " > "; break;
        case BinaryOp::LE: op = " <= "; break;
        case BinaryOp::GE: op = " >= "; break;
        case BinaryOp::AND: op = " && "; break;
        case BinaryOp::OR: op = " || "; break;
        default: op = " ? "; break;
    }
    print(op);
    
    if (node->right) {
        node->right->accept(this);
    }
}

void CodeGenerator::visit(UnaryExprNode* node) {
    std::string op;
    switch (node->op) {
        case UnaryOp::NOT: op = "!"; break;
        case UnaryOp::NEG: op = "-"; break;
    }
    print(op);
    print("(");
    if (node->operand) {
        node->operand->accept(this);
    }
    print(")");
}

void CodeGenerator::visit(CallExprNode* node) {
    print(node->functionName);
    print("(");
    for (size_t i = 0; i < node->arguments.size(); ++i) {
        if (i > 0) print(", ");
        node->arguments[i]->accept(this);
    }
    print(")");
}

void CodeGenerator::visit(LiteralNode* node) {
    switch (node->literalType) {
        case Type::INT:
            print(std::to_string(node->intValue));
            break;
        case Type::DOUBLE:
            print(std::to_string(node->doubleValue));
            break;
        case Type::FLOAT:
            print(std::to_string(node->floatValue) + "f");
            break;
        case Type::BOOL:
            print(node->boolValue ? "1" : "0");
            break;
        case Type::STRING:
            print("\"" + escapeString(node->stringValue) + "\"");
            break;
        default:
            print("0");
    }
}

void CodeGenerator::visit(VarNode* node) {
    print(node->name);
}

void CodeGenerator::visit(ArrayAccessNode* node) {
    print(node->arrayName);
    print("[");
    if (node->index) {
        node->index->accept(this);
    }
    print("]");
}

void CodeGenerator::visit(DoWhileNode* node) {
    indent();
    print("do {\n");
    indentLevel++;
    if (node->body) {
        node->body->accept(this);
    }
    indentLevel--;
    indent();
    print("} while (");
    if (node->condition) {
        node->condition->accept(this);
    }
    print(");\n");
}

void CodeGenerator::visit(BreakNode* node) {
    indent();
    print("break;\n");
}

void CodeGenerator::visit(ContinueNode* node) {
    indent();
    print("continue;\n");
}

void CodeGenerator::visit(SwitchNode* node) {
    indent();
    print("switch (");
    if (node->expression) {
        node->expression->accept(this);
    }
    print(") {\n");
    indentLevel++;
    for (auto& caseNode : node->cases) {
        caseNode->accept(this);
    }
    if (node->defaultCase) {
        indent();
        print("default:\n");
        indentLevel++;
        node->defaultCase->accept(this);
        indentLevel--;
    }
    indentLevel--;
    indent();
    print("}\n");
}

void CodeGenerator::visit(CaseNode* node) {
    indent();
    print("case ");
    if (node->value) {
        node->value->accept(this);
    }
    print(":\n");
    indentLevel++;
    if (node->block) {
        node->block->accept(this);
    }
    indentLevel--;
}

void CodeGenerator::visit(IncDecExprNode* node) {
    if (node->isIncrement) {
        if (node->isPrefix) {
            print("++");
            print(node->name);
        } else {
            print(node->name);
            print("++");
        }
    } else {
        if (node->isPrefix) {
            print("--");
            print(node->name);
        } else {
            print(node->name);
            print("--");
        }
    }
}

void CodeGenerator::visit(TernaryExprNode* node) {
    bool needParen = true;
    if (needParen) print("(");
    if (node->condition) {
        node->condition->accept(this);
    }
    print(" ? ");
    if (node->trueExpr) {
        node->trueExpr->accept(this);
    }
    print(" : ");
    if (node->falseExpr) {
        node->falseExpr->accept(this);
    }
    if (needParen) print(")");
}
