/*
	calculator08buggy.cpp | From: Programming -- Principles and Practice Using C++, by Bjarne Stroustrup
	We have inserted 3 bugs that the compiler will catch and 3 that it won't.
*/

/*
 *  UCF COP3330 Fall 2021 Assignment 6 Solution
 *  Copyright 2021 Rafael Alvarez-Romero
 */

#include "header.h"

class Token {
public:
    char kind;        
    double value;   
    string name;     
    Token(char ch) : kind(ch), value(0) {}
    Token(char ch, double val) : kind(ch), value(val) {}
    Token(char ch, string n) : kind(ch), name(n) {}
};

const char let = 'L';
const char quit = 'q';
const char print = ';';
const char number = '8';
const char name = 'a';
const char isConst = 'C';
const string constKey = "const";

class Token_stream {
    bool full;
    Token buffer;
public:
    Token_stream() :full(0), buffer(0) { }

    Token get();
    void unget(Token t) { buffer = t; full = true; }

    void ignore(char);
};


Token_stream::Token_stream()
    :full(false), buffer(0)   
{
}

void Token_stream::unget(Token t)
{
    if (full) error("putback() into a full buffer");
    buffer = t;       
    full = true;      
}

Token Token_stream::get()
{
    if (full) { full = false; return buffer; }
    char ch;
    cin >> ch;        

    switch (ch) {
    case '(':
    case ')':
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '=':
        return Token(ch);
    case '.':             
    case '0': 
    case '1': 
    case '2': 
    case '3': 
    case '4':
    case '5': 
    case '6': 
    case '7': 
    case '8': 
    case '9':    
    {
        cin.unget();
        double val;
        cin >> val;     
        return Token(number, val);
    }
    default:
        if (isalpha(ch) || ch == '_') {
            string s;
            s += ch;
            while (cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) s += ch;	
            cin.unget();
            if (s == "let") return Token(let);
            if (s == "quit") return Token(quit);
            return Token(name, s);
        }
        error("Bad token");
    }
}

void Token_stream::ignore(char c)
{
    if (full && c == buffer.kind) {
        full = false;
        return;
    }
    full = false;

    char ch;
    while (cin >> ch)
        if (ch == c) return;
}     

class Variable {
public:
    string name;
    double value;
    bool isConst;
    Variable(string n, double v, bool ic) :name(n), value(v), isConst(ic) { }
};

vector<Variable> names;

double get_value(string s)
{
    for (int i = 0; i < names.size(); ++i)
        if (names[i].name == s) return names[i].value;
    error("get: undefined variable ", s);
}

void set_value(string s, double d)
{
    for (int i = 0; i < names.size(); ++i)
        if (names[i].name == s) {
            if (names[i].name == s && names[i].isConst == false) {
                names[i].value = d;
                return;
            }
        }
    error("set: undefined variable ", s);
}


bool is_declared(string s)
{
    for (int i = 0; i < names.size(); ++i)
    {
        if (names[i].name == s && names[i].isConst == true)
            error("Cannot reassign const variable");
        else if (names[i].name == s && names[i].isConst == false)
            return true;
    }
    return false;
}

double define_names(string var, double val, bool isConst)
{
    if (is_declared(var))
        error(var, " declared twice");

    names.push_back(Variable(var, val, isConst));

    return val;
}

Token_stream ts;

double expression();   

double primary()
{
    Token t = ts.get();
    switch (t.kind) {
    case '(':           
    {
        double d = expression();
        t = ts.get();
        if (t.kind != ')') error("')' expected");
        return d;
    }
    case number:
        return t.value;   
    case name:
    {
        Token next = ts.get();
        if (next.kind == '=') {	
            double d = expression();
            set_value(t.name, d);
            return d;
        }
        else {
            ts.unget(next);		
            return get_value(t.name); 
        }
    }
    case '-':
        return -primary();
    case '+':
        return primary();
    default:
        error("primary expected");
    }
}




double term()
{
    double left = primary();
    Token t = ts.get(); 

    while (true) {
        switch (t.kind) {
        case '*':
            left *= primary();
            break;
        case '/':
        {
            double d = primary();
            if (d == 0) error("divide by zero");
            left /= d;
            break;
        }
        default:
            ts.unget(t);       
            return left;
        }
    }
}


double expression()
{
    double left = term(); 

    while (true) {
        Token t = ts.get();
        switch (t.kind) {
        case '+':
            left += term();    
            break;
        case '-':
            left -= term();    
            break;
        default:
            ts.unget(t);     
            return left;       
        }
    }
}

double declaration()
{
    Token t = ts.get();
    bool isC;
    if (t.kind == 'C')
    {
        isC = true;
        t = ts.get(); 
    }
    else
        isC = false;

    if (t.kind != 'a')
        error("A name expected in declaration");

    string name = t.name;
    if (is_declared(name))
    {
        cout << name + ", is declared twice. Reassign? y/n ";
        cin.clear();
        cin.ignore(10000, '\n'); 
        string ans;
        getline(cin, ans);
        if (ans == "n" || ans == "N")
            error(name, ", will not be reassigned ");
        if (ans == "y" || ans == "Y")
        {
            cout << "Please enter new value: ";
            int val;
            cin >> val;
            set_value(name, val);
            double d = val; 
            return d;
        }
    }
    Token t2 = ts.get();
    if (t2.kind != '=')
        error("= is missing in declaration of ", name);
    double d = expression();
    names.push_back(Variable(name, d, isC));
    return d;
}
double statement()
{
    Token t = ts.get();
    switch (t.kind) {
    case let:
        return declaration();
    default:
        ts.unget(t);
        return expression();
    }
}

void clean_up_mess()
{
    ts.ignore(print);
}

const string prompt = "> ";
const string result = "= ";

void calculate()
{
    while (true) try {
        cout << prompt;
        Token t = ts.get();
        while (t.kind == print) t = ts.get();
        if (t.kind == quit) return;
        ts.unget(t);
        cout << result << statement() << endl;
    }
    catch (runtime_error& e) {
        cerr << e.what() << endl;
        clean_up_mess();
    }
}


int main() {
    try {
        define_names("pi", 3.1415926535, false);	
        define_names("e", 2.7182818284, false);

        calculate();   
        return 0;
    }

    catch (exception& e) {
        cerr << e.what() << endl;
        char c;
        while (cin >> c && c != ';');
        return 1;
    }

    catch (...) {
        cerr << "exception \n";
        char c;
        while (cin >> c && c != ';');
        return 2;
    }
}