#include "semantic.h"
#include <sstream>
extern int yylineno;

void SemanticAnalyzer::enterScope() {
    scopes.push_back(std::map<std::string, Symbol>());
}

void SemanticAnalyzer::exitScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

void SemanticAnalyzer::declareSymbol(const std::string& name, Type type) {
    if (!scopes.empty()) {
        if (scopes.back().find(name) != scopes.back().end()) {
            std::stringstream ss;
            ss << "Line " << yylineno << ": Variable '" << name << "' already declared in this scope";
            errors.push_back(ss.str());
        } else {
            Symbol sym;
            sym.name = name;
            sym.type = type;
            sym.isFunction = false;
            scopes.back()[name] = sym;
        }
    }
}

Symbol* SemanticAnalyzer::lookupSymbol(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &found->second;
        }
    }
    return nullptr;
}

Symbol* SemanticAnalyzer::lookupFunction(const std::string& name) {
    auto found = functions.find(name);
    if (found != functions.end()) {
        return &found->second;
    }
    return nullptr;
}

Type SemanticAnalyzer::inferType(ExpressionNode* expr) {
    if (!expr) return Type::VOID;
    
    if (auto* lit = dynamic_cast<LiteralNode*>(expr)) {
        return lit->literalType;
    } else if (auto* var = dynamic_cast<VarNode*>(expr)) {
        Symbol* sym = lookupSymbol(var->name);
        if (sym) {
            return sym->type;
        }
        return Type::VOID;
    } else if (auto* call = dynamic_cast<CallExprNode*>(expr)) {
        Symbol* func = lookupFunction(call->functionName);
        if (func) {
            return func->returnType;
        }
        return Type::VOID;
    } else if (auto* bin = dynamic_cast<BinaryExprNode*>(expr)) {
        Type left = inferType(bin->left.get());
        Type right = inferType(bin->right.get());
        
        if (bin->op == BinaryOp::EQ || bin->op == BinaryOp::NE ||
            bin->op == BinaryOp::LT || bin->op == BinaryOp::GT ||
            bin->op == BinaryOp::LE || bin->op == BinaryOp::GE ||
            bin->op == BinaryOp::AND || bin->op == BinaryOp::OR) {
            return Type::BOOL;
        }
        
        if (left == right) return left;
        if ((left == Type::INT || left == Type::FLOAT || left == Type::DOUBLE) &&
            (right == Type::INT || right == Type::FLOAT || right == Type::DOUBLE)) {
            if (left == Type::DOUBLE || right == Type::DOUBLE) return Type::DOUBLE;
            if (left == Type::FLOAT || right == Type::FLOAT) return Type::FLOAT;
            return Type::INT;
        }
        
        return Type::VOID;
    } else if (auto* unary = dynamic_cast<UnaryExprNode*>(expr)) {
        if (unary->op == UnaryOp::NOT) {
            return Type::BOOL;
        }
        return inferType(unary->operand.get());
    }
    
    return Type::VOID;
}

bool SemanticAnalyzer::checkType(Type expected, Type actual, const std::string& context) {
    if (expected == actual) return true;
    if (expected == Type::VOID || actual == Type::VOID) return false;

    if ((expected == Type::INT || expected == Type::FLOAT || expected == Type::DOUBLE) &&
        (actual == Type::INT || actual == Type::FLOAT || actual == Type::DOUBLE)) {
        return true;
    }
    
    std::stringstream ss;
    ss << context << ": type mismatch, expected " << typeToString(expected)
       << " but got " << typeToString(actual);
    errors.push_back(ss.str());
    return false;
}

bool SemanticAnalyzer::analyze(ProgramNode* program) {
    if (!program) return false;
    program->accept(this);
    return errors.empty();
}

void SemanticAnalyzer::visit(ProgramNode* node) {
    for (auto& func : node->functions) {
        Symbol sym;
        sym.name = func->name;
        sym.type = func->returnType;
        sym.isFunction = true;
        sym.returnType = func->returnType;
        for (auto& param : func->parameters) {
            sym.paramTypes.push_back(param.second);
        }
        if (functions.find(func->name) != functions.end()) {
            std::stringstream ss;
            ss << "Function '" << func->name << "' already declared";
            errors.push_back(ss.str());
        } else {
            functions[func->name] = sym;
        }
    }

    for (auto& templ : node->templates) {
        templ->accept(this);
    }

    for (auto& cls : node->classes) {
        cls->accept(this);
    }

    for (auto& dir : node->directives) {
        dir->accept(this);
    }
    
    for (auto& global : node->globals) {
        global->accept(this);
    }
    
    for (auto& func : node->functions) {
        func->accept(this);
    }
}

