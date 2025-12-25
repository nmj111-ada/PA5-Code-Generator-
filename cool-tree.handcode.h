#define Expression_EXTRAS                    \
    Symbol type;                             \
    Symbol get_type() { return type; }       \
    Expression set_type(Symbol s) { type = s; return this; } \
    virtual void code(ostream&, Environment) = 0; \
    virtual bool IsEmpty() { return false; }

class assign_class : public Expression {
public:
    Symbol name;
    Expression expr;

    void code(ostream&, Environment);
    bool IsEmpty() { return false; }
};

class plus_class : public Expression {
public:
    Expression e1, e2;

    void code(ostream&, Environment);
    bool IsEmpty() { return false; }
};

class sub_class : public Expression {
public:
    Expression e1, e2;

    void code(ostream&, Environment);
    bool IsEmpty() { return false; }
};

class mul_class : public Expression {
public:
    Expression e1, e2;

    void code(ostream&, Environment);
    bool IsEmpty() { return false; }
};

class divide_class : public Expression {
public:
    Expression e1, e2;

    void code(ostream&, Environment);
    bool IsEmpty() { return false; }
};

class object_class : public Expression {
public:
    Symbol name;

    void code(ostream&, Environment);
    bool IsEmpty() { return false; }
};

class dispatch_class : public Expression {
public:
    Expression expr;
    Expressions actual;

    void code(ostream&, Environment);
    vector<Expression*> GetActuals();
    bool IsEmpty() { return false; }
};

class static_dispatch_class : public Expression {
public:
    Expression expr;
    Expressions actual;

    void code(ostream&, Environment);
    vector<Expression*> GetActuals();
    bool IsEmpty() { return false; }
};

class cond_class : public Expression {
public:
    Expression pred, then_exp, else_exp;

    void code(ostream&, Environment);
    bool IsEmpty() { return false; }
};

class loop_class : public Expression {
public:
    Expression pred, body;

    void code(ostream&, Environment);
    bool IsEmpty() { return false; }
};

class block_class : public Expression {
public:
    Expressions body;

    void code(ostream&, Environment);
    bool IsEmpty() { return false; }
};

class let_class : public Expression {
public:
    Symbol identifier;
    Symbol type_decl;
    Expression init;
    Expression body;

    void code(ostream&, Environment);
    bool IsEmpty() { return false; }
};

class case_class : public Expression {
public:
    Expression expr;
    Symbol type_decl;
    Symbol name;
    Expression branch;

    void code(ostream&, Environment);
    bool IsEmpty() { return false; }
};

class typcase_class : public Expression {
public:
    Expression expr;
    Cases cases;

    void code(ostream&, Environment);
    vector<branch_class*> GetCases();
    bool IsEmpty() { return false; }
};

class new__class : public Expression {
public:
    Symbol type_name;

    void code(ostream&, Environment);
    bool IsEmpty() { return false; }
};

class isvoid_class : public Expression {
public:
    Expression e;

    void code(ostream&, Environment);
    bool IsEmpty() { return false; }
};

class no_expr_class : public Expression {
public:
    void code(ostream&, Environment) {
        // 永远不会被调用
    }
    bool IsEmpty() { return true; }
};

#branch_class 补充
class branch_class : public tree_node {
public:
    Symbol name;
    Symbol type_decl;
    Expression expr;

    Symbol GetName() { return name; }
    Symbol GetTypeDecl() { return type_decl; }
    Expression GetExpr() { return expr; }
};

#formal_class 补充
class formal_class : public tree_node {
public:
    Symbol name;
    Symbol type_decl;

    Symbol GetName() { return name; }
    Symbol GetTypeDecl() { return type_decl; }
};

#attr_class 补充
class attr_class : public Feature {
public:
    Symbol name;
    Symbol type_decl;
    Expression init;

    Symbol GetName() { return name; }
    Symbol GetTypeDecl() { return type_decl; }
    Expression GetInit() { return init; }
};

#method_class 补充
class method_class : public Feature {
public:
    Symbol name;
    Formals formals;
    Symbol return_type;
    Expression expr;

    Symbol GetName() { return name; }
    Formals GetFormals() { return formals; }
    Expression GetExpr() { return expr; }
};

#细节补充
class Expression {
public:
    virtual void code(ostream& s, Environment env) = 0;

    virtual bool IsEmpty() { return false; }
};
vector<Expression*> dispatch_class::GetActuals() {
    vector<Expression*> v;
    for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
        v.push_back(actual->nth(i));
    }
    return v;
}

vector<Expression*> static_dispatch_class::GetActuals() {
    vector<Expression*> v;
    for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
        v.push_back(actual->nth(i));
    }
    return v;
}

vector<branch_class*> typcase_class::GetCases() {
    vector<branch_class*> v;
    for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
        v.push_back(cases->nth(i));
    }
    return v;
}

