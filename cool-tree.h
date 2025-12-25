#ifndef COOL_TREE_H
#define COOL_TREE_H

#include <iostream>
#include <vector>

#include "tree.h"
#include "list.h"
#include "symbol.h"

using std::ostream;
using std::vector;

/* ============================================================
   Forward declarations
   ============================================================ */

class Class_;
class Feature;
class Formal;
class Expression;
class Case;

typedef list<Class_> Classes;
typedef list<Feature> Features;
typedef list<Formal> Formals;
typedef list<Expression> Expressions;
typedef list<Case> Cases;

/* ============================================================
   Base AST nodes
   ============================================================ */

class Program : public tree_node {
public:
    virtual void cgen(ostream& s) = 0;
};

class Class_ : public tree_node {
public:
    virtual Symbol get_name() = 0;
    virtual Symbol get_parent() = 0;
    virtual Features get_features() = 0;
};

class Feature : public tree_node {
};

class Formal : public tree_node {
};

class Expression : public tree_node {
public:
    Symbol type;

    virtual Symbol get_type() { return type; }
    virtual Expression set_type(Symbol s) { type = s; return this; }

    virtual void code(ostream& s, class Environment env) = 0;
    virtual bool IsEmpty() { return false; }
};

class Case : public tree_node {
};

/* ============================================================
   Program
   ============================================================ */

class program_class : public Program {
public:
    Classes classes;

    program_class(Classes c) : classes(c) {}
    void cgen(ostream& s);
};

/* ============================================================
   Class
   ============================================================ */

class class__class : public Class_ {
public:
    Symbol name;
    Symbol parent;
    Features features;
    Symbol filename;

    class__class(Symbol n, Symbol p, Features f, Symbol fn)
        : name(n), parent(p), features(f), filename(fn) {}

    Symbol get_name() { return name; }
    Symbol get_parent() { return parent; }
    Features get_features() { return features; }
};

/* ============================================================
   Features
   ============================================================ */

class method_class : public Feature {
public:
    Symbol name;
    Formals formals;
    Symbol return_type;
    Expression expr;

    method_class(Symbol n, Formals f, Symbol r, Expression e)
        : name(n), formals(f), return_type(r), expr(e) {}

    Symbol GetName() { return name; }
    Formals GetFormals() { return formals; }
    Expression GetExpr() { return expr; }

    void code(ostream& s, class CgenNode* cls);
};

class attr_class : public Feature {
public:
    Symbol name;
    Symbol type_decl;
    Expression init;

    attr_class(Symbol n, Symbol t, Expression i)
        : name(n), type_decl(t), init(i) {}

    Symbol GetName() { return name; }
    Symbol GetTypeDecl() { return type_decl; }
    Expression GetInit() { return init; }
};

/* ============================================================
   Formals
   ============================================================ */

class formal_class : public Formal {
public:
    Symbol name;
    Symbol type_decl;

    formal_class(Symbol n, Symbol t)
        : name(n), type_decl(t) {}

    Symbol GetName() { return name; }
    Symbol GetTypeDecl() { return type_decl; }
};

/* ============================================================
   Expressions – constants
   ============================================================ */

class int_const_class : public Expression {
public:
    Symbol token;
    int_const_class(Symbol t) : token(t) {}
    void code(ostream& s, Environment env);
};

class string_const_class : public Expression {
public:
    Symbol token;
    string_const_class(Symbol t) : token(t) {}
    void code(ostream& s, Environment env);
};

class bool_const_class : public Expression {
public:
    bool val;
    bool_const_class(bool v) : val(v) {}
    void code(ostream& s, Environment env);
};

/* ============================================================
   Expressions – variables / assign
   ============================================================ */

class object_class : public Expression {
public:
    Symbol name;
    object_class(Symbol n) : name(n) {}
    void code(ostream& s, Environment env);
};

class assign_class : public Expression {
public:
    Symbol name;
    Expression expr;

    assign_class(Symbol n, Expression e) : name(n), expr(e) {}
    void code(ostream& s, Environment env);
};

/* ============================================================
   Expressions – arithmetic
   ============================================================ */