void SemanticAnalyzer::visit(DirectiveNode* node) {
}

void SemanticAnalyzer::visit(FunctionNode* node) {
    enterScope();

    for (auto& param : node->parameters) {
        declareSymbol(param.first, param.second);
    }

    if (node->body) {
        node->body->accept(this);
    }

    exitScope();
}

void SemanticAnalyzer::visit(TemplateNode* node) {
    if (node->function) {
        node->function->accept(this);
    }
}

void SemanticAnalyzer::visit(ClassNode* node) {
    enterScope();

    if (node->constructor) {
        node->constructor->accept(this);
    }

    for (auto& method : node->methods) {
        method->accept(this);
    }

    exitScope();
}

void SemanticAnalyzer::visit(MethodNode* node) {
    enterScope();

    for (auto& param : node->parameters) {
        declareSymbol(param.first, param.second);
    }

    if (node->body) {
        node->body->accept(this);
    }

    exitScope();
}

void SemanticAnalyzer::visit(ConstructorNode* node) {
    enterScope();

    for (auto& param : node->parameters) {
        declareSymbol(param.first, param.second);
    }

    if (node->body) {
        node->body->accept(this);
    }

    exitScope();
}

void SemanticAnalyzer::visit(BlockNode* node) {
    enterScope();
    for (auto& stmt : node->statements) {
        stmt->accept(this);
    }
    exitScope();
}

void SemanticAnalyzer::visit(VarDeclNode* node) {
    declareSymbol(node->name, node->type);
    
    if (node->initializer) {
        node->initializer->accept(this);
        Type initType = inferType(node->initializer.get());
        checkType(node->type, initType, "Variable initialization");
    }
}

void SemanticAnalyzer::visit(VarAssignNode* node) {
    Symbol* sym = lookupSymbol(node->name);
    if (!sym) {
        std::stringstream ss;
        ss << "Undefined variable '" << node->name << "'";
        errors.push_back(ss.str());
        return;
    }
    
    if (node->value) {
        node->value->accept(this);
        Type valueType = inferType(node->value.get());
        checkType(sym->type, valueType, "Variable assignment");
    }
}

void SemanticAnalyzer::visit(ReturnNode* node) {
    if (node->value) {
        node->value->accept(this);
    }
}

void SemanticAnalyzer::visit(IfNode* node) {
    if (node->condition) {
        node->condition->accept(this);
        Type condType = inferType(node->condition.get());
        checkType(Type::BOOL, condType, "If condition");
    }
    
    if (node->thenBlock) {
        node->thenBlock->accept(this);
    }
    
    if (node->elseBlock) {
        node->elseBlock->accept(this);
    }
}

void SemanticAnalyzer::visit(WhileNode* node) {
    if (node->condition) {
        node->condition->accept(this);
        Type condType = inferType(node->condition.get());
        checkType(Type::BOOL, condType, "While condition");
    }
    
    if (node->body) {
        node->body->accept(this);
    }
}

void SemanticAnalyzer::visit(ForNode* node) {
    enterScope();
    
    if (node->init) {
        node->init->accept(this);
    }
    
    if (node->condition) {
        node->condition->accept(this);
        Type condType = inferType(node->condition.get());
        checkType(Type::BOOL, condType, "For condition");
    }
    
    if (node->increment) {
        node->increment->accept(this);
    }
    
    if (node->body) {
        node->body->accept(this);
    }
    
    exitScope();
}

void SemanticAnalyzer::visit(BinaryExprNode* node) {
    if (node->left) node->left->accept(this);
    if (node->right) node->right->accept(this);
    
    Type leftType = inferType(node->left.get());
    Type rightType = inferType(node->right.get());

    if (node->op == BinaryOp::EQ || node->op == BinaryOp::NE ||
        node->op == BinaryOp::LT || node->op == BinaryOp::GT ||
        node->op == BinaryOp::LE || node->op == BinaryOp::GE) {
    } else if (node->op == BinaryOp::AND || node->op == BinaryOp::OR) {
        checkType(Type::BOOL, leftType, "Left operand of logical operator");
        checkType(Type::BOOL, rightType, "Right operand of logical operator");
    } else {
        if (leftType == Type::STRING || rightType == Type::STRING) {
            if (node->op != BinaryOp::ADD) {
                errors.push_back("String type only supports addition operator");
            }
        }
    }
}

void SemanticAnalyzer::visit(UnaryExprNode* node) {
    if (node->operand) {
        node->operand->accept(this);
    }
    
    if (node->op == UnaryOp::NOT) {
        Type opType = inferType(node->operand.get());
        checkType(Type::BOOL, opType, "Not operator");
    }
}

