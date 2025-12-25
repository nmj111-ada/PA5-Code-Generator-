#ifndef CGEN_H
#define CGEN_H

#include <iostream>
#include <map>
#include <vector>

#include "cool-tree.h"
#include "emit.h"

using std::ostream;
using std::map;
using std::vector;
class Environment {
public:
    CgenNode* m_class_node;

    Environment() : m_class_node(nullptr) {}

    void EnterScope() {
        var_env.push_back(map<Symbol, int>());
    }

    void ExitScope() {
        var_env.pop_back();
    }

    void AddVar(Symbol s) {
        var_env.back()[s] = var_cnt++;
    }

    void AddParam(Symbol s) {
        param_env[s] = param_cnt++;
    }

    int LookUpVar(Symbol s) {
        for (int i = var_env.size() - 1; i >= 0; --i) {
            auto it = var_env[i].find(s);
            if (it != var_env[i].end()) return it->second;
        }
        return -1;
    }

    int LookUpParam(Symbol s) {
        auto it = param_env.find(s);
        return it == param_env.end() ? -1 : it->second;
    }

    int LookUpAttrib(Symbol s) {
        return m_class_node->GetAttribIdx(s);
    }

    void AddObstacle() { var_cnt++; }

private:
    vector<map<Symbol, int>> var_env;
    map<Symbol, int> param_env;
    int var_cnt = 0;
    int param_cnt = 0;
};
class CgenNode : public class__class {
public:
    CgenNode(Class_ c, CgenNode* p)
        : class__class(*c), parentnd(p) {}

    CgenNode* get_parentnd() { return parentnd; }

    void set_parentnd(CgenNode* p) { parentnd = p; }

    void add_child(CgenNode* c) { children.push_back(c); }

    vector<CgenNode*>& get_children() { return children; }

    int GetTag() { return tag; }
    void SetTag(int t) { tag = t; }

    /* === 属性 === */
    vector<attr_class*> GetAttribs();
    vector<attr_class*> GetFullAttribs();
    int GetAttribIdx(Symbol s);
    map<Symbol, int>& GetAttribIdxTab();

    /* === 方法 === */
    vector<method_class*> GetMethods();
    vector<method_class*> GetFullMethods();
    map<Symbol, int>& GetDispatchIdxTab();

    /* === codegen === */
    void code_init(ostream& s);

private:
    CgenNode* parentnd;
    vector<CgenNode*> children;
    int tag;

    map<Symbol, int> attrib_idx_tab;
    map<Symbol, int> dispatch_idx_tab;
};
class CgenClassTable : public SymbolTable<Symbol, CgenNode> {
public:
    CgenClassTable(Classes classes, ostream& s);

    void code();

    CgenNode* GetClassNode(Symbol s) {
        return lookup(s);
    }

private:
    ostream& str;

    /* 构建 */
    void install_basic_classes();
    void install_classes(Classes);
    void build_inheritance_tree();

    /* 全局 */
    void code_global_data();
    void code_global_text();
    void code_select_gc();

    /* 常量 */
    void code_constants();

    /* 表 */
    void code_class_nameTab();
    void code_class_objTab();
    void code_dispatchTabs();
    void code_protObjs();

    /* 方法 */
    void code_class_inits();
    void code_class_methods();
};
extern CgenClassTable* codegen_classtable;
