#pragma once
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <string>
#include "term.hpp"
using namespace std;

template<typename Elemtype>
struct Linknode {
    Linknode* prenode = nullptr;
    Linknode* sucnode = nullptr;
    Elemtype data;
    Linknode(Elemtype d=Elemtype(), Linknode* pn = nullptr, Linknode* sn = nullptr)
        : data(d), prenode(pn), sucnode(sn) {
            if(!prenode) prenode=this;
            if(!sucnode) sucnode=this;
        }
};

template<typename Elemtype>
class LinkList {
protected:
    using node = Linknode<Elemtype>;
    int l = 0;
    node* headnode=new node();

    void checklength() {
        int i = 0;
        node* m = headnode->sucnode;
        while (m!=headnode) {
            m = m->sucnode;
            i++;
        }
        l = i;
    }

public:
    struct Iterator
    {
        node* ptr;
        Iterator(node*p):ptr(p){}
        Elemtype operator*(){return ptr->data;}
        Iterator operator++(){ptr=ptr->sucnode; return *this;}
        bool operator!=(const Iterator& other) const { return ptr != other.ptr; }
    };
    Iterator begin()const { return Iterator(headnode->sucnode); }
    Iterator end()const { return Iterator(headnode->prenode); }
    LinkList() :  l(0) {headnode->prenode=headnode,headnode->sucnode=headnode;}
    LinkList(const LinkList&other){
        l=other.l;
        for(auto da:other){
            tailinsert(da);
        }
    }
    ~LinkList() {
        clear();
    }

    void clear() {
        node* now = headnode->sucnode;
        while (now!=headnode) {
            node* next = now->sucnode;
            delete now;
            now = next;
        }
        delete headnode;
        headnode=new node();
        l = 0;
    }

    // 支持正负下标访问，-1为尾元素
    Elemtype& operator[](int idx) {
        if (l == 0) throw out_of_range("Empty list!");
        if (idx < -l || idx >= l) throw out_of_range("Index out of range");
        int realIdx = idx;
        if (idx < 0) realIdx = l + idx;
        node* now = headnode->sucnode;
        for (int i = 0; i < realIdx; ++i) now = now->sucnode;
        return now->data;
    }

    LinkList operator+(const LinkList&other) const{
        if(l!=other.l) throw invalid_argument("Lengths are not mathed");
        LinkList res(*this);
        node*cur1=headnode->sucnode,*cur2=other.headnode->sucnode,*cur=res.headnode->sucnode;
        for(int i=0;i<l;i++){
            cur->data=cur1->data+cur2->data;
            cur=cur->sucnode;
            cur1=cur1->sucnode;
            cur2=cur2->sucnode;
        }
        return res;
    }

    LinkList operator-(const LinkList&other) const{
        if(l!=other.l) throw invalid_argument("Lengths are not mathed");
        LinkList res(*this);
        node*cur1=headnode->sucnode,*cur2=other.headnode->sucnode,*cur=res.headnode->sucnode;
        for(int i=0;i<l;i++){
            cur->data=cur1->data-cur2->data;
            cur=cur->sucnode;
            cur1=cur1->sucnode;
            cur2=cur2->sucnode;
        }
        return res;
    }

    // 头插
    void headinsert(Elemtype value) {
        node* newnode = new node(value);
        newnode->prenode=headnode;
        newnode->sucnode=headnode->sucnode;
        newnode->sucnode->prenode=newnode;
        headnode->sucnode=newnode;
        l++;
    }

    void tailinsert(Elemtype value) {
        node* newnode = new node(value);
        newnode->sucnode=headnode;
        newnode->prenode=headnode->prenode;
        newnode->prenode->sucnode=newnode;
        headnode->prenode=newnode;
        l++;
    }

    int index(Elemtype value){
        node*cur=headnode->sucnode;
        for(int i=0;i<l;i++){
            if(cur->data==value) return i;
            cur=cur->sucnode;
        }
        return -1;
    }

    // insert(pos, value), 支持 pos ∈ [-l, l]
    void insert(int pos, Elemtype value) {
        if (pos < -l || pos > l) throw out_of_range("Insert position out of range");
        node*newnode=new node(value);
        int insert_pos = pos;
        if (pos < 0) insert_pos = l + pos + 1; // -1:尾插, -l:头插
        node*now=headnode;
        for(int i=0;i<insert_pos;i++){
            now=now->sucnode;
        }
        newnode->prenode=now;
        newnode->sucnode=now->sucnode;
        newnode->sucnode->prenode=newnode;
        now->sucnode=newnode;
        l++;
    }