void SemanticAnalyzer::visit(CallExprNode* node) {
    Symbol* func = lookupFunction(node->functionName);
    if (!func) {
        std::stringstream ss;
        ss << "Undefined function '" << node->functionName << "'";
        errors.push_back(ss.str());
        return;
    }
    
    if (func->paramTypes.size() != node->arguments.size()) {
        std::stringstream ss;
        ss << "Function '" << node->functionName << "' expects "
           << func->paramTypes.size() << " arguments but got "
           << node->arguments.size();
        errors.push_back(ss.str());
    } else {
        for (size_t i = 0; i < node->arguments.size(); ++i) {
            node->arguments[i]->accept(this);
            Type argType = inferType(node->arguments[i].get());
            checkType(func->paramTypes[i], argType, "Function argument");
        }
    }
}

void SemanticAnalyzer::visit(LiteralNode* node) {
}

void SemanticAnalyzer::visit(VarNode* node) {
    Symbol* sym = lookupSymbol(node->name);
    if (!sym) {
        std::stringstream ss;
        ss << "Line " << node->line << ": Undefined variable '" << node->name << "'";
        errors.push_back(ss.str());
    }
}

void SemanticAnalyzer::visit(ArrayAccessNode* node) {
    Symbol* sym = lookupSymbol(node->arrayName);
    if (!sym) {
        std::stringstream ss;
        ss << "Line " << node->line << ": Undefined array '" << node->arrayName << "'";
        errors.push_back(ss.str());
    }
    if (node->index) {
        node->index->accept(this);
        Type indexType = inferType(node->index.get());
        checkType(Type::INT, indexType, "Array index");
    }
}

void SemanticAnalyzer::visit(IncDecNode* node) {
    Symbol* sym = lookupSymbol(node->name);
    if (!sym) {
        std::stringstream ss;
        ss << "Line " << node->line << ": Undefined variable '" << node->name << "'";
        errors.push_back(ss.str());
    } else {
        if (sym->type != Type::INT && sym->type != Type::FLOAT && sym->type != Type::DOUBLE) {
            std::stringstream ss;
            ss << "Line " << node->line << ": Increment/decrement only works on numeric types";
            errors.push_back(ss.str());
        }
    }
}

void SemanticAnalyzer::visit(IncDecExprNode* node) {
    Symbol* sym = lookupSymbol(node->name);
    if (!sym) {
        std::stringstream ss;
        ss << "Line " << node->line << ": Undefined variable '" << node->name << "'";
        errors.push_back(ss.str());
    } else {
        if (sym->type != Type::INT && sym->type != Type::FLOAT && sym->type != Type::DOUBLE) {
            std::stringstream ss;
            ss << "Line " << node->line << ": Increment/decrement only works on numeric types";
            errors.push_back(ss.str());
        }
    }
}

void SemanticAnalyzer::visit(DoWhileNode* node) {
    if (node->body) {
        node->body->accept(this);
    }
    if (node->condition) {
        node->condition->accept(this);
        Type condType = inferType(node->condition.get());
        checkType(Type::BOOL, condType, "Do-while condition");
    }
}

void SemanticAnalyzer::visit(BreakNode* node) {
}

void SemanticAnalyzer::visit(ContinueNode* node) {
}

void SemanticAnalyzer::visit(SwitchNode* node) {
    if (node->expression) {
        node->expression->accept(this);
    }
    for (auto& caseNode : node->cases) {
        caseNode->accept(this);
    }
    if (node->defaultCase) {
        node->defaultCase->accept(this);
    }
}

void SemanticAnalyzer::visit(CaseNode* node) {
    if (node->value) {
        node->value->accept(this);
    }
    if (node->block) {
        node->block->accept(this);
    }
}

void SemanticAnalyzer::visit(TernaryExprNode* node) {
    if (node->condition) {
        node->condition->accept(this);
        Type condType = inferType(node->condition.get());
        if (condType != Type::BOOL && condType != Type::INT) {
            std::stringstream ss;
            ss << "Line " << node->line << ": Ternary condition must be boolean or numeric";
            errors.push_back(ss.str());
        }
    }
    if (node->trueExpr) {
        node->trueExpr->accept(this);
    }
    if (node->falseExpr) {
        node->falseExpr->accept(this);
    }
    Type trueType = inferType(node->trueExpr.get());
    Type falseType = inferType(node->falseExpr.get());
    if (trueType != falseType) {
        std::stringstream ss;
        ss << "Line " << node->line << ": Ternary operator branches must have compatible types";
        errors.push_back(ss.str());
    }
}