class plus_class : public Expression {
public:
    Expression e1, e2;
    plus_class(Expression a1, Expression a2) : e1(a1), e2(a2) {}
    void code(ostream& s, Environment env);
};

class sub_class : public Expression {
public:
    Expression e1, e2;
    sub_class(Expression a1, Expression a2) : e1(a1), e2(a2) {}
    void code(ostream& s, Environment env);
};

class mul_class : public Expression {
public:
    Expression e1, e2;
    mul_class(Expression a1, Expression a2) : e1(a1), e2(a2) {}
    void code(ostream& s, Environment env);
};

class divide_class : public Expression {
public:
    Expression e1, e2;
    divide_class(Expression a1, Expression a2) : e1(a1), e2(a2) {}
    void code(ostream& s, Environment env);
};

/* ============================================================
   Expressions – comparison
   ============================================================ */

class lt_class : public Expression {
public:
    Expression e1, e2;
    lt_class(Expression a1, Expression a2) : e1(a1), e2(a2) {}
    void code(ostream& s, Environment env);
};

class leq_class : public Expression {
public:
    Expression e1, e2;
    leq_class(Expression a1, Expression a2) : e1(a1), e2(a2) {}
    void code(ostream& s, Environment env);
};

class eq_class : public Expression {
public:
    Expression e1, e2;
    eq_class(Expression a1, Expression a2) : e1(a1), e2(a2) {}
    void code(ostream& s, Environment env);
};

/* ============================================================
   Expressions – control flow
   ============================================================ */

class cond_class : public Expression {
public:
    Expression pred, then_exp, else_exp;
    cond_class(Expression p, Expression t, Expression e)
        : pred(p), then_exp(t), else_exp(e) {}
    void code(ostream& s, Environment env);
};

class loop_class : public Expression {
public:
    Expression pred, body;
    loop_class(Expression p, Expression b) : pred(p), body(b) {}
    void code(ostream& s, Environment env);
};

class block_class : public Expression {
public:
    Expressions body;
    block_class(Expressions b) : body(b) {}
    void code(ostream& s, Environment env);
};

/* ============================================================
   let
   ============================================================ */

class let_class : public Expression {
public:
    Symbol identifier;
    Symbol type_decl;
    Expression init;
    Expression body;

    let_class(Symbol i, Symbol t, Expression init, Expression b)
        : identifier(i), type_decl(t), init(init), body(b) {}

    void code(ostream& s, Environment env);
};

/* ============================================================
   dispatch
   ============================================================ */

class dispatch_class : public Expression {
public:
    Expression expr;
    Expressions actual;

    dispatch_class(Expression e, Expressions a)
        : expr(e), actual(a) {}

    void code(ostream& s, Environment env);
    vector<Expression*> GetActuals();
};

class static_dispatch_class : public Expression {
public:
    Expression expr;
    Symbol type_name;
    Expressions actual;

    static_dispatch_class(Expression e, Symbol t, Expressions a)
        : expr(e), type_name(t), actual(a) {}

    void code(ostream& s, Environment env);
    vector<Expression*> GetActuals();
};

/* ============================================================
   case
   ============================================================ */

class branch_class : public Case {
public:
    Symbol name;
    Symbol type_decl;
    Expression expr;

    branch_class(Symbol n, Symbol t, Expression e)
        : name(n), type_decl(t), expr(e) {}

    Symbol GetName() { return name; }
    Symbol GetTypeDecl() { return type_decl; }
    Expression GetExpr() { return expr; }
};

class typcase_class : public Expression {
public:
    Expression expr;
    Cases cases;

    typcase_class(Expression e, Cases c) : expr(e), cases(c) {}

    void code(ostream& s, Environment env);
    vector<branch_class*> GetCases();
};

/* ============================================================
   misc
   ============================================================ */

class new__class : public Expression {
public:
    Symbol type_name;
    new__class(Symbol t) : type_name(t) {}
    void code(ostream& s, Environment env);
};

class isvoid_class : public Expression {
public:
    Expression e;
    isvoid_class(Expression x) : e(x) {}
    void code(ostream& s, Environment env);
};

class no_expr_class : public Expression {
public:
    no_expr_class() {}
    void code(ostream&, Environment) {}
    bool IsEmpty() { return true; }
};

#endif