    // 删除第pos个节点，支持负下标
    Elemtype erase(int pos) {
        if (l == 0) throw out_of_range("Empty list!");
        if (pos < -l || pos >= l) throw out_of_range("Erase position out of range");
        int realPos = pos;
        if (pos < 0) realPos = l + pos;
        node*cur=headnode->sucnode;
        for(int i=0;i<realPos;i++){
            cur=cur->sucnode;
        }
        cur->prenode->sucnode=cur->sucnode;
        cur->sucnode->prenode=cur->prenode;
        Elemtype e=cur->data;
        delete cur;
        l--;
        return e;
    }

    // 打印链表
    void print() const {
        if(!l) return;
        node* now = headnode->sucnode;
        cout<<now->data;
        now=now->sucnode;
        while (now!=headnode) {
            cout <<'+'<< now->data;
            now = now->sucnode;
        }
        cout << endl;
    }

    // 获取长度
    int size() const { return l; }

    // 获取尾元素
    Elemtype& back() {
        if (!l) throw out_of_range("Empty list!");
        return headnode->prenode->data;
    }
};
template<typename Elemtype>
class ordered_LinkList{
    protected:
    using node = Linknode<Elemtype>;
    int l = 0;
    node* headnode=new node();
    bool reverse=0;

    void checklength() {
        int i = 0;
        node* m = headnode->sucnode;
        while (m!=headnode) {
            m = m->sucnode;
            i++;
        }
        l = i;
    }

    public:
    struct Iterator
    {
        node* ptr;
        Iterator(node*p):ptr(p){}
        Elemtype operator*(){return ptr->data;}
        Iterator operator++(){ptr=ptr->sucnode; return *this;}
        bool operator!=(const Iterator& other) const { return ptr != other.ptr; }
    };
    Iterator begin()const { return Iterator(headnode->sucnode); }
    Iterator end()const { return Iterator(headnode); }
    ordered_LinkList() :  l(0) {headnode->prenode=headnode,headnode->sucnode=headnode;}
    ordered_LinkList(const ordered_LinkList<Elemtype>&other){
        l=other.l;
        reverse=other.reverse;
        for(auto da:other){
            insert(da);
        }
    }
    ordered_LinkList(const LinkList<Elemtype>&other,bool rev):reverse(rev){
        l=other.l;
        for(auto da:other){
            insert(da);
        }
    }
    ~ordered_LinkList() {
        clear();
    }

    void clear() {
        node* now = headnode->sucnode;
        while (now!=headnode) {
            node* next = now->sucnode;
            delete now;
            now = next;
        }
        delete headnode;
        headnode=new node();
        l = 0;
    }

    // 支持正负下标访问，-1为尾元素
    Elemtype& operator[](int idx) {
        if (l == 0) throw out_of_range("Empty list!");
        if (idx < -l || idx >= l) throw out_of_range("Index out of range");
        int realIdx = idx;
        if (idx < 0) realIdx = l + idx;
        node* now = headnode;
        for (int i = 0; i < realIdx; ++i) now = now->sucnode;
        return now->data;
    }
    int index(Elemtype value){
        node*cur=headnode->sucnode;
        for(int i=0;i<l;i++){
            if(cur->data==value) return i;
            cur=cur->sucnode;
        }
        return -1;
    }
    void sort(){sort(reverse);}
    void sort(bool rev){
        for(int i=0;i<l;i++){
            for(node*cur=headnode->sucnode;cur!=headnode;cur=cur->sucnode){
                if((cur->data>cur->sucnode->data)^rev){
                    Elemtype temp=cur->data;
                    cur->data=cur->sucnode->data;
                    cur->sucnode->data=temp;
                }
            }
        }
    }
    void insert(Elemtype value){
        node* cur = headnode;
        while(cur->sucnode != headnode && ((cur->sucnode->data < value) ^ reverse)) {
            cur = cur->sucnode;
        }
        node* newnode = new node(value);
        newnode->prenode = cur;
        newnode->sucnode = cur->sucnode;
        cur->sucnode->prenode = newnode;
        cur->sucnode = newnode;
        l++;
    }
    ordered_LinkList operator+(const ordered_LinkList&other) const{
        if(l!=other.l) throw invalid_argument("Lengths are not mathed");
        ordered_LinkList res(*this);
        node*cur1=headnode->sucnode,*cur2=other.headnode->sucnode,*cur=res.headnode->sucnode;
        for(int i=0;i<l;i++){
            cur->data=cur1->data+cur2->data;
            cur=cur->sucnode;
            cur1=cur1->sucnode;
            cur2=cur2->sucnode;
        }
        return res;
    }

