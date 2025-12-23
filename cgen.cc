#include "cgen.h"
#include "cool-tree.h"
#include "emit.h"

extern int cgen_Memmgr;

CgenClassTable* codegen_classtable = nullptr;
static int labelnum = 0;

/program_class::cgen/
void program_class::cgen(ostream& s) {
    codegen_classtable = new CgenClassTable(classes, s);
}

/CgenClassTable 构造与主流程/
CgenClassTable::CgenClassTable(Classes classes, ostream& s)
    : str(s) {

    install_basic_classes();
    install_classes(classes);
    build_inheritance_tree();

    code();
}
void CgenClassTable::code() {
    code_global_data();
    code_select_gc();
    code_constants();
    code_class_nameTab();
    code_class_objTab();
    code_dispatchTabs();
    code_protObjs();
    code_global_text();
    code_class_inits();
    code_class_methods();
}

/global data / text/
void CgenClassTable::code_global_data() {
    str << DATA << endl;
    str << ALIGN << 2 << endl;
    str << GLOBAL << CLASSNAMETAB << endl;
    str << GLOBAL << CLASSOBJTAB << endl;
}

void CgenClassTable::code_global_text() {
    str << TEXT << endl;
    str << GLOBAL << "Main_init" << endl;
    str << GLOBAL << "Main.main" << endl;
}

/GC 选择/
void CgenClassTable::code_select_gc() {
    if (cgen_Memmgr == 1) {
        str << GLOBAL << "_GenGC_Init" << endl;
        str << GLOBAL << "_GenGC_Assign" << endl;
    }
}

/常量生成/
void CgenClassTable::code_constants() {
    stringtable.add_string("");
    inttable.add_string("0");

    stringtable.code_string_table(str, stringclasstag);
    inttable.code_string_table(str, intclasstag);
    code_bools(boolclasstag);
}

/class_nameTab/
void CgenClassTable::code_class_nameTab() {
    str << CLASSNAMETAB << LABEL << endl;
    for (auto& it : nodes) {
        emit_word(it.second->name->get_string() + STRCONST_SUFFIX, str);
    }
}

/class_objTab/
void CgenClassTable::code_class_objTab() {
    str << CLASSOBJTAB << LABEL << endl;
    for (auto& it : nodes) {
        CgenNode* node = it.second;
        emit_word(node->name->get_string() + PROTOBJ_SUFFIX, str);
        emit_word(node->name->get_string() + CLASSINIT_SUFFIX, str);
    }
}

/dispatchTabs/
void CgenClassTable::code_dispatchTabs() {
    for (auto& it : nodes) {
        CgenNode* cls = it.second;
        str << cls->name->get_string() << DISPTAB_SUFFIX << LABEL << endl;

        auto methods = cls->GetFullMethods();
        for (auto m : methods) {
            emit_word(
                m->GetDefClass()->name->get_string() + METHOD_SEP +
                m->name->get_string(),
                str
            );
        }
    }
}

/protObjs/
void CgenClassTable::code_protObjs() {
    for (auto& it : nodes) {
        CgenNode* cls = it.second;
        str << cls->name->get_string() + PROTOBJ_SUFFIX << LABEL << endl;

        emit_word(cls->GetClassTag(), str);
        emit_word(cls->GetObjectSize(), str);
        emit_word(cls->name->get_string() + DISPTAB_SUFFIX, str);

        auto attribs = cls->GetFullAttributes();
        for (auto a : attribs) {
            Symbol type = a->GetType();
            if (type == Int) {
                emit_word(inttable.lookup_string("0"), str);
            } else if (type == Bool) {
                emit_word(booltable.lookup_string("0"), str);
            } else if (type == Str) {
                emit_word(stringtable.lookup_string(""), str);
            } else {
                emit_word(0, str);
            }
        }
    }
}

