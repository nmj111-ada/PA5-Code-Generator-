#ifndef COOL_TREE_HANDCODE_H
#define COOL_TREE_HANDCODE_H

#include "cool-tree.h"
#include "cgen.h"
/* ============================================================
   Expression extras（PA5 标准）
   ============================================================ */

#define Expression_EXTRAS                             \
    Symbol type;                                      \
    Symbol get_type() { return type; }                \
    Expression set_type(Symbol s) { type = s; return this; } \
    virtual void code(ostream&, Environment) = 0;     \
    virtual bool IsEmpty() { return false; }

/* ============================================================
   所有 Expression 节点的 code() 声明
   ============================================================ */

/* ---------- constants ---------- */

class int_const_class : public Expression {
public:
    Symbol token;
    void code(ostream&, Environment);
};

class string_const_class : public Expression {
public:
    Symbol token;
    void code(ostream&, Environment);
};

class bool_const_class : public Expression {
public:
    bool val;
    void code(ostream&, Environment);
};

/* ---------- variables ---------- */

class object_class : public Expression {
public:
    Symbol name;
    void code(ostream&, Environment);
};

class assign_class : public Expression {
public:
    Symbol name;
    Expression expr;
    void code(ostream&, Environment);
};

/* ---------- arithmetic ---------- */

class plus_class : public Expression {
public:
    Expression e1, e2;
    void code(ostream&, Environment);
};

class sub_class : public Expression {
public:
    Expression e1, e2;
    void code(ostream&, Environment);
};

class mul_class : public Expression {
public:
    Expression e1, e2;
    void code(ostream&, Environment);
};

class divide_class : public Expression {
public:
    Expression e1, e2;
    void code(ostream&, Environment);
};

/* ---------- comparison ---------- */

class lt_class : public Expression {
public:
    Expression e1, e2;
    void code(ostream&, Environment);
};

class leq_class : public Expression {
public:
    Expression e1, e2;
    void code(ostream&, Environment);
};

class eq_class : public Expression {
public:
    Expression e1, e2;
    void code(ostream&, Environment);
};

/* ---------- control flow ---------- */

class cond_class : public Expression {
public:
    Expression pred, then_exp, else_exp;
    void code(ostream&, Environment);
};

class loop_class : public Expression {
public:
    Expression pred, body;
    void code(ostream&, Environment);
};

class block_class : public Expression {
public:
    Expressions body;
    void code(ostream&, Environment);
};

/* ---------- let ---------- */

class let_class : public Expression {
public:
    Symbol identifier;
    Symbol type_decl;
    Expression init;
    Expression body;
    void code(ostream&, Environment);
};

/* ---------- dispatch ---------- */

class dispatch_class : public Expression {
public:
    Expression expr;
    Expressions actual;

    void code(ostream&, Environment);
    vector<Expression*> GetActuals();
};

class static_dispatch_class : public Expression {
public:
    Expression expr;
    Symbol type_name;
    Expressions actual;

    void code(ostream&, Environment);
    vector<Expression*> GetActuals();
};

/* ---------- case ---------- */

class branch_class : public tree_node {
public:
    Symbol name;
    Symbol type_decl;
    Expression expr;

    Symbol GetName() { return name; }
    Symbol GetTypeDecl() { return type_decl; }
    Expression GetExpr() { return expr; }
};

class typcase_class : public Expression {
public:
    Expression expr;
    Cases cases;

    void code(ostream&, Environment);
    vector<branch_class*> GetCases();
};

/* ---------- misc ---------- */

class new__class : public Expression {
public:
    Symbol type_name;
    void code(ostream&, Environment);
};

class isvoid_class : public Expression {
public:
    Expression e;
    void code(ostream&, Environment);
};

class no_expr_class : public Expression {
public:
    void code(ostream&, Environment) {}
    bool IsEmpty() { return true; }
};

#endif