    ordered_LinkList operator-(const ordered_LinkList&other) const{
        if(l!=other.l) throw invalid_argument("Lengths are not mathed");
        ordered_LinkList res(*this);
        node*cur1=headnode->sucnode,*cur2=other.headnode->sucnode,*cur=res.headnode->sucnode;
        for(int i=0;i<l;i++){
            cur->data=cur1->data-cur2->data;
            cur=cur->sucnode;
            cur1=cur1->sucnode;
            cur2=cur2->sucnode;
        }
        return res;
    }
    ordered_LinkList combine(const ordered_LinkList&other,bool rev) const{
        ordered_LinkList res(*this,rev);
        node*cur=other.headnode->sucnode;
        while(cur!=other.headnode){
            res.insert(cur->data);
            cur=cur->sucnode;
        }
        return res;
    }
    void print() const {
        if(!l) return;
        node* now = headnode->sucnode;
        cout<<now->data;
        now=now->sucnode;
        while (now!=headnode) {
            cout <<'+'<< now->data;
            now = now->sucnode;
        }
        cout << endl;
    }

    // 获取长度
    int size() const { return l; }

    // 获取尾元素
    Elemtype& back() {
        if (!l) throw out_of_range("Empty list!");
        return headnode->prenode->data;
    }
    Elemtype erase(int pos) {
        if (l == 0) throw out_of_range("Empty list!");
        if (pos < -l || pos >= l) throw out_of_range("Erase position out of range");
        int realPos = pos;
        if (pos < 0) realPos = l + pos;
        node*cur=headnode->sucnode;
        for(int i=0;i<realPos;i++){
            cur=cur->sucnode;
        }
        cur->prenode->sucnode=cur->sucnode;
        cur->sucnode->prenode=cur->prenode;
        Elemtype e=cur->data;
        delete cur;
        l--;
        return e;
    }
    Elemtype erase(node*cur){
        Elemtype e=cur->data;
        cur->prenode->sucnode=cur->sucnode;
        cur->sucnode->prenode=cur->prenode;
        delete cur;
        l--;
        return e;
    }
};




class Polynomial_Link : public ordered_LinkList<term>
{
    protected:
    typedef Linknode<term> Tnode;
    std::string variableName = "x";
    static constexpr double EPS = 1e-12;
    public:
    void setVariableName(const std::string &name) { variableName = name; }
    std::string getVariableName() const { return variableName; }