/class_inits/
void CgenClassTable::code_class_inits() {
    for (auto& it : nodes) {
        it.second->code_init(str);
    }
}
void CgenNode::code_init(ostream& s) {
    emit_init_ref(name, s);
    s << LABEL;

    emit_addiu(SP, SP, -12, s);
    emit_store(FP, 3, SP, s);
    emit_store(SELF, 2, SP, s);
    emit_store(RA, 1, SP, s);
    emit_addiu(FP, SP, 4, s);
    emit_move(SELF, ACC, s);

    if (get_parentnd()->name != No_class) {
        emit_jal(get_parentnd()->name->get_string() + CLASSINIT_SUFFIX, s);
    }

    auto attribs = GetAttribs();
    auto idx_tab = GetAttribIdxTab();

    for (auto attr : attribs) {
        int idx = idx_tab[attr->name];
        if (attr->init->IsEmpty()) {
            if (attr->type_decl == Int)
                emit_load_int(ACC, inttable.lookup_string("0"), s);
            else if (attr->type_decl == Bool)
                emit_load_bool(ACC, BoolConst(0), s);
            else if (attr->type_decl == Str)
                emit_load_string(ACC, stringtable.lookup_string(""), s);
            else
                emit_move(ACC, ZERO, s);
        } else {
            Environment env;
            env.m_class_node = this;
            attr->init->code(s, env);
        }
        emit_store(ACC, idx + 3, SELF, s);
    }

    emit_move(ACC, SELF, s);
    emit_load(FP, 3, SP, s);
    emit_load(SELF, 2, SP, s);
    emit_load(RA, 1, SP, s);
    emit_addiu(SP, SP, 12, s);
    emit_return(s);
}

/methods/
void CgenClassTable::code_class_methods() {
    for (auto& it : nodes) {
        auto methods = it.second->GetMethods();
        for (auto m : methods) {
            m->code(str, it.second);
        }
    }
}



/常量 & object & assign
/***********************************
 * Constants
 ***********************************/
void int_const_class::code(ostream& s, Environment env) {
    emit_load_int(ACC, inttable.lookup_string(token->get_string()), s);
}

void string_const_class::code(ostream& s, Environment env) {
    emit_load_string(ACC, stringtable.lookup_string(token->get_string()), s);
}

void bool_const_class::code(ostream& s, Environment env) {
    emit_load_bool(ACC, BoolConst(val), s);
}

/***********************************
 * Object
 ***********************************/
void object_class::code(ostream& s, Environment env) {
    int idx;
    if (name == self) {
        emit_move(ACC, SELF, s);
    } else if ((idx = env.LookUpVar(name)) != -1) {
        emit_load(ACC, idx + 1, SP, s);
    } else if ((idx = env.LookUpParam(name)) != -1) {
        emit_load(ACC, idx + 3, FP, s);
    } else if ((idx = env.LookUpAttrib(name)) != -1) {
        emit_load(ACC, idx + 3, SELF, s);
    }
}

/***********************************
 * Assign
 ***********************************/
void assign_class::code(ostream& s, Environment env) {
    expr->code(s, env);

    int idx;
    if ((idx = env.LookUpVar(name)) != -1) {
        emit_store(ACC, idx + 1, SP, s);
    } else if ((idx = env.LookUpParam(name)) != -1) {
        emit_store(ACC, idx + 3, FP, s);
    } else if ((idx = env.LookUpAttrib(name)) != -1) {
        emit_store(ACC, idx + 3, SELF, s);
    }
}

/算术运算plus / sub / mul / divide/
void plus_class::code(ostream& s, Environment env) {
    e1->code(s, env);
    emit_push(ACC, s);
    env.AddObstacle();

    e2->code(s, env);
    emit_jal("Object.copy", s);

    emit_addiu(SP, SP, 4, s);
    emit_load(T1, 0, SP, s);

    emit_load(T1, 3, T1, s);
    emit_load(T2, 3, ACC, s);
    emit_add(T3, T1, T2, s);
    emit_store(T3, 3, ACC, s);
}

void sub_class::code(ostream& s, Environment env) {
    e1->code(s, env);
    emit_push(ACC, s);
    env.AddObstacle();

    e2->code(s, env);
    emit_jal("Object.copy", s);

    emit_addiu(SP, SP, 4, s);
    emit_load(T1, 0, SP, s);

    emit_load(T1, 3, T1, s);
    emit_load(T2, 3, ACC, s);
    emit_sub(T3, T1, T2, s);
    emit_store(T3, 3, ACC, s);
}

void mul_class::code(ostream& s, Environment env) {
    e1->code(s, env);
    emit_push(ACC, s);
    env.AddObstacle();

    e2->code(s, env);
    emit_jal("Object.copy", s);

    emit_addiu(SP, SP, 4, s);
    emit_load(T1, 0, SP, s);

    emit_load(T1, 3, T1, s);
    emit_load(T2, 3, ACC, s);
    emit_mul(T3, T1, T2, s);
    emit_store(T3, 3, ACC, s);
}