    void insert(const term& t) {
        Tnode* cur = headnode;
        // 找到合适的位置插入（降幂排列）
        while (cur->sucnode != headnode && cur->sucnode->data.degree > t.degree) {
            cur = cur->sucnode;
        }
        // 如果度数相同，直接合并
        if (cur->sucnode != headnode && cur->sucnode->data.degree == t.degree) {
            cur->sucnode->data.coefficient += t.coefficient;
            // 如果系数近似为0则删除
            if (std::fabs(cur->sucnode->data.coefficient) < EPS) {
                erase(cur->sucnode);
            }
        } else {
            // 插入新节点
            Tnode* newnode = new Tnode(t);
            newnode->prenode = cur;
            newnode->sucnode = cur->sucnode;
            cur->sucnode->prenode = newnode;
            cur->sucnode = newnode;
            l++;
        }
    }
    void differentiate(int num){
        if(num<=0) return;
        Tnode*cur=headnode->sucnode;
        while(cur!=headnode){
            for(int i=0;i<num;i++){
                cur->data.differentiate();
            }   
            if(std::fabs(cur->data.coefficient) < EPS){
                cur=cur->sucnode;
                erase(cur->prenode);
            }
            else cur=cur->sucnode;
        }
    }
    Polynomial_Link operator+(const Polynomial_Link&other) const{
        Polynomial_Link res;
        res.variableName = this->variableName;
        Tnode*cur1=headnode->sucnode,*cur2=other.headnode->sucnode;
        while(cur1!=headnode||cur2!=other.headnode){
            if(cur1==headnode){res.insert(cur2->data);cur2=cur2->sucnode;}
            else if(cur2==other.headnode){res.insert(cur1->data);cur1=cur1->sucnode;}
            else if(cur1->data.degree==cur2->data.degree){
                if(std::fabs(cur1->data.coefficient+cur2->data.coefficient) > EPS)
                    res.insert(cur1->data+cur2->data);
                cur1=cur1->sucnode,cur2=cur2->sucnode;
            }
            else if(cur1->data<cur2->data){
                res.insert(cur1->data);
                cur1=cur1->sucnode;
            }
            else{
                res.insert(cur2->data);
                cur2=cur2->sucnode;
            }
        }
        return res;
    }
    Polynomial_Link operator-(const Polynomial_Link&other) const{
        Polynomial_Link res;
        res.variableName = this->variableName;
        Tnode*cur1=headnode->sucnode,*cur2=other.headnode->sucnode;
        while(cur1!=headnode||cur2!=other.headnode){
            if(cur1==headnode){
                term tmp=cur2->data;
                tmp.coefficient=-tmp.coefficient;
                res.insert(tmp);
                cur2=cur2->sucnode;
            }
            else if(cur2==other.headnode){res.insert(cur1->data);cur1=cur1->sucnode;}
            else if(cur1->data.degree==cur2->data.degree){
                if(std::fabs(cur1->data.coefficient-cur2->data.coefficient) > EPS)
                    res.insert(cur1->data-cur2->data);
                cur1=cur1->sucnode,cur2=cur2->sucnode;
            }
            else if(cur1->data<cur2->data){
                res.insert(cur1->data);
                cur1=cur1->sucnode;
            }
            else{
                term tmp=cur2->data;
                tmp.coefficient=-tmp.coefficient;
                res.insert(tmp);
                cur2=cur2->sucnode;
            }
        }
        return res;
    }
    Polynomial_Link operator*(const Polynomial_Link& other) const {
        Polynomial_Link res;
        res.variableName = this->variableName;
        for (Tnode* cur1 = headnode->sucnode; cur1 != headnode; cur1 = cur1->sucnode) {
            for (Tnode* cur2 = other.headnode->sucnode; cur2 != other.headnode; cur2 = cur2->sucnode) {
                term prod = cur1->data * cur2->data;
                res.insert(prod); // 插入时合并同类项
            }
        }
        return res;
    }
    Polynomial_Link& operator=(const Polynomial_Link& other) {
        if (this == &other) return *this; // 防止自赋值

        this->clear(); // 清空当前链表，释放内存

        // 复制链表内容
        for (auto it = other.begin(); it != other.end(); ++it) {
            this->insert(*it); // 使用你的插入函数逐项插入
        }
        this->l = other.l;
        this->variableName = other.variableName;
        return *this;
    }
    term erase(Linknode<term>*cur){
        term e=cur->data;
        cur->prenode->sucnode=cur->sucnode;
        cur->sucnode->prenode=cur->prenode;
        delete cur;
        l--;
        return e;
    }
    void print() const {
        if(!l) { cout << "0\n"; return; }
        node* now = headnode->sucnode;
        auto printSingle = [&](const term &t, bool first){
            double c = t.coefficient;
            long long ic = llround(c);
            const double eps = 1e-12;
            string cs;
            if(ic==1&&t.degree!=0) cs=""; // 系数为1且非常数项，省略系数
            else if(ic==-1&&t.degree!=0) cs='-'; // 系数为-1且非常数项，省略系数但保留负号
            else if (std::fabs(c - (double)ic) < eps) cs = to_string(ic);
            else {
                ostringstream ss; ss.setf(ios::fixed); ss.precision(6); ss << c; cs = ss.str();
            }
            if (first) {
                cout << cs;
            } else {
                if (c >= 0) cout << '+';
                cout << cs;
            }
            cout << variableName << "^" << t.degree;
        };
        printSingle(now->data, true);
        now = now->sucnode;
        while (now!=headnode) {
            printSingle(now->data, false);
            now = now->sucnode;
        }
        cout << endl;
    }
};

class vect_Link:public LinkList<double>{
    public:
    using LinkList<double>::LinkList;
    using Dnode=Linknode<double>;
    double module()const{
        double res=0;
        for(Dnode*cur=headnode->sucnode;cur!=headnode;cur=cur->sucnode){
            res+=cur->data*cur->data;
        }
        return sqrt(res);
    }
    double operator*(const vect_Link&other)const{
        if(l!=other.l) throw invalid_argument("Different vect lenths");
        double res=0;
        Dnode*cur1=headnode->sucnode,*cur2=other.headnode->sucnode;
        while(cur1!=headnode&&cur2!=other.headnode){
            res+=cur1->data*cur2->data;
            cur1=cur1->sucnode,cur2=cur2->sucnode;
        }
        return res;
    }void print() const {
        if(!l) return;
        node* now = headnode->sucnode;
        cout<<now->data;
        now=now->sucnode;
        while (now!=headnode) {
            if(now->data>=0) cout<<'+';
            cout << now->data;
            now = now->sucnode;
        }
        cout << endl;
    }
};

double cos(vect_Link s1,vect_Link s2){
    if(s1.size()!=s2.size()) throw invalid_argument("The two vector have different dimension!");
    double m1=s1.module(),m2=s2.module();
    if(!m1||!m2) throw invalid_argument("There exists zero vector!");
    return (s1*s2)/(s1.module()*s2.module());
}