void divide_class::code(ostream& s, Environment env) {
    e1->code(s, env);
    emit_push(ACC, s);
    env.AddObstacle();

    e2->code(s, env);
    emit_jal("Object.copy", s);

    emit_addiu(SP, SP, 4, s);
    emit_load(T1, 0, SP, s);

    emit_load(T1, 3, T1, s);
    emit_load(T2, 3, ACC, s);
    emit_div(T3, T1, T2, s);
    emit_store(T3, 3, ACC, s);
}

/比较运算lt / leq / eq/
void lt_class::code(ostream& s, Environment env) {
    e1->code(s, env);
    emit_push(ACC, s);
    env.AddObstacle();

    e2->code(s, env);

    emit_addiu(SP, SP, 4, s);
    emit_load(T1, 0, SP, s);

    emit_load(T1, 3, T1, s);
    emit_load(T2, 3, ACC, s);

    int ltrue = labelnum++;
    int lend  = labelnum++;

    emit_blt(T1, T2, ltrue, s);
    emit_load_bool(ACC, BoolConst(0), s);
    emit_branch(lend, s);

    emit_label_def(ltrue, s);
    emit_load_bool(ACC, BoolConst(1), s);

    emit_label_def(lend, s);
}
/if / while / block/
void cond_class::code(ostream& s, Environment env) {
    pred->code(s, env);
    emit_fetch_int(T1, ACC, s);

    int lfalse = labelnum++;
    int lend   = labelnum++;

    emit_beq(T1, ZERO, lfalse, s);
    then_exp->code(s, env);
    emit_branch(lend, s);

    emit_label_def(lfalse, s);
    else_exp->code(s, env);

    emit_label_def(lend, s);
}

void loop_class::code(ostream& s, Environment env) {
    int lstart = labelnum++;
    int lend   = labelnum++;

    emit_label_def(lstart, s);
    pred->code(s, env);
    emit_fetch_int(T1, ACC, s);
    emit_beq(T1, ZERO, lend, s);

    body->code(s, env);
    emit_branch(lstart, s);

    emit_label_def(lend, s);
    emit_move(ACC, ZERO, s);
}

void block_class::code(ostream& s, Environment env) {
    for (int i = body->first(); body->more(i); i = body->next(i)) {
        body->nth(i)->code(s, env);
    }
}

/let表达式/
void let_class::code(ostream& s, Environment env) {
    if (init->IsEmpty()) {
        if (type_decl == Int)
            emit_load_int(ACC, inttable.lookup_string("0"), s);
        else if (type_decl == Bool)
            emit_load_bool(ACC, BoolConst(0), s);
        else if (type_decl == Str)
            emit_load_string(ACC, stringtable.lookup_string(""), s);
        else
            emit_move(ACC, ZERO, s);
    } else {
        init->code(s, env);
    }

    emit_push(ACC, s);

    env.EnterScope();
    env.AddVar(identifier);

    body->code(s, env);

    env.ExitScope();
    emit_addiu(SP, SP, 4, s);
}

/dispatch/
void dispatch_class::code(ostream& s, Environment env) {
    auto actuals = GetActuals();
    for (auto e : actuals) {
        e->code(s, env);
        emit_push(ACC, s);
        env.AddObstacle();
    }

    expr->code(s, env);

    emit_bne(ACC, ZERO, labelnum, s);
    emit_load_address(ACC, "str_const0", s);
    emit_load_imm(T1, 1, s);
    emit_jal("_dispatch_abort", s);
    emit_label_def(labelnum++, s);

    Symbol cname = env.m_class_node->name;
    if (expr->get_type() != SELF_TYPE)
        cname = expr->get_type();

    CgenNode* node = codegen_classtable->GetClassNode(cname);

    emit_load(T1, 2, ACC, s);
    int idx = node->GetDispatchIdxTab()[name];
    emit_load(T1, idx, T1, s);
    emit_jalr(T1, s);
}

/method & init/
void method_class::code(ostream& s, CgenNode* cls) {
    emit_method_ref(cls->name, name, s);
    s << LABEL;

    emit_addiu(SP, SP, -12, s);
    emit_store(FP, 3, SP, s);
    emit_store(SELF, 2, SP, s);
    emit_store(RA, 1, SP, s);

    emit_addiu(FP, SP, 4, s);
    emit_move(SELF, ACC, s);

    Environment env;
    env.m_class_node = cls;
    for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
        env.AddParam(formals->nth(i)->GetName());
    }

    expr->code(s, env);

    emit_load(FP, 3, SP, s);
    emit_load(SELF, 2, SP, s);
    emit_load(RA, 1, SP, s);
    emit_addiu(SP, SP, 12, s);
    emit_addiu(SP, SP, formals->len() * 4, s);
    emit_return(s);
}
