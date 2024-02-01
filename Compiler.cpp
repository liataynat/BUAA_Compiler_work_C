#include<bits/stdc++.h>
#include<string>
#include<iostream>
#include<stack>
int line_num=1;
int top_tokens=0;
int top_sts=0;//最高层表
int now_sts=0;//目前所处表
int judge_fpnum=-1;//当前所处函数的（第几个）参数
int judge_fpt=-1;//当前所处函数的参数的类型
int judge_f=-1; // 正在处理的函数
int error_f=404;//判断是否存在f类错误（便于返回return的位置）
int flag_tokens_1=0;
int flag_tokens_2=0;
int first_call=1;
int first_call_e=1;
int first_call_mid = 1;
int is_return =0;//判断是否处于f类中return问题
int fblock=0;//判断是否是函数最外层的block
int in_main=0;//判断是否进入main函数
int num_return = 0;//目前return的个数
int num_print = 0;
int in_for = 0;//在循环中
int f_or_brack = 0;//判断是在函数（）中还是数组[]中
int list_print[1000] = {0};
int in_which_f = -1;//f类错误处理补丁
int llvm_r = 1;
//解决if和for基本块问题
int llvm_if_r = 1;
std::stack <int> ifstack;
int llvm_or_r = 1;
std::stack <int> orstack;
int llvm_and_r = 1;
std::stack <int> andstack;
int llvm_c_r = 1;
std::stack <int> cstack;
int llvm_s_r = 1;
std::stack <int> sstack;
int llvm_f_r = 1;
std::stack <int> fstack;
int llvm_b_r = 1;
std::stack <int> bstack;
int in_if_or_for = 0;
int if_num = 0;
// Token 类型
typedef enum {
    IDENFR,INTCON,STRCON,MAINTK,CONSTTK,INTTK,BREAKTK,CONTINUETK,IFTK,ELSETK,NOT,AND,OR,FORTK,GETINTTK,PRINTFTK,RETURNTK,PLUS,MINU,VOIDTK,MULT,DIV,MOD,LSS,LEQ,GRE,GEQ,EQL,NEQ,ASSIGN,SEMICN,COMMA,LPARENT,RPARENT,LBRACK,RBRACK,LBRACE,RBRACE,UNKNOWN
} TokenType;

// Token 结构体
typedef struct {
    TokenType type;
    char *value;
    int line;
    int error;
} Token,TreeNode;
Token *tokens[100000];  // 存储Token的数组
typedef struct{
    int num;
    //int id;
    int tableId;
    Token *token;
    int type;
    int con;
    //
    int dim1;
    int dim2;
    //try-maybe-delete
    int dim[105][105]={0};
    //func
    int retype;
    int paramNum;
    int paramTypeList[10]={0};
    int paramTypeList_help[10] = {0};
}Symbol;

Symbol temp_symbol={0};

Symbol create_symbol(int a1,int a2,int a3,int a4,int a5,int a6,int a7,int location){
    Symbol a;
    a.tableId=a1;
    a.type = a2;
    a.con = a3;
    a.dim1 = a4;
    a.dim2 = a5;
    a.retype = a6;
    a.paramNum = a7;
    a.token=tokens[location];
    return a;
}//无法处理实际数据

typedef struct{
    //int id;
    int fid;
    std::unordered_map<std::string,Symbol> Hashmap;
}Symboltable;
Symboltable st[1000];

void s_in_st(Symbol a,Symboltable& b){
    b.Hashmap.insert(std::make_pair(a.token->value,a));
}

// 使用有限自动机解析 Token
Token *get_next_token(const char **input) {
    int len;
    const char *start = *input;
    const char *c = *input;
    // 跳过空白字符
    while (isspace(**input)) {
        if((**input) == '\n'){
            line_num++;
        }
        (*input)++;
    }
    // 检查是否到达字符串末尾
    if (**input == '\0') {
        line_num++;
        return NULL;
    }
    // 记录解析前的字符串位置
    start = *input;
    // 初始化 Token
    Token *token = (Token *)malloc(sizeof(Token));
    token->value = NULL;
    if (isdigit(**input)) { // 解析整数或浮点数
        //bool is_float = false;
        while (isdigit(**input) ) {
            //  || (!is_float && **input == '.')
            // if (**input == '.') {
            //     is_float = true;
            // }
            (*input)++;
        }
        len = *input - start;
        token->value = (char *)malloc(len + 1);
        strncpy(token->value, start, len);
        token->value[len] = '\0';

        token->type = INTCON;
    } else if (isalpha(**input) || **input == '_') {// 解析标识符或关键字
        // 解析标识符或关键字
        while (isalnum(**input) || **input == '_') {
            (*input)++;
        }
        int len = *input - start;
        token->value = (char *)malloc(len + 1);
        strncpy(token->value, start, len);
        token->value[len] = '\0';
        // 验证是否为关键字
        const char *keywords[] = {"int", "main", "const", "break", "continue","if","else","for","getint","printf","return","void"};
        size_t num_keywords = sizeof(keywords) / sizeof(keywords[0]);
        bool is_keyword = false;
        for (size_t i = 0; i < num_keywords; i++) {
            if (strcmp(token->value, keywords[i]) == 0) {
                is_keyword = true;
                if(strcmp(token->value, "int") == 0){
                    token->type = INTTK;
                }else if(strcmp(token->value, "main") == 0){
                    //printf("int main!\n");
                    token->type = MAINTK;
                }else if(strcmp(token->value, "const") == 0){
                    token->type = CONSTTK;
                }else if(strcmp(token->value, "break") == 0){
                    token->type = BREAKTK;
                }else if(strcmp(token->value, "continue") == 0){
                    token->type = CONTINUETK;
                }else if(strcmp(token->value, "if") == 0){
                    token->type = IFTK;
                }else if(strcmp(token->value, "else") == 0){
                    token->type = ELSETK;
                }else if(strcmp(token->value, "for") == 0){
                    token->type = FORTK;
                }else if(strcmp(token->value, "getint") == 0){
                    token->type = GETINTTK;
                }else if(strcmp(token->value, "printf") == 0){
                    token->type = PRINTFTK;
                }else if(strcmp(token->value, "return") == 0){
                    token->type = RETURNTK;
                }else if(strcmp(token->value, "void") == 0){
                    token->type = VOIDTK;
                }
                break;
            }
        }//处理关键字
        if(is_keyword == false){
            token->type = IDENFR;
        }
    } else {//对于其它字符的处理
        int temp_a;
        int a_err_num;
        switch (**input) {
            case '!':
                    c=*input;
                    c++;
                    if(*c == '='){
                        (*input)++;
                        token->type = NEQ;
                        len = *input - start +1; 
                        token->value = (char *)malloc(len + 1);
                        strncpy(token->value, start, len);
                        token->value[len] = '\0';
                        break;
                    }else{
                        token->type = NOT;
                        len = *input - start +1;
                        token->value = (char *)malloc(len + 1);
                        strncpy(token->value, start, len);
                        token->value[len] = '\0';
                        break;
                    }
            case '&':
                    (*input)++;
                    if(**input == '&'){
                        token->type = AND;
                        len = *input - start +1;
                        token->value = (char *)malloc(len + 1);
                        strncpy(token->value, start, len);
                        token->value[len] = '\0';
                        break;
                    }else{
                        //error();
                        break;
                    }
            case '|':
                    (*input)++;
                    if(**input == '|'){
                        token->type = OR;
                        len = *input - start +1;
                        token->value = (char *)malloc(len + 1);
                        strncpy(token->value, start, len);
                        token->value[len] = '\0';
                        break;
                    }else{
                        //error();
                        break;
                    }
            case '+':
                    token->type = PLUS;
                    len= *input - start +1;
                    token->value = (char *)malloc(len + 1);
                    strncpy(token->value, start, len);
                    token->value[len] = '\0';
                    break;
            case '-':
                    token->type = MINU;
                    len = *input - start +1;
                    token->value = (char *)malloc(len + 1);
                    strncpy(token->value, start, len);
                    token->value[len] = '\0';
                    break;
            case '*':
                    token->type = MULT;
                    len = *input - start +1;
                    token->value = (char *)malloc(len + 1);
                    strncpy(token->value, start, len);
                    token->value[len] = '\0';
                    break;
            case '/'://考虑注释
                    c=*input;
                    (c)++;
                    if(*c=='/'){
                        (*input)++;
                        while(**input!='\n'){
                            (*input)++;
                        }
                        line_num++;
                    }else if(*c == '*'){
                        (*input)++;
                        while(**input!='*' || *c!='/'){
                            if(**input == '\n'){
                                line_num++;
                            }
                            (*input)++;
                            c=*input;
                            (c)++;
                        }
                        (*input)++;
                    }else{
                    token->type = DIV;
                    len = *input - start +1;
                    token->value = (char *)malloc(len + 1);
                    strncpy(token->value, start, len);
                    token->value[len] = '\0';
                    }
                    break;
            case '%':
                    token->type = MOD;
                    len = *input - start +1;
                    token->value = (char *)malloc(len + 1);
                    strncpy(token->value, start, len);
                    token->value[len] = '\0';
                    break;
            case '<':
                    c=*input;
                    c++;
                    if(*c == '='){
                        (*input)++;
                        token->type = LEQ;
                        len = *input - start +1;
                        token->value = (char *)malloc(len + 1);
                        strncpy(token->value, start, len);
                        token->value[len] = '\0';
                        break;
                    }else{
                        token->type = LSS;
                        len = *input - start +1;
                        token->value = (char *)malloc(len + 1);
                        strncpy(token->value, start, len);
                        token->value[len] = '\0';
                        break;
                    }
            case '>':
                    c=*input;
                    c++;
                    if(*c == '='){
                        (*input)++;
                        token->type = GEQ;
                        len = *input - start +1;
                        token->value = (char *)malloc(len + 1);
                        strncpy(token->value, start, len);
                        token->value[len] = '\0';
                        break;
                    }else{
                        token->type = GRE;
                        len = *input - start +1;
                        token->value = (char *)malloc(len + 1);
                        strncpy(token->value, start, len);
                        token->value[len] = '\0';
                        break;
                    }
            case '=':
                    c=*input;
                    c++;
                    if(*c == '='){
                        (*input)++;
                        token->type = EQL;
                        len = *input - start +1;
                        token->value = (char *)malloc(len + 1);
                        strncpy(token->value, start, len);
                        token->value[len] = '\0';
                        break;
                    }else{
                        token->type = ASSIGN;
                        len = *input - start +1;
                        token->value = (char *)malloc(len + 1);
                        strncpy(token->value, start, len);
                        token->value[len] = '\0';
                        break;
                    }
            case ';':
                    token->type = SEMICN;
                    len = *input - start +1;
                    token->value = (char *)malloc(len + 1);
                    strncpy(token->value, start, len);
                    token->value[len] = '\0';
                    break;
            case ',':
                    token->type = COMMA;
                    len = *input - start +1;
                    token->value = (char *)malloc(len + 1);
                    strncpy(token->value, start, len);
                    token->value[len] = '\0';
                    break;
            case '(':
                    token->type = LPARENT;
                    len = *input - start +1;
                    token->value = (char *)malloc(len + 1);
                    strncpy(token->value, start, len);
                    token->value[len] = '\0';
                    break;
            case ')':
                    token->type = RPARENT;
                    len = *input - start +1;
                    token->value = (char *)malloc(len + 1);
                    strncpy(token->value, start, len);
                    token->value[len] = '\0';
                    break;
            case '[':
                    token->type = LBRACK;
                    len = *input - start + 1;
                    token->value = (char *)malloc(len + 1);
                    strncpy(token->value, start, len);
                    token->value[len] = '\0';
                    break;
            case ']':
                    token->type = RBRACK;
                    len = *input - start +1;
                    token->value = (char *)malloc(len + 1);
                    strncpy(token->value, start, len);
                    token->value[len] = '\0';
                    break;
            case '{':
                    token->type = LBRACE;
                    len = *input - start +1;
                    token->value = (char *)malloc(len + 1);
                    strncpy(token->value, start, len);
                    token->value[len] = '\0';
                    break;
            case '}':
                    token->type = RBRACE;
                    len = *input - start +1;
                    token->value = (char *)malloc(len + 1);
                    strncpy(token->value, start, len);
                    token->value[len] = '\0';
                    break;
            case '"':
                    num_print++;
                    a_err_num=1;
                    (*input)++;
                    c=*input;
                    while(*c != '\n'){
                        if(*c == '"'){
                            a_err_num++;
                        }
                        *c++;
                    }
                    if(a_err_num > 2){
                        token->error = 1;
                    }
                    //printf("%d\n",token->error);
                    temp_a=0;
                    while(**input!='"' || (a_err_num!=2 && a_err_num%2 == 1)){
                        temp_a=**input;
                        if(!( temp_a==32||temp_a==33||(temp_a>=40&&temp_a<=126) )){
                            c=*input;
                            *c++;
                            if(**input == '%' && *c == 'd'){
                                list_print[num_print]++;
                            }else{
                                token->error = 1;
                            }
                        }
                        temp_a = **input;
                        c = *input;
                        *c++;
                        if(temp_a == 92){
                            if(*c != 'n'){
                                token->error = 1;
                            }
                        }
                        //printf("%d\n",token->error);
                        if(**input == '"'){
                            a_err_num--;
                        }
                        (*input)++;
                    }
                    //printf("123!\n");
                    //a类错误
                    token->type = STRCON;
                    len = *input - start +1;
                    token->value = (char *)malloc(len + 1);
                    strncpy(token->value, start, len);
                    token->value[len] = '\0';
                    break;
            default:// 无法识别的字符
                    (*input)++;
                    token->type = UNKNOWN;
                    token->value = (char *)malloc(2);
                    token->value[0] = **input;
                    token->value[1] = '\0';
                    break;
        }
        // 移动字符串位置
        (*input)++;
    }
    //printf("%s\n",*input);
    token->line=line_num;
    return token;
}

//错误处理输出
void error(char a,int location){
    FILE *file;
    if(first_call_e==1){
        file = fopen("error.txt", "w+"); 
        first_call_e--;
    }else{
        file = fopen("error.txt", "a"); 
    }
    fprintf(file, "%d %c\n", tokens[location]->line,a); 
    fclose(file);
    //printf("%d %c\n", tokens[location]->line,a); 
}
//错误处理
bool find_b_error(int location){
    //printf("in fbe\n");
    //printf("%s\n",tokens[location]->value);
    int judge=0;
    std::string front;
    Symbol end;
    for (auto p : st[now_sts].Hashmap) {
        //printf("in fbe while\n");
	    front = p.first;   //key
        end = p.second;   //value
        //printf("important!!!%s\n",front.c_str());
        //printf("%s\n",tokens[location]->value);
        if(strcmp(front.c_str(),tokens[location]->value) == 0){
            //printf("%s\n",front.c_str());
            judge=1;
            break;
        }
    }
    //printf("out fbe\n");
    if(judge==1){
        error('b',location);
        judge=0;
        return true;
    }
    return false;
}
bool find_c_error(int location){
    //printf("%s\n",tokens[location]->value);
    int temp_in_c=now_sts;
    std::string front;
    Symbol end;
    //printf("in erc now_sts:%d\n",now_sts);
    while(st[temp_in_c].fid!=-1){
        for (auto p : st[temp_in_c].Hashmap) {
            front = p.first;   //key
            end = p.second;   //value
            //printf("in hash map:%s\n",front.c_str());
            if(strcmp(front.c_str(),tokens[location]->value) == 0){
                return false;
            }
        }
        temp_in_c = st[temp_in_c].fid;
    }
    for (auto p : st[temp_in_c].Hashmap) {
        front = p.first;   //key
        end = p.second;   //value
        if(strcmp(front.c_str(),tokens[location]->value) == 0){
            return false;
        }
    }
    error('c',location);
    return true;
}
bool find_d_error(int location){
    //printf("in fde:%s\n",tokens[location]->value);
    std::string front;
    Symbol end;
    for(auto p: st[0].Hashmap){
        front = p.first;   //key
        end = p.second;   //value
        if(strcmp(front.c_str(),tokens[location]->value) == 0){
            //printf("paramNum:%d,judge_fpnum:%d\n",p.second.paramNum,judge_fpnum);
            if(p.second.paramNum == judge_fpnum){
                return false;
            }else{
                error('d',location);
                return true;
            }
        }
    }
    return false;
}
int alu_judge_fpt(int location,int tool){
    int temp_in_c=now_sts;
    std::string front;
    Symbol end;
    while(st[temp_in_c].fid!=-1){
        for (auto p : st[temp_in_c].Hashmap) {
            front = p.first;   //key
            end = p.second;   //value
            if(strcmp(front.c_str(),tokens[location]->value) == 0){
                return p.second.type - tool;
            }
        }
        temp_in_c = st[temp_in_c].fid;
    }
    for (auto p : st[temp_in_c].Hashmap) {
        front = p.first;   //key
        end = p.second;   //value
        if(strcmp(front.c_str(),tokens[location]->value) == 0){
            return p.second.type - tool;
        }
    }
    return 404;
}
bool find_e_error(){
    if(f_or_brack != 1){
        return false;
    }
    if(judge_f==-1){
        return false;
    }
    std::string front;
    Symbol end;
    for(auto p: st[0].Hashmap){
        front = p.first;   //key
        end = p.second;   //value
        if(strcmp(front.c_str(),tokens[judge_f]->value) == 0 && judge_fpnum != -1){
            if(p.second.paramTypeList[judge_fpnum-1] == judge_fpt){
            }else{
                //printf("me?\n");
                error('e',judge_f);
                judge_fpt=-1;
                return true;
            }
        }
    }
    judge_fpt=-1;
    return false;
}
bool find_e_f_error(int a){
    //printf("%d\n",top_tokens);
    //printf("in fefe\n");
    if(judge_f==-1){
        return false;
    }
    std::string front;
    Symbol end;
    int ef=-1;
    for(auto p:st[0].Hashmap){
        front = p.first;   //key
        end = p.second;   //value
        if(strcmp(front.c_str(),tokens[a]->value) == 0){
            ef=p.second.retype;
        }
    }
    //printf("ef:%d\n",ef);
    if(ef==-1){
        return false;
    }
    for(auto p: st[0].Hashmap){
        front = p.first;   //key
        end = p.second;   //value
        if(strcmp(front.c_str(),tokens[judge_f]->value) == 0){
            //printf("in while %d,%d\n",p.second.paramTypeList[judge_fpnum-1],judge_fpnum);
            if(p.second.paramTypeList[judge_fpnum-1] == ef){
            }else{
                error('e',judge_f);
                judge_fpt=-1;
                return true;
            }
        }
    }
    judge_fpt=-1;
    return false;
}
bool find_f_error(int location){
    //printf("%d\n",error_f);
    if(in_main || error_f == 404){
        return false;
    }
    std::string front;
    Symbol end;
    //printf("%d\n",judge_f);
    for(auto p: st[0].Hashmap){
        front = p.first;   //key
        end = p.second;   //value
        //printf("%s\n",front.c_str());
        if(strcmp(front.c_str(),tokens[judge_f]->value) == 0){
            //printf("111:%d:%d\n",p.second.retype,error_f);
            if(p.second.retype == error_f){
            }else{
                error('f',location);
                error_f=404;
                return true;
            }
        }
    }
    error_f=404;
    return false;
}
bool find_g_error(int location){
    if(in_main == 0){
        int retype = -1;
        std::string front;
        Symbol end;
        for (auto p : st[0].Hashmap) {
            front = p.first;   //key
            end = p.second;   //value
            if(strcmp(front.c_str(),tokens[judge_f]->value) == 0){
                if(p.second.retype == 0){
                    retype = 0;
                }else{
                    retype = 1;
                }
            }
        }
        if(retype != 0){
            return false;
        }
    }
    int R_num=1;
    int L_num=0;
    int tlocation = location-1;
    //printf("%d :%s\n",tlocation,tokens[tlocation]->value);
    int use = 0;
    while( (R_num-L_num) != 0){
        if(tokens[tlocation]->type == RETURNTK && (R_num-L_num) == 1 && R_num == 1){
            use = 1;
            break;
        }
        if(tokens[tlocation]->type == LBRACE){
            L_num++;
        }
        if(tokens[tlocation]->type == RBRACE){
            R_num++;
        }
        tlocation--;
    }
    //printf("%d\n",tlocation);
    if(use != 1){
        error('g',location);
        return true;
    }else{
        // int g_fine = 0;
        // if(tokens[tlocation+1]->type != SEMICN){
        //     g_fine = 1;
        // }
        // while(tokens[tlocation]->type != SEMICN){
        //     if(tokens[tlocation]->type == RBRACE){
        //         if(g_fine == 0){
        //             error('g',location);
        //             return true;
        //         }else{
        //             break;
        //         }
        //     }
        //     tlocation++;
        // }
        // if(tokens[tlocation+1]->type!=RBRACE){
        //     error('g',location);
        //     return true;
        // }
        int location_return = tlocation;
        int exit_iden = 0;
        int exit_semi = 0;
        int static_iden = 0;
        while(tokens[tlocation]->type != RBRACE){
            if(tokens[tlocation]->type == SEMICN){   
                exit_semi++;
                if(exit_iden != 0){
                    static_iden++;
                    exit_iden = 0;
                }
            }else if(tokens[tlocation]->type == IDENFR || tokens[tlocation]->type == INTCON){
                exit_iden++;
            }
            tlocation++;
        }
        //printf("%d:%d")
        //int minu_line = tokens[tlocation]->line - tokens[location_return]->line;
        if((static_iden == 1 && exit_iden == 0) || (static_iden == 0 && exit_iden != 0)){
            return false;
        }
        error('g',location);
        return true;
    }
    return false;
}
bool find_h_error(int location,int assign){
    //printf("in fhe:%d\n",location);
    if(tokens[assign]->type != ASSIGN){
        return false;
    }
    int temp_in_c=now_sts;
    std::string front;
    Symbol end;
    while(st[temp_in_c].fid!=-1){
        for (auto p : st[temp_in_c].Hashmap) {
            front = p.first;   //key
            end = p.second;   //value
            if(strcmp(front.c_str(),tokens[location]->value) == 0){
                if(p.second.con == 1){
                    error('h',location);
                    return true;
                }
            }
        }
        temp_in_c = st[temp_in_c].fid;
    }
    for (auto p : st[temp_in_c].Hashmap) {
        front = p.first;   //key
        end = p.second;   //value
        if(strcmp(front.c_str(),tokens[location]->value) == 0){
            if(p.second.con == 1){
                    error('h',location);
                    return true;
                }
        }
    }
    return false;
}
//token输出
void print_token(const Token *token) {
    if(token->value==NULL){
        return ;
    }
    if (!token) {
        printf("END\n");
        return;
    }

    const char *type;
    switch (token->type) {
        case IDENFR: type = "IDENFR"; break;
        case INTCON: type = "INTCON"; break;
        case STRCON: type = "STRCON"; break;
        case MAINTK: type = "MAINTK"; break;
        case CONSTTK: type = "CONSTTK"; break;
        case INTTK: type = "INTTK"; break;
        case BREAKTK: type = "BREAKTK"; break;
        case CONTINUETK: type = "CONTINUETK"; break;
        case IFTK: type = "IFTK"; break;
        case ELSETK: type = "ELSETK"; break;
        case NOT: type = "NOT"; break;
        case AND: type = "AND"; break;
        case OR: type = "OR"; break;
        case FORTK: type = "FORTK"; break;
        case GETINTTK: type = "GETINTTK"; break;
        case PRINTFTK: type = "PRINTFTK"; break;
        case RETURNTK: type = "RETURNTK"; break;
        case PLUS: type = "PLUS"; break;
        case MINU: type = "MINU"; break;
        case VOIDTK: type = "VOIDTK"; break;
        case MULT: type = "MULT"; break;
        case DIV: type = "DIV"; break;
        case MOD: type = "MOD"; break;
        case LSS: type = "LSS"; break;
        case LEQ: type = "LEQ"; break;
        case GRE: type = "GRE"; break;
        case GEQ: type = "GEQ"; break;
        case EQL: type = "EQL"; break;
        case NEQ: type = "NEQ"; break;
        case ASSIGN: type = "ASSIGN"; break;
        case SEMICN: type = "SEMICN"; break;
        case COMMA: type = "COMMA"; break;
        case LPARENT: type = "LPARENT"; break;
        case RPARENT: type = "RPARENT"; break;
        case LBRACK: type = "LBRACK"; break;
        case RBRACK: type = "RBRACK"; break;
        case LBRACE: type = "LBRACE"; break;
        case RBRACE: type = "RBRACE"; break;
        default: type = "INVALID"; break;
    }

    //输出output.txt
    FILE *file;
    if(first_call==1){
        file = fopen("output.txt", "w+"); 
        first_call--;
    }else{
        file = fopen("output.txt", "a"); 
    }
    //fprintf(file, "%s %s %d\n", type, token->value,token->line); // 使用fprintf将内容写入文件
    fclose(file); // 关闭文件
    //printf("%s: %s\n",type,token->value);
}
//语法输出
void printff(const char *str){
    FILE *file;
    if(first_call==1){
        file = fopen("output.txt", "w+"); 
        first_call--;
    }else{
        file = fopen("output.txt", "a"); 
    }
    //fprintf(file, "%s\n", str); // 使用fprintf将内容写入文件
    fclose(file); // 关闭文件
}

const char *readTextFile(const char *filename) {
    // 打开文件
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        exit(1);
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 为字符串分配内存
    char *content = static_cast<char*>(malloc(fsize +1));
    if (!content) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }

    // 读取文件内容到字符串中
    fread(content, 1, fsize, file);

    // 关闭文件
    fclose(file);

    // 字符串结尾需要添加'\0'字符
    content[fsize] = '\0';

    return content;
}

//llvm输出
void writeTollvmFile(const std::string& str) {
    std::ofstream file;

    if (first_call_mid) {
        file.open("llvm_ir.txt", std::ios::out); // First time, open the file with overwrite mode 
        first_call_mid = 0;                      // Set the first_call flag to false after the first call
    } else {
        file.open("llvm_ir.txt", std::ios::app); // After the first call, open the file with append mode
    }

    if (file.is_open()) {
        file << str;
       file.close();
    } else {
        std::cout << "Unable to open file";
    }
};

void new_llvm_r(){
    llvm_r++;
}//新建一个寄存器
int find_llvm_r_in_global(const std::string& str){
    int temp_in_c=now_sts;
    std::string front;
    Symbol end;
    //printf("in erc now_sts:%d\n",now_sts);
    while(st[temp_in_c].fid!=-1){
        for (auto p : st[temp_in_c].Hashmap) {
            front = p.first;   //key
            end = p.second;   //value
            //printf("in hash map:%s\n",front.c_str());
            if(strcmp(front.c_str(),str.c_str()) == 0){
                return 0;
            }
        }
        temp_in_c = st[temp_in_c].fid;
    }
    for (auto p : st[temp_in_c].Hashmap) {
        front = p.first;   //key
        end = p.second;   //value
        if(strcmp(front.c_str(),str.c_str()) == 0){
            return 1;
        }
    }
    return 0;
}//查询相应字符的寄存器or值  ::1为全局变量，0为非全局
int get_llvm_r(const std::string& str){
    int temp_in_c=now_sts;
    std::string front;
    Symbol end;
    //printf("in erc now_sts:%d\n",now_sts);
    while(st[temp_in_c].fid!=-1){
        for (auto p : st[temp_in_c].Hashmap) {
            front = p.first;   //key
            end = p.second;   //value
            //printf("in hash map:%s\n",front.c_str());
            if(strcmp(front.c_str(),str.c_str()) == 0){
                //printf("%s???%d\n",str.c_str(),p.second.num);
                return p.second.num;
            }
        }
        temp_in_c = st[temp_in_c].fid;
    }
    for (auto p : st[temp_in_c].Hashmap) {
        front = p.first;   //key
        end = p.second;   //value
        if(strcmp(front.c_str(),str.c_str()) == 0){
            return p.second.num;
        }
    }
    return 0;
}//返回寄存器的num（非全局即为寄存器的值）
void alu_writetollvmfile(const std::string& str,int r1,int r2,int b1,int b2){
    if(b1 == 1 && b2 == 1){
        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = "+str+" i32 "+"%s"+std::to_string(r1)+", %s"+std::to_string(r2)+"\n");
        new_llvm_r();
    }else if(b1 == 1 && b2 == 0){
        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = "+str+" i32 "+"%s"+std::to_string(r1)+", "+std::to_string(r2)+"\n");
        new_llvm_r();
    }else if(b1 == 0 && b2 == 1){
        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = "+str+" i32 "+std::to_string(r1)+", %s"+std::to_string(r2)+"\n");
        new_llvm_r();
    }else if(b1 == 0 && b2 == 0){
        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = "+str+" i32 "+std::to_string(r1)+", "+std::to_string(r2)+"\n");
        new_llvm_r();
    }
}//ALU输出
void cond_writetollvmfile(const std::string& str,int r1,int r2,int b1,int b2){
    if(b1 == 1 && b2 == 1){
        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = "+"icmp "+str+" i32 "+"%s"+std::to_string(r1)+", %s"+std::to_string(r2)+"\n");
        new_llvm_r();
    }else if(b1 == 1 && b2 == 0){
        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = "+"icmp "+str+" i32 "+"%s"+std::to_string(r1)+", "+std::to_string(r2)+"\n");
        new_llvm_r();
    }else if(b1 == 0 && b2 == 1){
        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = "+"icmp "+str+" i32 "+std::to_string(r1)+", %s"+std::to_string(r2)+"\n");
        new_llvm_r();
    }else if(b1 == 0 && b2 == 0){
        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = "+"icmp "+str+" i32 "+std::to_string(r1)+", "+std::to_string(r2)+"\n");
        new_llvm_r();
    }
}
int skip_plus_lck(int location){
    while(tokens[location]->type == PLUS||tokens[location]->type == LPARENT){
        location++;
    }
    return location;
}//跳过+号和(
int skip_plus(int location){
    while(tokens[location]->type == PLUS){
        location++;
    }
    return location;
}//跳过+号
int find_global(int location){
    std::string front;
    Symbol end;
    for (auto p : st[0].Hashmap) {
        //printf("in fbe while\n");
	    front = p.first;   //key
        end = p.second;   //value
        //printf("important!!!%s\n",front.c_str());
        //printf("%s\n",tokens[location]->value);
        if(strcmp(front.c_str(),tokens[location]->value) == 0){
            //printf("%s:;%d\n",front.c_str(),p.second.num);
            return p.second.num;
        }
    }
    return 404;
}//寻找全军变量的值
int find_global_dim(int location,int dim,int dim1,int dim2){
    std::string front;
    Symbol end;
    for (auto p : st[0].Hashmap) {
        //printf("in fbe while\n");
	    front = p.first;   //key
        end = p.second;   //value
        //printf("important!!!%s\n",front.c_str());
        //printf("%s\n",tokens[location]->value);
        if(strcmp(front.c_str(),tokens[location]->value) == 0){
            //printf("%s:;%d\n",front.c_str(),p.second.num);
            if(dim == 1){
                return end.dim[0][dim1];
            }else if(dim == 2){
                return end.dim[dim1][dim2];
            }
        }
    }
    return 404;
}//寻找全局变量dim的值
void set_global_num(int location,int num){
    std::string front;
    Symbol end;
    for (auto p : st[0].Hashmap) {
        //printf("in fbe while\n");
	    front = p.first;   //key
        end = p.second;   //value
        //printf("important!!!%s\n",front.c_str());
        //printf("%s\n",tokens[location]->value);
        if(strcmp(front.c_str(),tokens[location]->value) == 0){
            st[0].Hashmap[front].num = num;
            //printf("%s:;%d\n",front.c_str(),p.second.num);
        }
    }
}//设置全局变量的值
void set_f_r(int location,int num){
    std::string front;
    Symbol end;
    for (auto p : st[now_sts].Hashmap) {
        //printf("in fbe while\n");
	    front = p.first;   //key
        end = p.second;   //value
        //printf("important!!!%s\n",front.c_str());
        //printf("%s\n",tokens[location]->value);
        if(strcmp(front.c_str(),tokens[location]->value) == 0){
            st[now_sts].Hashmap[front].num = num;
            //printf("%s!!!%d\n",front.c_str(),num);
        }
    }
}//设置函数里的寄存器，类似于int a=，为a分配一个寄存器
int precedence(std::string op){
    if(op == "+" || op == "-")
        return 1;
    if(op == "*" || op == "/" || op == "%")
        return 2;
    return 0;
}
int applyOp(int a, int b, std::string op){
    if(op == "+")
        return a + b;
    if(op == "-")
         return a - b;
    if(op == "*")
         return a * b;
    if(op == "/")
        return b != 0 ? a / b : throw std::invalid_argument("Divide by zero error.");
    if(op == "%")
         return a % b;   // % operator added for modulo operation

    throw std::invalid_argument("Unknown operator.");
}
int get_global_num(int location){
    //printf("in get num:%s\n",tokens[location]->value);
    std::stack<int> values;
    std::stack<std::string> ops;
    int first_in = 1;
    while((tokens[location]->type == INTCON || tokens[location]->type == IDENFR || tokens[location]->type == PLUS || tokens[location]->type == MINU || tokens[location]->type == MULT || tokens[location]->type == DIV ||tokens[location]->type == MOD ||tokens[location]->type == RPARENT||tokens[location]->type == LPARENT) && !(tokens[location-1]->type == IDENFR && tokens[location]->type == IDENFR)) {
        std::string str(tokens[location]->value);
        if(first_in == 1){
            if(str == "+" || str == "-" ){
                values.push(0);
            }
            first_in = 0;
        }
        if(tokens[location]->type == INTCON) {
            values.push(std::stoi(tokens[location]->value));
        }else if(tokens[location]->type == IDENFR){
            int temp = location;
            int brack = 0;
            if(tokens[location+1]->type != LBRACK){
                values.push(find_global(location));
            }else{
                location= location+2;
                brack++;
                int dim = 1;
                int dim1 = get_global_num(location);
                //printf("%d\n",dim1);
                int dim2;
                while(brack!=0){
                    if(tokens[location]->type == LBRACK){
                        brack++;
                    }
                    if(tokens[location]->type == RBRACK){
                        brack--;
                    }
                    location++;
                }
                if(tokens[location]->type == LBRACK){
                    dim++;
                    brack++;
                    dim2 = get_global_num(location+1);
                    while(brack!=0){
                        if(tokens[location]->type == LBRACK){
                            brack++;
                        }
                        if(tokens[location]->type == RBRACK){
                            brack--;
                        }
                        location++;
                    }
                }
                //printf("%d\n",find_global_dim(temp,dim,dim1,dim2));
                values.push(find_global_dim(temp,dim,dim1,dim2));
                location--;
            }
        }else if(tokens[location]->type == LPARENT){
            ops.push("(");
            if(tokens[location+1]->type == PLUS||tokens[location+1]->type == MINU){
                    values.push(0);
            }
        }else if(tokens[location]->type == RPARENT) {
            while(!ops.empty() && ops.top() != "(") {
                int val2 = values.top(); values.pop();
                int val1 = values.top(); values.pop();
                std::string op = ops.top(); ops.pop();
                values.push(applyOp(val1, val2, op));
            }
            // pop "("
            if(!ops.empty()){
                ops.pop();
            }
        }else if(str == "*" || str == "/" || str == "%") {
            if(str == "*" || str == "/"){
                int flag = 0;
                while(tokens[location+1]->type == PLUS||tokens[location+1]->type == MINU){
                    if(tokens[location+1]->type == MINU){
                        flag = flag ^ 1;
                    }
                    location++;
                }
                if(flag == 1){
                    int temp = values.top();
                    values.pop();
                    temp = -temp;
                    values.push(temp);
                }
            }else{
                while(tokens[location+1]->type == PLUS||tokens[location+1]->type == MINU){
                    location++;
                }
            }
            while(!ops.empty() && precedence(ops.top()) >= precedence(str)) {
                int val2 = values.top(); values.pop();
                int val1 = values.top(); values.pop();
                std::string op = ops.top(); ops.pop();
                values.push(applyOp(val1, val2, op));
            }
            ops.push(str);
        }else if(str == "+" || str == "-" ){
            while(tokens[location+1]->type == PLUS||tokens[location+1]->type == MINU){
                if(tokens[location+1]->type == MINU){
                    if(str == "+"){
                        str = "-";
                    }else{
                        str = "+";
                    }
                }
                location++;
            }
            while(!ops.empty() && precedence(ops.top()) >= precedence(str)) {
                int val2 = values.top(); values.pop();
                int val1 = values.top(); values.pop();
                std::string op = ops.top(); ops.pop();
                values.push(applyOp(val1, val2, op));
            }
            ops.push(str);
        }else {
            throw std::invalid_argument("Illegal token");
        }
        location++;
    }
    //printf("1\n");
    // Entire expression has been parsed, apply remaining ops to remaining values
    while(!ops.empty()) {
        int val2 = values.top(); values.pop();
        int val1 = values.top(); values.pop();
        std::string op = ops.top(); ops.pop();
        //printf("%d,%s,%d\n",val1,op.c_str(),val2);
        values.push(applyOp(val1, val2, op));
    }
    //printf("2\n");
    // Top of values contains result, if not empty
    if(values.empty())
        throw std::invalid_argument("No value found.");
    //printf("3\n");
    return values.top();
}//在添加数组后暂未满足b[1] = {a[1]};的寻找a[1]的需求
int judge_f_type(int location){
    std::string front;
    Symbol end;
    for (auto p : st[0].Hashmap) {
        //printf("in fbe while\n");
	    front = p.first;   //key
        end = p.second;   //value
        //printf("important!!!%s\n",front.c_str());
        //printf("%s\n",tokens[location]->value);
        if(strcmp(front.c_str(),tokens[location]->value) == 0){
            if(end.retype == 0){
                return 1;
            }else{
                return 0;
            }
        }
    }
    return 404;
}
int f_paramnum(int location){
    std::string front;
    Symbol end;
    for (auto p : st[0].Hashmap) {
        //printf("in fbe while\n");
	    front = p.first;   //key
        end = p.second;   //value
        //printf("important!!!%s\n",front.c_str());
        //printf("%s\n",tokens[location]->value);
        if(strcmp(front.c_str(),tokens[location]->value) == 0){
            return end.paramNum;
            //printf("%s:;%d\n",front.c_str(),p.second.num);
        }
    }
    return -1;
}
bool judge_A_if(int location){
    int flag = 0;
    int temp_frl = 0;
    while(1){
        if(tokens[location]->type == LPARENT){
            temp_frl++;
        }
        if(tokens[location]->type == RPARENT || tokens[location]->type == LBRACE || tokens[location]->type == OR||tokens[location]->type == AND){
            if(tokens[location]->type == LBRACE ||tokens[location]->type == OR||tokens[location]->type == AND){
                break;
            }else{
                if(temp_frl == 0){
                    break;
                }else{
                    temp_frl--;
                }
            }
        }
        if(tokens[location]->type == LSS||tokens[location]->type == LEQ||tokens[location]->type == GRE||tokens[location]->type == GEQ||tokens[location]->type == EQL||tokens[location]->type == NEQ){
            flag = 1;
            break;
        }
        location++;
    }
    if(flag == 1){
        return false;
    }else{
        return true;
    }
}//判断下一个是不是&&
bool go_o(int location){
    int flag = 0;
    while(1){
        if(tokens[location]->type == OR){
            flag = 1;
            break;
        }
        if(tokens[location]->type == LBRACE){
            break;
        }
        location++;
    }if(flag){
        return true;
    }else{
        return false;
    }
}
bool lookbacksafe(int location){
    int flag = 0;
    while(tokens[location]->type != LPARENT){
        if(tokens[location]->type == CONTINUETK||tokens[location]->type == BREAKTK){
            flag = 1;
        }
        location--;
    }
    if(!flag){
        return true;
    }else{
        return false;
    }
}//没有continue返回true
void change_s_dim(int location,int dim1,int dim2,int num){
    std::string front;
    Symbol end;
    for (auto p : st[0].Hashmap) {
        //printf("in fbe while\n");
	    front = p.first;   //key
        end = p.second;   //value
        //printf("important!!!%s\n",front.c_str());
        //printf("%s\n",tokens[location]->value);
        if(strcmp(front.c_str(),tokens[location]->value) == 0){
            st[0].Hashmap[front].dim[dim1][dim2] = num;
            //printf("%s!!!%d\n",front.c_str(),num);
        }
    }
}//修改location中dim数组的值
int pass_num(int location){
    while(tokens[location]->type != COMMA&&tokens[location]->type != RBRACE){
        location++;
    }  
    if(tokens[location]->type == RBRACE){
        location = location+2;
    }
    return location;
}
int judge_s_dim(int location){
    const std::string& str = tokens[location]->value;
    //printf("%s\n",str.c_str());
    int temp_in_c=now_sts;
    std::string front;
    Symbol end;
    //printf("in erc now_sts:%d\n",now_sts);
    while(st[temp_in_c].fid!=-1){
        for (auto p : st[temp_in_c].Hashmap) {
            front = p.first;   //key
            end = p.second;   //value
            //printf("in hash map:%s\n",front.c_str());
            if(strcmp(front.c_str(),str.c_str()) == 0){
                return end.type;
            }
        }
        temp_in_c = st[temp_in_c].fid;
    }
    for (auto p : st[temp_in_c].Hashmap) {
        front = p.first;   //key
        end = p.second;   //value
        if(strcmp(front.c_str(),str.c_str()) == 0){
            return end.type;
        }
    }
    return 404;
}//查看symbol的维数
int get_s_dim1(int location){
    const std::string& str = tokens[location]->value;
    int temp_in_c=now_sts;
    std::string front;
    Symbol end;
    //printf("in erc now_sts:%d\n",now_sts);
    while(st[temp_in_c].fid!=-1){
        for (auto p : st[temp_in_c].Hashmap) {
            front = p.first;   //key
            end = p.second;   //value
            //printf("in hash map:%s\n",front.c_str());
            if(strcmp(front.c_str(),str.c_str()) == 0){
                return end.dim1;
            }
        }
        temp_in_c = st[temp_in_c].fid;
    }
    for (auto p : st[temp_in_c].Hashmap) {
        front = p.first;   //key
        end = p.second;   //value
        if(strcmp(front.c_str(),str.c_str()) == 0){
            return end.dim1;
        }
    }
    return 0;
}//获取数组dim1大小
int get_s_dim2(int location){
    const std::string& str = tokens[location]->value;
    int temp_in_c=now_sts;
    std::string front;
    Symbol end;
    //printf("in erc now_sts:%d\n",now_sts);
    while(st[temp_in_c].fid!=-1){
        for (auto p : st[temp_in_c].Hashmap) {
            front = p.first;   //key
            end = p.second;   //value
            //printf("in hash map:%s\n",front.c_str());
            if(strcmp(front.c_str(),str.c_str()) == 0){
                return end.dim2;
            }
        }
        temp_in_c = st[temp_in_c].fid;
    }
    for (auto p : st[temp_in_c].Hashmap) {
        front = p.first;   //key
        end = p.second;   //value
        if(strcmp(front.c_str(),str.c_str()) == 0){
            return end.dim2;
        }
    }
    return 0;
}//获取数组dim2大小
Symbol get_f_symbol(int location){
    std::string front;
    Symbol end;
    for (auto p : st[0].Hashmap) {
        //printf("in fbe while\n");
	    front = p.first;   //key
        end = p.second;   //value
        //printf("important!!!%s\n",front.c_str());
        //printf("%s\n",tokens[location]->value);
        if(strcmp(front.c_str(),tokens[location]->value) == 0){
            //printf("%s:;%d\n",front.c_str(),p.second.num);
            return end;
        }
    }
    return end;
}
int get_eq_type(int location){
    int type = 4;
    int first_see = 1;
    while(tokens[location]->type != RPARENT){
        if(tokens[location]->type == OR){
            if(first_see == 1){
                type = 2;
                first_see--;
                break;
            }else{
                type = 1;
                break;
            }
        }
        if(tokens[location]->type == AND){
            if(first_see == 1){
                type = 3;
                first_see--;
            }
        }
        location++;
    }
    return type;
}//获取当前EqEXP的类型，连或连与：1，连或尾与：2，尾或连与：3，尾或尾与：4
int get_stack_if_2_top(){
    int temp_1 = ifstack.top();
    ifstack.pop();
    int temp_2 = ifstack.top();
    ifstack.push(temp_1);
    return temp_2;
}//获取IF栈顶的第二个值

int for_solve_eq_bug(int location){
    int flag = 0;
    int lr = 0;
    while(tokens[location]->type != AND && tokens[location]->type != OR && tokens[location]->type != EQL && tokens[location]->type != NEQ && tokens[location]->type != RPARENT && tokens[location]->type != SEMICN){
        if(tokens[location]->type == LBRACK){
            lr = 1;
            location++;
            while(lr != 0){
                if(tokens[location]->type == LBRACK){
                    lr++;
                }
                if(tokens[location]->type == RBRACK){
                    lr--;
                }
                location++;
            }
            location--;
        }
        if(tokens[location]->type == LPARENT){
            lr = 1;
            location++;
            while(lr != 0){
                if(tokens[location]->type == LPARENT){
                    lr++;
                }
                if(tokens[location]->type == RPARENT){
                    lr--;
                }
                location++;
            }
            location--;
        }
        if(tokens[location]->type == LSS||tokens[location]->type == LEQ||tokens[location]->type == GRE||tokens[location]->type == GEQ){
            flag = 1;
            break;
        }
        location++;
    }
    return flag;
}//若relexp为i1则相应函数为1


//语法分析树（一次性）
TreeNode *parse_compunit(Token **tokens);
TreeNode *parse_decl(Token **tokens);
TreeNode *parse_constdecl(Token **tokens);
TreeNode *parse_btype(Token **tokens);
TreeNode *parse_constdef(Token **tokens);
TreeNode *parse_constinitval(Token **tokens, int dim1, int dim2, int* num_p_1, int* num_p_2, int r);
TreeNode *parse_vardecl(Token **tokens);
TreeNode *parse_vardef(Token **tokens);
TreeNode *parse_initval(Token **tokens, int dim1, int dim2, int* num_p_1, int* num_p_2, int r);
TreeNode *parse_funcdef(Token **tokens);
TreeNode *parse_mainfuncdef(Token **tokens);
TreeNode *parse_functype(Token **tokens);
TreeNode *parse_funcfparams(Token **tokens);
TreeNode *parse_funcfparam(Token **tokens);
TreeNode *parse_block(Token **tokens);
TreeNode *parse_blockitem(Token **tokens);
TreeNode *parse_stmt(Token **tokens);
TreeNode *parse_forstmt(Token **tokens);
TreeNode *parse_exp(Token **tokens);
TreeNode *parse_cond(Token **tokens);
TreeNode *parse_lval(Token **tokens);
TreeNode *parse_primaryexp(Token **tokens);
TreeNode *parse_number(Token **tokens);
TreeNode *parse_unaryexp(Token **tokens);
TreeNode *parse_unaryop(Token **tokens);
TreeNode *parse_funcrparams(Token **tokens);
TreeNode *parse_mulexp(Token **tokens);
TreeNode *parse_addexp(Token **tokens);
TreeNode *parse_relexp(Token **tokens);
TreeNode *parse_eqexp(Token **tokens);
TreeNode *parse_landexp(Token **tokens);
TreeNode *parse_lorexp(Token **tokens);
TreeNode *parse_constexp(Token **tokens);

TreeNode *parse_compunit(Token **tokens){
    writeTollvmFile("declare i32 @getint()\n");
    writeTollvmFile("declare void @putint(i32)\n");
    writeTollvmFile("declare void @putch(i32)\n");
    writeTollvmFile("declare void @putstr(i8*)\n");
    writeTollvmFile("\n");
    st[top_sts].fid=-1;
    TreeNode *node = NULL;
    while(tokens[top_tokens+2]->type != LPARENT){
        node = parse_decl(tokens);
    }
    while(tokens[top_tokens+1]->type != MAINTK){
        //print_token(tokens[top_tokens]);
        node = parse_funcdef(tokens);
    }
    node = parse_mainfuncdef(tokens);
    // if(tokens[top_tokens]->type == INTTK){
    //     //node = parse_decl();
    //     flag_tokens_1=top_tokens+1;
    //     if(tokens[flag_tokens_1]->type == MAINTK){
    //         node = parse_mainfuncdef(tokens);
    //     }else{
    //         flag_tokens_2=top_tokens+2;
    //         if(tokens[flag_tokens_2]->type == LPARENT){
    //             node = parse_funcdef(tokens);
    //             node = parse_mainfuncdef(tokens);
    //         }else if(tokens[flag_tokens_2]->type == LBRACK || tokens[flag_tokens_2]->type == COMMA || tokens[flag_tokens_2]->type == SEMICN){
    //             node = parse_decl(tokens);
    //         }else{
    //             //error;
    //         }
    //     }
    // }else if(tokens[top_tokens]->type == CONSTTK){
    //     node = parse_decl(tokens);
    //     node = 
    // }else if(tokens[top_tokens]->type == VOIDTK){
    //     node = parse_funcdef(tokens);
    //     node = parse_mainfuncdef(tokens);
    // }else{
    //     //error;
    // }
    printff("<CompUnit>");
    return node;
}
TreeNode *parse_decl(Token **tokens){
    //printf("decl\n");
    TreeNode *node = NULL;
    if(tokens[top_tokens]->type == CONSTTK){
        node = parse_constdecl(tokens);
    }else if(tokens[top_tokens]->type == INTTK){
        node = parse_vardecl(tokens);
    }
    return NULL;
}//无输出
TreeNode *parse_constdecl(Token **tokens){
    //printf("constdecl\n");
    TreeNode *node = NULL;
    if(tokens[top_tokens]->type == CONSTTK){
        print_token(tokens[top_tokens]);
        top_tokens++;
        node = parse_btype(tokens);
        node = parse_constdef(tokens);
        while(tokens[top_tokens]->type == COMMA){
            print_token(tokens[top_tokens]);
            top_tokens++;
            node = parse_constdef(tokens);
        }
        if(tokens[top_tokens]->type == SEMICN){
            print_token(tokens[top_tokens]);
            top_tokens++;
        }else{
            error('i',top_tokens-1);
        }
        //多次！！！
    }
    printff("<ConstDecl>");
    return NULL;
}
TreeNode *parse_btype(Token **tokens){
    if(tokens[top_tokens]->type == INTTK){
    }else{
        //error;
    }
    print_token(tokens[top_tokens]);
    top_tokens++;
    return NULL;
}//无输出
TreeNode *parse_constdef(Token **tokens){
    TreeNode *node = NULL;
    int local_llvm_r_1;
    int temp_location;
    if(tokens[top_tokens]->type == IDENFR){
        // if(now_sts != 0){
        //     local_llvm_r_1 = llvm_r;
        //     writeTollvmFile("    %s"+std::to_string(llvm_r)+" = alloca i32\n");
        //     new_llvm_r();
        // }
        int temp_b=top_tokens;
        int dim=0;
        int global_num[105][105] = {0};
        int alu_dim[3];
        //find_b_error(tokens);
        print_token(tokens[top_tokens]);
        top_tokens++;
        //多次！！
        while(tokens[top_tokens]->type == LBRACK){
            dim++;
            print_token(tokens[top_tokens]);
            top_tokens++;
            if(tokens[top_tokens]->type == PLUS||tokens[top_tokens]->type == LPARENT||tokens[top_tokens]->type == IDENFR||tokens[top_tokens]->type == INTCON||tokens[top_tokens]->type == MINU){//+,(,ident,intconst
                alu_dim[dim] = get_global_num(top_tokens);
                node = parse_constexp(tokens);
            }else{
                //error;
            }
            if(tokens[top_tokens]->type == RBRACK){
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else{
                error('k',top_tokens-1);
            }
        }
        if(now_sts != 0){
            local_llvm_r_1 = llvm_r;
            if(dim == 0){
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = alloca i32\n");
            }else if(dim == 1){
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = alloca ["+std::to_string(alu_dim[1])+" x i32]\n");
            }else if(dim == 2){
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = alloca ["+std::to_string(alu_dim[1])+" x [ "+std::to_string(alu_dim[2])+" x i32]]\n");
            }
            new_llvm_r();
        }
        if(tokens[top_tokens]->type == ASSIGN){
            print_token(tokens[top_tokens]);
            top_tokens++;
            //printf("ready in get num\n");
            if(now_sts == 0){
                //此时考虑多维数组
                if(dim == 0){
                    global_num[0][0] = get_global_num(top_tokens);
                }else if(dim == 1){
                    int temp_top_tokens = top_tokens;
                    temp_top_tokens++;
                    //printf("%s\n",tokens[temp_top_tokens]->value);
                    for(int k=0;k<alu_dim[1];k++){
                        global_num[0][k] = get_global_num(temp_top_tokens);
                        temp_top_tokens = pass_num(temp_top_tokens);
                        temp_top_tokens++;
                    }
                }else if(dim == 2){
                    int temp_top_tokens = top_tokens;
                    temp_top_tokens=temp_top_tokens+2;
                    for(int k=0;k<alu_dim[1];k++){
                        for(int p=0;p<alu_dim[2];p++){
                            global_num[k][p] = get_global_num(temp_top_tokens);
                            temp_top_tokens = pass_num(temp_top_tokens);
                            temp_top_tokens++;
                        }
                    }
                }
            }

            //printf("out get num\n");
            temp_location = top_tokens;
            int param1 = 0,param2 = 0;
            if(dim == 0){
                node = parse_constinitval(tokens,0,0,&param1,&param2,local_llvm_r_1);
            }else if(dim == 1){
                node = parse_constinitval(tokens,alu_dim[1],0,&param1,&param2,local_llvm_r_1);
            }else if(dim == 2){
                node = parse_constinitval(tokens,alu_dim[1],alu_dim[2],&param1,&param2,local_llvm_r_1);
            }
            //node = parse_constinitval(tokens);
        }else{
            //error;
        }
        //printf("here\n");
        if(!find_b_error(temp_b)){
            s_in_st(create_symbol(now_sts,dim,1,alu_dim[1],alu_dim[2],-1,-1,temp_b),st[now_sts]);
        }
        //printf("here\n");
        if(now_sts == 0){
            if(dim == 0){
                writeTollvmFile("@"+std::string(tokens[temp_b]->value)+" = dso_local constant i32 "+std::to_string(global_num[0][0])+"\n");//@a = dso_local constant i32 5
                set_global_num(temp_b,global_num[0][0]);
            }else if(dim == 1){
                int judge_zero = 0;
                for(int k=0;k<alu_dim[1];k++){
                    judge_zero = judge_zero+global_num[0][k];
                }
                if(judge_zero != 0){
                    writeTollvmFile("@"+std::string(tokens[temp_b]->value)+" = dso_local constant ["+std::to_string(alu_dim[1])+" x i32] [");
                    int k;
                    for(k=0;k<alu_dim[1]-1;k++){
                        writeTollvmFile("i32 "+std::to_string(global_num[0][k])+", ");
                        change_s_dim(temp_b,0,k,global_num[0][k]);
                    }
                    writeTollvmFile("i32 "+std::to_string(global_num[0][k]));
                    change_s_dim(temp_b,0,k,global_num[0][k]);
                    writeTollvmFile("]\n");
                }else{
                    writeTollvmFile("@"+std::string(tokens[temp_b]->value)+" = dso_local constant ["+std::to_string(alu_dim[1])+" x i32] zeroinitializer\n");
                }
            }else if(dim == 2){
                int judge_zero = 0;
                for(int k=0;k<alu_dim[1];k++){
                    for(int p=0;p<alu_dim[2];p++){
                        judge_zero = judge_zero+global_num[k][p];
                    }
                }
                if(judge_zero != 0){
                    writeTollvmFile("@"+std::string(tokens[temp_b]->value)+" = dso_local constant ["+std::to_string(alu_dim[1])+" x ["+std::to_string(alu_dim[2])+" x i32]] [");
                    int k,p;
                    int judge_zero_2;
                    for(k=0;k<alu_dim[1]-1;k++){
                        judge_zero_2 = 0;
                        for(int tt = 0;tt<alu_dim[2];tt++){
                            judge_zero_2 = judge_zero_2+global_num[k][tt];
                        }
                        if(judge_zero_2 != 0){
                            writeTollvmFile("["+std::to_string(alu_dim[2])+" x i32] [");
                            for(p=0;p<alu_dim[2]-1;p++){
                                writeTollvmFile("i32 "+std::to_string(global_num[k][p])+", ");
                                change_s_dim(temp_b,k,p,global_num[k][p]);
                            }
                            writeTollvmFile("i32 "+std::to_string(global_num[k][p]));
                            change_s_dim(temp_b,k,p,global_num[k][p]);
                            writeTollvmFile("], ");
                        }else{
                            writeTollvmFile("["+std::to_string(alu_dim[2])+" x i32] zeroinitializer, ");
                        }
                    }
                        judge_zero_2 = 0;
                        for(int tt = 0;tt<alu_dim[2];tt++){
                            judge_zero_2 = judge_zero_2+global_num[k][tt];
                        }
                        if(judge_zero_2 != 0){
                            writeTollvmFile("["+std::to_string(alu_dim[2])+" x i32] [");
                            for(p=0;p<alu_dim[2]-1;p++){
                                writeTollvmFile("i32 "+std::to_string(global_num[k][p])+", ");
                                change_s_dim(temp_b,k,p,global_num[k][p]);
                            }
                            writeTollvmFile("i32 "+std::to_string(global_num[k][p]));
                            change_s_dim(temp_b,k,p,global_num[k][p]);
                            writeTollvmFile("]");
                        }else{
                            writeTollvmFile("["+std::to_string(alu_dim[2])+" x i32] zeroinitializer");
                        }
                    writeTollvmFile("]\n");
                }else{
                    writeTollvmFile("@"+std::string(tokens[temp_b]->value)+" = dso_local constant ["+std::to_string(alu_dim[1])+" x ["+std::to_string(alu_dim[2])+" x i32]] zeroinitializer\n");
                }
            }
        }else{
            set_f_r(temp_b,local_llvm_r_1);
        }
    }
    printff("<ConstDef>");
    return NULL;
}
TreeNode *parse_constinitval(Token **tokens, int dim1, int dim2, int* num_p_1, int* num_p_2, int r){
    int num1 = *num_p_1;
    int num2 = *num_p_2;    
    TreeNode *node = NULL;
    if(tokens[top_tokens]->type == LBRACE){
        print_token(tokens[top_tokens]);
        top_tokens++;
        if(tokens[top_tokens]->type == RBRACE){
            print_token(tokens[top_tokens]);
            top_tokens++;
        }else{
            node = parse_constinitval(tokens,dim1,dim2,num_p_1,num_p_2,r);
            while(tokens[top_tokens]->type == COMMA){
                print_token(tokens[top_tokens]);
                top_tokens++;
                node = parse_constinitval(tokens,dim1,dim2,num_p_1,num_p_2,r);
            }
            if(tokens[top_tokens]->type == RBRACE){
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else{
                //error;
            }
        }
    }else{
        int temp = llvm_r;
        node = parse_constexp(tokens);
        //此处进行定义store输出
        if(now_sts != 0){if(dim1 == 0 && dim2 == 0){
            if(temp == llvm_r){
                writeTollvmFile("    store i32 " +std::string(tokens[top_tokens-1]->value)+", i32* %s"+std::to_string(r)+"\n");//store i32 %2, i32* %1
            }else{
                writeTollvmFile("    store i32 %s" + std::to_string(llvm_r-1)+", i32* %s"+std::to_string(r)+"\n");
            }
        }else if(dim1 != 0 && dim2 == 0){
            if(temp == llvm_r){
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(dim1)+" x i32], ["+std::to_string(dim1)+" x i32]* %s"+std::to_string(r)+", i32 0, i32 "+std::to_string(num1)+"\n");//%a4 = getelementptr [3 x i32], [3 x i32]* %a1, i32 0, i32 2
                new_llvm_r();
                writeTollvmFile("    store i32 "+std::string(tokens[top_tokens-1]->value)+", i32* %s"+std::to_string(llvm_r-1)+"\n");//store i32 1, i32* %a4
            }else{
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(dim1)+" x i32], ["+std::to_string(dim1)+" x i32]* %s"+std::to_string(r)+", i32 0, i32 "+std::to_string(num1)+"\n");//%a4 = getelementptr [3 x i32], [3 x i32]* %a1, i32 0, i32 2
                new_llvm_r();
                writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-2)+", i32* %s"+std::to_string(llvm_r-1)+"\n");//store i32 1, i32* %a4
            }
        }else if(dim1 != 0 && dim2 != 0){
            if(temp == llvm_r){
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(dim1)+" x ["+std::to_string(dim2)+" x i32]], ["+std::to_string(dim1)+" x ["+std::to_string(dim2)+" x i32]]* %s"+std::to_string(r)+", i32 0, i32 "+std::to_string(num1)+", i32 "+std::to_string(num2)+"\n");//%a6 = getelementptr [3 x [3 x i32]], [3 x [3 x i32]]* %a5, i32 0, i32 0, i32 0
                new_llvm_r();
                writeTollvmFile("    store i32 "+std::string(tokens[top_tokens-1]->value)+", i32* %s"+std::to_string(llvm_r-1)+"\n");//store i32 1, i32* %a4
            }else{
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(dim1)+" x ["+std::to_string(dim2)+" x i32]], ["+std::to_string(dim1)+" x ["+std::to_string(dim2)+" x i32]]* %s"+std::to_string(r)+", i32 0, i32 "+std::to_string(num1)+", i32 "+std::to_string(num2)+"\n");//%a6 = getelementptr [3 x [3 x i32]], [3 x [3 x i32]]* %a5, i32 0, i32 0, i32 0
                new_llvm_r();
                writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-2)+", i32* %s"+std::to_string(llvm_r-1)+"\n");//store i32 1, i32* %a4
            }
        }
        }
        if(num2+1>=dim2){
            *num_p_1 = *num_p_1 +1;
            *num_p_2 = 0;
        }else{
            *num_p_2 = *num_p_2 +1;
        }
    }
    printff("<ConstInitVal>");
    return NULL;
}
TreeNode *parse_vardecl(Token **tokens){
    TreeNode *node = NULL;
    node = parse_btype(tokens);
    node = parse_vardef(tokens);
    while(tokens[top_tokens]->type == COMMA){
        print_token(tokens[top_tokens]);
        top_tokens++;
        node = parse_vardef(tokens);
    }
    if(tokens[top_tokens]->type == SEMICN){
        print_token(tokens[top_tokens]);
        top_tokens++;
    }else{
        error('i',top_tokens-1);
    }
    printff("<VarDecl>");
    return NULL;
}
TreeNode *parse_vardef(Token **tokens){
    TreeNode *node = NULL;
    int local_llvm_r_1;
    int temp_location = -1;
    if(tokens[top_tokens]->type == IDENFR){
        int temp_b=top_tokens;
        int dim=0;
        int global_num[515][515] = {0};
        int alu_dim[3];
        print_token(tokens[top_tokens]);
        top_tokens++;
        //多次！！
        while(tokens[top_tokens]->type == LBRACK){
            dim++;
            print_token(tokens[top_tokens]);
            top_tokens++;
            if(tokens[top_tokens]->type == PLUS||tokens[top_tokens]->type == LPARENT||tokens[top_tokens]->type == IDENFR||tokens[top_tokens]->type == INTCON){//+,(,ident,intconst
                alu_dim[dim] = get_global_num(top_tokens);
                node = parse_constexp(tokens);
            }else{
                //error;
            }
            if(tokens[top_tokens]->type == RBRACK){
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else{
                error('k',top_tokens-1);
            }
        }
        if(now_sts != 0){
            local_llvm_r_1 = llvm_r;
            if(dim == 0){
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = alloca i32\n");
            }else if(dim == 1){
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = alloca ["+std::to_string(alu_dim[1])+" x i32]\n");
            }else if(dim == 2){
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = alloca ["+std::to_string(alu_dim[1])+" x [ "+std::to_string(alu_dim[2])+" x i32]]\n");
            }
            new_llvm_r();
        }
        if(tokens[top_tokens]->type == ASSIGN){
            print_token(tokens[top_tokens]);
            top_tokens++;
            if(now_sts == 0){
                if(dim == 0){
                    global_num[0][0] = get_global_num(top_tokens);
                }else if(dim == 1){
                    int temp_top_tokens = top_tokens;
                    temp_top_tokens++;
                    //printf("%s\n",tokens[temp_top_tokens]->value);
                    for(int k=0;k<alu_dim[1];k++){
                        global_num[0][k] = get_global_num(temp_top_tokens);
                        temp_top_tokens = pass_num(temp_top_tokens);
                        temp_top_tokens++;
                    }
                }else if(dim == 2){
                    int temp_top_tokens = top_tokens;
                    temp_top_tokens=temp_top_tokens+2;
                    for(int k=0;k<alu_dim[1];k++){
                        for(int p=0;p<alu_dim[2];p++){
                            global_num[k][p] = get_global_num(temp_top_tokens);
                            temp_top_tokens = pass_num(temp_top_tokens);
                            temp_top_tokens++;
                        }
                    }
                }
            }
            //printf("out get num\n");
            temp_location = top_tokens;
            //倘若数组，我应该在此处
            int param1 = 0,param2 = 0;
            if(dim == 0){
                node = parse_initval(tokens,0,0,&param1,&param2,local_llvm_r_1);
            }else if(dim == 1){
                node = parse_initval(tokens,alu_dim[1],0,&param1,&param2,local_llvm_r_1);
            }else if(dim == 2){
                node = parse_initval(tokens,alu_dim[1],alu_dim[2],&param1,&param2,local_llvm_r_1);
            }
            //node = parse_initval(tokens);
        }
        if(!find_b_error(temp_b)){
            s_in_st(create_symbol(now_sts,dim,0,alu_dim[1],alu_dim[2],-1,-1,temp_b),st[now_sts]);
        }
        if(now_sts == 0){//全局变量
             if(dim == 0){
                writeTollvmFile("@"+std::string(tokens[temp_b]->value)+" = dso_local global i32 "+std::to_string(global_num[0][0])+"\n");//@a = dso_local constant i32 5
                set_global_num(temp_b,global_num[0][0]);
            }else if(dim == 1){
                int judge_zero = 0;
                for(int k=0;k<alu_dim[1];k++){
                    //judge_zero = judge_zero+global_num[0][k];
                    if(global_num[0][k] != 0 ){
                       judge_zero = 1;
                    }
                }
                if(judge_zero != 0){
                    writeTollvmFile("@"+std::string(tokens[temp_b]->value)+" = dso_local global ["+std::to_string(alu_dim[1])+" x i32] [");
                    int k;
                    for(k=0;k<alu_dim[1]-1;k++){
                        writeTollvmFile("i32 "+std::to_string(global_num[0][k])+", ");
                        change_s_dim(temp_b,0,k,global_num[0][k]);
                    }
                    writeTollvmFile("i32 "+std::to_string(global_num[0][k]));
                    change_s_dim(temp_b,0,k,global_num[0][k]);
                    writeTollvmFile("]\n");
                }else{
                    writeTollvmFile("@"+std::string(tokens[temp_b]->value)+" = dso_local global ["+std::to_string(alu_dim[1])+" x i32] zeroinitializer\n");
                }
            }else if(dim == 2){
                int judge_zero = 0;
                for(int k=0;k<alu_dim[1];k++){
                    for(int p=0;p<alu_dim[2];p++){
                        if(global_num[k][p] != 0){
                            judge_zero = 1;
                        }
                    }
                }
                if(judge_zero != 0){
                    writeTollvmFile("@"+std::string(tokens[temp_b]->value)+" = dso_local global ["+std::to_string(alu_dim[1])+" x ["+std::to_string(alu_dim[2])+" x i32]] [");
                    int k,p;
                    int judge_zero_2;
                    for(k=0;k<alu_dim[1]-1;k++){
                        judge_zero_2 = 0;
                        for(int tt = 0;tt<alu_dim[2];tt++){
                            judge_zero_2 = judge_zero_2+global_num[k][tt];
                        }
                        if(judge_zero_2 != 0){
                            writeTollvmFile("["+std::to_string(alu_dim[2])+" x i32] [");
                            for(p=0;p<alu_dim[2]-1;p++){
                                writeTollvmFile("i32 "+std::to_string(global_num[k][p])+", ");
                                change_s_dim(temp_b,k,p,global_num[k][p]);
                            }
                            writeTollvmFile("i32 "+std::to_string(global_num[k][p]));
                            change_s_dim(temp_b,k,p,global_num[k][p]);
                            writeTollvmFile("], ");
                        }else{
                            writeTollvmFile("["+std::to_string(alu_dim[2])+" x i32] zeroinitializer, ");
                        }
                    }
                        judge_zero_2 = 0;
                        for(int tt = 0;tt<alu_dim[2];tt++){
                            judge_zero_2 = judge_zero_2+global_num[k][tt];
                        }
                        if(judge_zero_2 != 0){
                            writeTollvmFile("["+std::to_string(alu_dim[2])+" x i32] [");
                            for(p=0;p<alu_dim[2]-1;p++){
                                writeTollvmFile("i32 "+std::to_string(global_num[k][p])+", ");
                                change_s_dim(temp_b,k,p,global_num[k][p]);
                            }
                            writeTollvmFile("i32 "+std::to_string(global_num[k][p]));
                            change_s_dim(temp_b,k,p,global_num[k][p]);
                            writeTollvmFile("]");
                        }else{
                            writeTollvmFile("["+std::to_string(alu_dim[2])+" x i32] zeroinitializer");
                        }
                    writeTollvmFile("]\n");
                }else{
                    writeTollvmFile("@"+std::string(tokens[temp_b]->value)+" = dso_local global ["+std::to_string(alu_dim[1])+" x ["+std::to_string(alu_dim[2])+" x i32]] zeroinitializer\n");
                }
            }
        }else{
            set_f_r(temp_b,local_llvm_r_1);
        }
    }else{
        //error;
    }
    printff("<VarDef>");
    return NULL;
}
TreeNode *parse_initval(Token **tokens, int dim1, int dim2, int* num_p_1, int* num_p_2, int r){
    int num1 = *num_p_1;
    int num2 = *num_p_2;
    TreeNode *node = NULL;
    if(tokens[top_tokens]->type == LBRACE){
        print_token(tokens[top_tokens]);
        top_tokens++;
        if(tokens[top_tokens]->type == RBRACE){
            print_token(tokens[top_tokens]);
            top_tokens++;
        }else{
            node = parse_initval(tokens,dim1,dim2,num_p_1,num_p_2,r);
            while(tokens[top_tokens]->type == COMMA){
                print_token(tokens[top_tokens]);
                top_tokens++;
                node = parse_initval(tokens,dim1,dim2,num_p_1,num_p_2,r);
            }
            if(tokens[top_tokens]->type == RBRACE){
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else{
                //error;
            }
        }
    }else{
        int temp = llvm_r;
        node = parse_exp(tokens);
        //此处进行定义store输出
        if(now_sts != 0){
            if(dim1 == 0 && dim2 == 0){
                if(temp == llvm_r){
                    writeTollvmFile("    store i32 " +std::string(tokens[top_tokens-1]->value)+", i32* %s"+std::to_string(r)+"\n");//store i32 %2, i32* %1
                }else{
                    writeTollvmFile("    store i32 %s" + std::to_string(llvm_r-1)+", i32* %s"+std::to_string(r)+"\n");
                }
            }else if(dim1 != 0 && dim2 == 0){
                if(temp == llvm_r){
                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(dim1)+" x i32], ["+std::to_string(dim1)+" x i32]* %s"+std::to_string(r)+", i32 0, i32 "+std::to_string(num1)+"\n");//%a4 = getelementptr [3 x i32], [3 x i32]* %a1, i32 0, i32 2
                    new_llvm_r();
                    writeTollvmFile("    store i32 "+std::string(tokens[top_tokens-1]->value)+", i32* %s"+std::to_string(llvm_r-1)+"\n");//store i32 1, i32* %a4
                }else{
                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(dim1)+" x i32], ["+std::to_string(dim1)+" x i32]* %s"+std::to_string(r)+", i32 0, i32 "+std::to_string(num1)+"\n");//%a4 = getelementptr [3 x i32], [3 x i32]* %a1, i32 0, i32 2
                    new_llvm_r();
                    writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-2)+", i32* %s"+std::to_string(llvm_r-1)+"\n");//store i32 1, i32* %a4
                }
            }else if(dim1 != 0 && dim2 != 0){
                if(temp == llvm_r){
                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(dim1)+" x ["+std::to_string(dim2)+" x i32]], ["+std::to_string(dim1)+" x ["+std::to_string(dim2)+" x i32]]* %s"+std::to_string(r)+", i32 0, i32 "+std::to_string(num1)+", i32 "+std::to_string(num2)+"\n");//%a6 = getelementptr [3 x [3 x i32]], [3 x [3 x i32]]* %a5, i32 0, i32 0, i32 0
                    new_llvm_r();
                    writeTollvmFile("    store i32 "+std::string(tokens[top_tokens-1]->value)+", i32* %s"+std::to_string(llvm_r-1)+"\n");//store i32 1, i32* %a4
                }else{
                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(dim1)+" x ["+std::to_string(dim2)+" x i32]], ["+std::to_string(dim1)+" x ["+std::to_string(dim2)+" x i32]]* %s"+std::to_string(r)+", i32 0, i32 "+std::to_string(num1)+", i32 "+std::to_string(num2)+"\n");//%a6 = getelementptr [3 x [3 x i32]], [3 x [3 x i32]]* %a5, i32 0, i32 0, i32 0
                    new_llvm_r();
                    writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-2)+", i32* %s"+std::to_string(llvm_r-1)+"\n");//store i32 1, i32* %a4
                }
            }
        }
        if(num2+1>=dim2){
            *num_p_1 = *num_p_1 +1;
            *num_p_2 = 0;
        }else{
            *num_p_2 = *num_p_2 +1;
        }
    }
    printff("<InitVal>");
    return NULL;
}
TreeNode *parse_funcdef(Token **tokens){
    llvm_r = 0;
    TreeNode *node = NULL;
    //printf("in funcdef\n");
    int babala = top_tokens;
    node = parse_functype(tokens);
    if(tokens[top_tokens]->type == IDENFR){
        writeTollvmFile(std::string(tokens[top_tokens]->value)+"(");
        int temp_b=top_tokens;
        temp_symbol.tableId=now_sts;
        temp_symbol.con=-1;
        temp_symbol.dim1=-1;
        temp_symbol.dim2=-1;
        print_token(tokens[top_tokens]);
        top_tokens++;
        if(tokens[top_tokens]->type == LPARENT){
            print_token(tokens[top_tokens]);
            top_tokens++;
        }else{
            //error;
        }
        if(tokens[top_tokens]->type == RPARENT){
            top_sts++;
            st[top_sts].fid=now_sts;
            now_sts=top_sts;
            fblock++;
            print_token(tokens[top_tokens]);
            top_tokens++;
        }else if(tokens[top_tokens]->type == LBRACE){
            top_sts++;
            st[top_sts].fid=now_sts;
            now_sts=top_sts;
            fblock++;
            //printf("in error_j\n");
            error('j',top_tokens-1);
        }else{
            node = parse_funcfparams(tokens);
            if(tokens[top_tokens]->type == RPARENT){
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else{
                error('j',top_tokens-1);
            }
        }
        writeTollvmFile("){\n");
        int temp_now_sts=now_sts;
        now_sts=st[now_sts].fid;
        //printf("in end\n");
        if(!find_b_error(temp_b)){
            //printf("in end\n");
            temp_symbol.token=tokens[temp_b];
            Symbol rel = temp_symbol;
            //printf("here fid:%d\n",st[now_sts].fid);
            s_in_st(rel,st[now_sts]);
            //printf("here\n");
        }
        now_sts=temp_now_sts;
        temp_symbol={0};
        judge_f=temp_b;
        //printf("in end\n");
        node = parse_block(tokens);
        if(tokens[babala]->type == VOIDTK){
            writeTollvmFile("    ret void\n");
        }
        writeTollvmFile("}\n");
        find_g_error(top_tokens-1);
        judge_f=-1;
    }else{
        //error;
    }
    printff("<FuncDef>");
    return NULL;
}
TreeNode *parse_mainfuncdef(Token **tokens){
    //printf("in main:%d\n",top_tokens);
    llvm_r = 1;
    writeTollvmFile("\n");
    writeTollvmFile("define dso_local i32 @main() {\n");
    in_main=1;
    TreeNode *node = NULL;
    if(tokens[top_tokens]->type == INTTK){
        print_token(tokens[top_tokens]);
        top_tokens++;
        if(tokens[top_tokens]->type == MAINTK){
            print_token(tokens[top_tokens]);
            top_tokens++;
            if(tokens[top_tokens]->type == LPARENT){
                print_token(tokens[top_tokens]);
                top_tokens++;
                if(tokens[top_tokens]->type == RPARENT){
                    print_token(tokens[top_tokens]);
                    top_tokens++;
                    node = parse_block(tokens);
                    find_g_error(top_tokens-1);
                }else{
                    error('j',top_tokens);
                    node = parse_block(tokens);
                    find_g_error(top_tokens-1);
                }
            }else{
                //error;
            }
        }else{
            //error;
        }
    }else{
        //error;
    }
    printff("<MainFuncDef>");
    writeTollvmFile("}");
    return NULL;
}
TreeNode *parse_functype(Token **tokens){
    TreeNode *node = NULL;
    if(tokens[top_tokens]->type == INTTK){
        temp_symbol.type=-1;
        temp_symbol.retype=0;
        print_token(tokens[top_tokens]);
        top_tokens++;
        writeTollvmFile("define dso_local i32 @");
    }else if(tokens[top_tokens]->type == VOIDTK){
        temp_symbol.type=-1;
        temp_symbol.retype=1;
        print_token(tokens[top_tokens]);
        top_tokens++;
        writeTollvmFile("define dso_local void @");
    }else{
        //error;
    }
    printff("<FuncType>");
    return NULL;
}
TreeNode *parse_funcfparams(Token **tokens){
    top_sts++;
    st[top_sts].fid=now_sts;
    now_sts=top_sts;
    fblock++;
    TreeNode *node = NULL;
    // writeTollvmFile("i32 %s"+std::to_string(llvm_r));
    // new_llvm_r();
    node = parse_funcfparam(tokens);
    while(tokens[top_tokens]->type == COMMA){
        print_token(tokens[top_tokens]);
        top_tokens++;
        temp_symbol.paramNum++;
        writeTollvmFile(", ");
        // writeTollvmFile(", i32 %s"+std::to_string(llvm_r));
        // new_llvm_r();
        node = parse_funcfparam(tokens);
    }
    temp_symbol.paramNum++;
    printff("<FuncFParams>");
    return NULL;
}
TreeNode *parse_funcfparam(Token **tokens){
    TreeNode *node = NULL;
    node = parse_btype(tokens);
    int dim=0;
    int temp_b;
    int inwhile = 0;
    int temp_f_or_brack = 0;
    int get_num = 0;
    if(tokens[top_tokens]->type == IDENFR){
        //printf("%s&%d ready in set_f_r\n",tokens[top_tokens]->value,llvm_r-1);
        temp_b=top_tokens;
        print_token(tokens[top_tokens]);
        //printf("%s\n",tokens[top_tokens]->value);
        top_tokens++;
        if(tokens[top_tokens]->type == LBRACK){
            dim++;
            //printf("pn:%d,ptl:%d\n",temp_symbol.paramNum,temp_symbol.paramTypeList[temp_symbol.paramNum]);
            temp_symbol.paramTypeList[temp_symbol.paramNum]++;
            //printf("pn:%d,ptl:%d\n",temp_symbol.paramNum,temp_symbol.paramTypeList[temp_symbol.paramNum]);
            print_token(tokens[top_tokens]);
            top_tokens++;
            if(tokens[top_tokens]->type == RBRACK){
                print_token(tokens[top_tokens]);
                top_tokens++;
                while(tokens[top_tokens]->type == LBRACK){
                    inwhile = 1;
                    if(f_or_brack != 2){
                        temp_f_or_brack = f_or_brack;
                        f_or_brack = 2;
                    }
                    dim++;
                    temp_symbol.paramTypeList[temp_symbol.paramNum]++;
                    print_token(tokens[top_tokens]);
                    top_tokens++;
                    get_num = get_global_num(top_tokens);
                    node = parse_constexp(tokens);
                    if(tokens[top_tokens]->type == RBRACK){
                        print_token(tokens[top_tokens]);
                        top_tokens++;
                    }else{
                        error('k',top_tokens-1);
                    }
                }
                if(inwhile == 1){f_or_brack = temp_f_or_brack;}
            }else{
                error('k',top_tokens-1);
                while(tokens[top_tokens]->type == LBRACK){
                    inwhile = 1;
                    if(f_or_brack != 2){
                        temp_f_or_brack = f_or_brack;
                        f_or_brack = 2;
                    }
                    dim++;
                    temp_symbol.paramTypeList[temp_symbol.paramNum]++;
                    print_token(tokens[top_tokens]);
                    top_tokens++;
                    get_num = get_global_num(top_tokens);
                    node = parse_constexp(tokens);
                    if(tokens[top_tokens]->type == RBRACK){
                        print_token(tokens[top_tokens]);
                        top_tokens++;
                    }else{
                        error('k',top_tokens-1);
                    }
                }
                if(inwhile == 1){f_or_brack = temp_f_or_brack;}
            }

        }
    }
    if(dim == 0){
        writeTollvmFile("i32 %s"+std::to_string(llvm_r));
        new_llvm_r();
        if(!find_b_error(temp_b)){
            s_in_st(create_symbol(now_sts,dim,0,-1,-1,-1,-1,temp_b),st[now_sts]);
        }
    }else if(dim == 1){
        writeTollvmFile("i32* %s"+std::to_string(llvm_r));
        new_llvm_r();
        if(!find_b_error(temp_b)){
            s_in_st(create_symbol(now_sts,dim,0,-2,-1,-1,-1,temp_b),st[now_sts]);
        }
    }else if(dim == 2){
        writeTollvmFile("["+std::to_string(get_num)+" x i32]* %s"+std::to_string(llvm_r));//[3 x i32]* %a2
        new_llvm_r();
        if(!find_b_error(temp_b)){
            s_in_st(create_symbol(now_sts,dim,0,-2,get_num,-1,-1,temp_b),st[now_sts]);
        }
        temp_symbol.paramTypeList_help[temp_symbol.paramNum] = get_num;
    }
    set_f_r(temp_b,llvm_r-1);
    printff("<FuncFParam>");
    return NULL;
}
TreeNode *parse_block(Token **tokens){
    //printf("block:%d fblock:%d\n",st[now_sts].fid,fblock);
    if(fblock==0){
        top_sts++;
        st[top_sts].fid=now_sts;
        now_sts=top_sts;
    }else{
        llvm_r++;
        if(judge_f!=-1){
            int tool = top_tokens;
            Symbol temp_symbol = get_f_symbol(judge_f);
            for(int i=0;i<temp_symbol.paramNum;i++){
                while(tokens[tool]->type != IDENFR){
                    tool--;
                }
                tool--;
            }
            int temp_tokens = tool+1;
            //printf("%s\n",tokens[temp_tokens]->value);
            for(int i=0;i<f_paramnum(judge_f);i++){
                //printf("%s,%d\n",tokens[temp_tokens]->value,temp_tokens);
                set_f_r(temp_tokens,llvm_r);
                if(temp_symbol.paramTypeList[i] == 0){
                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = alloca i32\n");//%4 = alloca i32
                    writeTollvmFile("    store i32 %s"+std::to_string(i)+", i32* %s"+std::to_string(llvm_r)+"\n");//store i32 %0, i32* %3
                    //temp_tokens = temp_tokens+3;
                }else if(temp_symbol.paramTypeList[i] == 1){
                    // %a5 = alloca i32*
	                // store i32* %a1, i32** %a5
                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = alloca i32*\n");
                    writeTollvmFile("    store i32* %s"+std::to_string(i)+", i32** %s"+std::to_string(llvm_r)+"\n");
                    //temp_tokens = temp_tokens+5;
                }else if(temp_symbol.paramTypeList[i] == 2){
                    // %a6 = alloca [2 x i32]*
	                // store [2 x i32]* %a2, [2 x i32]** %a6
                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = alloca ["+std::to_string(temp_symbol.paramTypeList_help[i])+" x i32]*\n");
                    writeTollvmFile("    store ["+std::to_string(temp_symbol.paramTypeList_help[i])+" x i32]* %s"+std::to_string(i)+", ["+std::to_string(temp_symbol.paramTypeList_help[i])+" x i32]** %s"+std::to_string(llvm_r)+"\n");
                    //temp_tokens = temp_tokens+8;
                }
                //set_f_r(top_tokens-(f_paramnum(judge_f)-1)*3-2+i*3,llvm_r);
                new_llvm_r();
                temp_tokens++;
                while(tokens[temp_tokens]->type != IDENFR){
                    temp_tokens++;
                }
            }
        }
        fblock--;
    }
    //printf("block:%d\n",st[now_sts].fid);
    TreeNode *node = NULL;
    if(tokens[top_tokens]->type == LBRACE){
        print_token(tokens[top_tokens]);
        top_tokens++;
        //可多次!!!
        while(tokens[top_tokens]->type == CONSTTK||tokens[top_tokens]->type == INTTK||tokens[top_tokens]->type == IDENFR||tokens[top_tokens]->type == LPARENT||tokens[top_tokens]->type == PLUS||tokens[top_tokens]->type == MINU||tokens[top_tokens]->type == LBRACE||tokens[top_tokens]->type == IFTK||tokens[top_tokens]->type == FORTK||tokens[top_tokens]->type == BREAKTK||tokens[top_tokens]->type == CONTINUETK||tokens[top_tokens]->type == RETURNTK||tokens[top_tokens]->type == PRINTFTK||tokens[top_tokens]->type == SEMICN||tokens[top_tokens]->type == INTCON){
            node = parse_blockitem(tokens);
            //printf("out blockitem\n");
            //print_token(tokens[top_tokens]);
        }
        if(tokens[top_tokens]->type == RBRACE){
            print_token(tokens[top_tokens]);
            top_tokens++;
        }else{
            //error;
        }
    }
    now_sts=st[now_sts].fid;
    printff("<Block>");
    return NULL;
}
TreeNode *parse_blockitem(Token **tokens){
    TreeNode *node = NULL;
    if(tokens[top_tokens]->type == CONSTTK||tokens[top_tokens]->type == INTTK){
        node = parse_decl(tokens);
    }else if(tokens[top_tokens]->type == IDENFR||tokens[top_tokens]->type == LPARENT||tokens[top_tokens]->type == PLUS||tokens[top_tokens]->type == MINU||tokens[top_tokens]->type == LBRACE||tokens[top_tokens]->type == IFTK||tokens[top_tokens]->type == FORTK||tokens[top_tokens]->type == BREAKTK||tokens[top_tokens]->type == CONTINUETK||tokens[top_tokens]->type == RETURNTK||tokens[top_tokens]->type == PRINTFTK||tokens[top_tokens]->type == SEMICN||tokens[top_tokens]->type == INTCON){
        node = parse_stmt(tokens);
    }else{
        //error;
    }
    return NULL;
}//无输出
TreeNode *parse_stmt(Token **tokens){
    TreeNode *node = NULL;
    //printf("stmt:%s\n",tokens[top_tokens]->value);
    switch(tokens[top_tokens]->type){
        case BREAKTK:{
            writeTollvmFile("    br label %B"+std::to_string(bstack.top())+"\n");
            //printf("%d\n",in_for);
            if(in_for == 0){
                error('m',top_tokens);
            }
            print_token(tokens[top_tokens]);
            top_tokens++;
            if(tokens[top_tokens]->type == SEMICN){
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else{
                error('i',top_tokens-1);
            }
            break;
        }
        case CONTINUETK:{
            writeTollvmFile("    br label %F"+std::to_string(fstack.top())+"\n");
            if(in_for == 0){
                error('m',top_tokens);
            }
            print_token(tokens[top_tokens]);
            top_tokens++;
            if(tokens[top_tokens]->type == SEMICN){
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else{
                error('i',top_tokens-1);
            }
            break;
        }
        case PRINTFTK:{
            int temp_i = top_tokens;
            print_token(tokens[top_tokens]);
            top_tokens++;
            if(tokens[top_tokens]->type == LPARENT){
                print_token(tokens[top_tokens]);
                top_tokens++;
                if(tokens[top_tokens]->type == STRCON){
                    int temp_location = top_tokens;
                    if(tokens[top_tokens]->error == 1){
                        error('a',top_tokens);
                    }//a类错误
                    print_token(tokens[top_tokens]);
                    top_tokens++;
                    int print_list[100],print_list_num = 0,print_list_type[100];
                    while(tokens[top_tokens]->type == COMMA){
                        print_token(tokens[top_tokens]);
                        top_tokens++;
                        int local_llvm_r_1 = llvm_r;
                        node = parse_exp(tokens);
                        //printf("%d\n",llvm_r);
                        if(local_llvm_r_1 != llvm_r){
                            print_list_type[print_list_num] = 1;
                            print_list[print_list_num] = llvm_r - 1;
                            print_list_num++;
                        }else{
                            print_list_type[print_list_num] = 0;
                            print_list[print_list_num] = std::stoi(tokens[top_tokens-1]->value);
                            print_list_num++;
                        }
                        list_print[num_print]--;
                    }
                    //由于为局部数组，所以直接在下面处理
                    std::string str(tokens[temp_location]->value);
                    int j=0;
                    for(int i=1;i<str.size()-1;i++){
                        //printf("%c\n",str[i]);
                        if(str[i] == '%' && str[i+1] == 'd'){
                            if(print_list_type[j] == 1){
                                writeTollvmFile("    call void @putint(i32 %s"+std::to_string(print_list[j])+")\n");
                            }else{
                                writeTollvmFile("    call void @putint(i32 "+std::to_string(print_list[j])+")\n");
                            }
                            j++;
                            //writeTollvmFile("");//call void @putint(i32 %4)
                            i++;
                        }else if(str[i] == '\\' && str[i+1] == 'n'){
                            writeTollvmFile("    call void @putch(i32 10)\n");
                            i++;
                        }else{
                            writeTollvmFile("    call void @putch(i32 "+std::to_string((int)str[i])+")\n");//call void @putch(i32 108)
                        }
                    }

                    if(tokens[top_tokens]->type == RPARENT){
                        print_token(tokens[top_tokens]);
                        top_tokens++;
                    }else{
                        error('j',top_tokens-1);
                    }
                    if(tokens[top_tokens]->type == SEMICN){
                        print_token(tokens[top_tokens]);
                        top_tokens++;
                    }else{
                        error('i',top_tokens-1);
                    }
                }else{
                    //error;
                }
            }else{
                //error;
            }
            if(list_print[num_print] != 0){
                error('l',temp_i);
            }
            num_print++;
            break;
        }
        case RETURNTK:{
            is_return = 1;
            int to_f=top_tokens;
            print_token(tokens[top_tokens]);
            top_tokens++;
            int local_llvm_r_0 = llvm_r-1;
            if(tokens[top_tokens]->type == LPARENT||tokens[top_tokens]->type == IDENFR||tokens[top_tokens]->type == PLUS||tokens[top_tokens]->type == MINU||tokens[top_tokens]->type == INTCON){
                node = parse_exp(tokens);
            }
            int local_llvm_r_1 = llvm_r-1;
            //printf("%s:%d\n",tokens[top_tokens]->value,judge_f);
            if(tokens[top_tokens]->type == SEMICN){
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else{
                error('i',top_tokens-1);
            }
            is_return = 0;
            //printf("%s\n",tokens[top_tokens]->value);
            find_f_error(to_f);
            //printf("%s\n",tokens[top_tokens]->value);
            error_f = 404;

            if(in_main == 0){
                if(judge_f_type(judge_f)){
                    //printf("?\n");
                    if(local_llvm_r_0 == local_llvm_r_1){
                        writeTollvmFile("    ret i32 "+std::string(tokens[top_tokens-2]->value)+"\n");
                    }else{
                        writeTollvmFile("    ret i32 %s"+std::to_string(llvm_r-1)+"\n");
                    }
                }else{
                    //printf("??\n");
                    writeTollvmFile("    ret void\n");//ret void
                }
            }else{
                if(local_llvm_r_0 == local_llvm_r_1){
                    writeTollvmFile("    ret i32 "+std::string(tokens[top_tokens-2]->value)+"\n");
                }else{
                    writeTollvmFile("    ret i32 %s"+std::to_string(llvm_r-1)+"\n");
                }
            }
            break;
        }
        case IFTK:{
            if_num++;
            in_if_or_for = 1;
            print_token(tokens[top_tokens]);
            top_tokens++;
            ifstack.push(llvm_if_r+1);
            ifstack.push(llvm_if_r);
            if(tokens[top_tokens]->type == LPARENT){
                print_token(tokens[top_tokens]);
                top_tokens++;
                node = parse_cond(tokens);
            }
            writeTollvmFile("    br i1 %s"+std::to_string(llvm_r - 1)+", label %if"+std::to_string(llvm_if_r)+", label %if"+std::to_string(llvm_if_r + 1) +"\n");
            llvm_if_r= llvm_if_r+2;
            int temp_llvm_if_r = llvm_if_r;
            if(tokens[top_tokens]->type == RPARENT){
                writeTollvmFile("if"+std::to_string(ifstack.top())+":\n");//true的stmt
                ifstack.pop();
                print_token(tokens[top_tokens]);
                top_tokens++;
                node = parse_stmt(tokens);
            }else{
                error('j',top_tokens-1);
                node = parse_stmt(tokens);
            }
            //writeTollvmFile("    br label %"++"\n");
            //预读有没有else
            int in_else = 0;
            if(tokens[top_tokens]->type == ELSETK){
                in_else = 1;
                if(temp_llvm_if_r == llvm_if_r){
                    if(lookbacksafe(top_tokens)){
                        writeTollvmFile("    br label %if"+std::to_string(llvm_if_r)+"\n");
                    }
                    //writeTollvmFile("    br label %if"+std::to_string(llvm_if_r)+"\n");
                }else{
                    llvm_if_r++;
                    if(lookbacksafe(top_tokens)){
                        writeTollvmFile("    br label %if"+std::to_string(llvm_if_r)+"\n");
                    }
                    //writeTollvmFile("    br label %if"+std::to_string(llvm_if_r)+"\n");
                }
                int temp = ifstack.top();
                ifstack.pop();
                ifstack.push(llvm_if_r);
                ifstack.push(temp);
                llvm_if_r++;
                //修改if栈
                writeTollvmFile("if"+std::to_string(ifstack.top())+":\n");
                ifstack.pop();
                print_token(tokens[top_tokens]);
                top_tokens++;
                node = parse_stmt(tokens);
                //if(lookbacksafe(top_tokens)){
                    writeTollvmFile("    br label %if"+std::to_string(ifstack.top())+"\n");
                //}
                writeTollvmFile("if"+std::to_string(ifstack.top())+":\n");
                ifstack.pop();
            }
            if(in_else == 0){
                //if(lookbacksafe(top_tokens)){
                    writeTollvmFile("    br label %if"+std::to_string(ifstack.top())+"\n");
                //}
                writeTollvmFile("if"+std::to_string(ifstack.top())+":\n");
                ifstack.pop();
            }
            if_num--;
            break;
        }
        case FORTK:{
            in_if_or_for = 2;
            in_for++;
            print_token(tokens[top_tokens]);
            //printf("%s\n",tokens[top_tokens]->value);
            top_tokens++;
            if(tokens[top_tokens]->type == LPARENT){
                print_token(tokens[top_tokens]);
                top_tokens++;
                //printf("%s\n",tokens[top_tokens]->value);
                if(tokens[top_tokens]->type == SEMICN){
                    print_token(tokens[top_tokens]);
                    top_tokens++;
                }else{
                    node = parse_forstmt(tokens);
                    //printf("%s\n",tokens[top_tokens-1]->value);
                    if(tokens[top_tokens]->type == SEMICN){
                        print_token(tokens[top_tokens]);
                        top_tokens++;
                    }else{
                        //error;
                    }
                }
                    cstack.push(llvm_c_r);
                    llvm_c_r++;
                    bstack.push(llvm_b_r);
                    llvm_b_r++;
                    sstack.push(llvm_s_r);
                    llvm_s_r++;
                    fstack.push(llvm_f_r);
                    llvm_f_r++;
                    writeTollvmFile("    br label %C"+std::to_string(cstack.top())+"\n");
                    writeTollvmFile("C"+std::to_string(cstack.top())+":\n");//Cx:
                    //printf("%s\n",tokens[top_tokens]->value);
                if(tokens[top_tokens]->type == SEMICN){
                    node = parse_cond(tokens);
                    writeTollvmFile("    br i1 %s"+std::to_string(llvm_r-1)+", label %S"+std::to_string(sstack.top())+", label %B"+std::to_string(bstack.top())+"\n");
                    print_token(tokens[top_tokens]);
                    top_tokens++;
                }else{
                    node = parse_cond(tokens);
                    writeTollvmFile("    br i1 %s"+std::to_string(llvm_r-1)+", label %S"+std::to_string(sstack.top())+", label %B"+std::to_string(bstack.top())+"\n");
                    if(tokens[top_tokens]->type == SEMICN){
                        print_token(tokens[top_tokens]);
                        top_tokens++;
                    }else{
                        //error;
                    }
                }
                //printf("%s\n",tokens[top_tokens]->value);
                if(tokens[top_tokens]->type == RPARENT){
                    writeTollvmFile("F"+std::to_string(fstack.top())+":\n");
                    writeTollvmFile("    br label %C"+std::to_string(cstack.top())+"\n");
                    print_token(tokens[top_tokens]);
                    top_tokens++;
                }else{
                    writeTollvmFile("F"+std::to_string(fstack.top())+":\n");
                    node = parse_forstmt(tokens);
                    writeTollvmFile("    br label %C"+std::to_string(cstack.top())+"\n");
                    if(tokens[top_tokens]->type == RPARENT){
                        print_token(tokens[top_tokens]);
                        top_tokens++;
                    }else{
                        error('j',top_tokens-1);
                    }
                }
                writeTollvmFile("S"+std::to_string(sstack.top())+":\n");
                node = parse_stmt(tokens);
                writeTollvmFile("    br label %F"+std::to_string(fstack.top())+"\n");
                writeTollvmFile("B"+std::to_string(bstack.top())+":\n");
            }
            in_for--;
            //printf("for end\n");
            //printf("%s\n",tokens[top_tokens]->value);
            cstack.pop();
            fstack.pop();
            bstack.pop();
            sstack.pop();
            break;
        }
        case LBRACE:{
            node = parse_block(tokens);
            break;
        }
        case IDENFR:{
            flag_tokens_1=top_tokens+1;
            if(tokens[flag_tokens_1]->type == LPARENT){
                node = parse_exp(tokens);
                if(tokens[top_tokens]->type == SEMICN){
                    print_token(tokens[top_tokens]);
                    top_tokens++;
                }else{
                    error('i',top_tokens-1);
                }
            }else{
                int temp_use=0;
                int L_real_dim = 0;
                //printf("i see u!\n");
                while(tokens[flag_tokens_1]->type == LBRACK || temp_use!=0){
                    if(temp_use == 0){
                        L_real_dim++;
                    }
                    while(tokens[flag_tokens_1]->type != RBRACK){
                        if(tokens[flag_tokens_1]->type == LBRACK){
                            temp_use++;
                        }
                        flag_tokens_1++;
                        if(tokens[flag_tokens_1]->type == ASSIGN){
                            break;
                        }
                    }
                    if(tokens[flag_tokens_1]->type == ASSIGN){
                        break;
                    }
                    temp_use--;
                    flag_tokens_1++;
                }
                //printf("i see u!\n");
                if(tokens[flag_tokens_1]->type == ASSIGN){
                    int temp_location_1 = top_tokens;
                    int local_llvm_r_1;
                    if(find_llvm_r_in_global(tokens[top_tokens]->value)){
                        local_llvm_r_1 = llvm_r;
                    }else{
                        local_llvm_r_1 = get_llvm_r(std::string(tokens[top_tokens]->value));
                    }
                    //int local_llvm_r_1 = get_llvm_r(std::string(tokens[top_tokens]->value));
                    // if(find_llvm_r_in_global(tokens[top_tokens]->value)){
                    //     writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32, i32* @"+std::string(tokens[top_tokens]->value)+"\n");//%18 = load i32, i32* @a
                    //     new_llvm_r();
                    // }else{
                    //     writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32, i32* %s"+std::to_string(get_llvm_r(std::string(tokens[top_tokens]->value)))+"\n");//%13 = load i32, i32* %7
                    //     new_llvm_r();
                    // }
                    node = parse_lval(tokens);
                    int store_location = llvm_r-1;
                    print_token(tokens[top_tokens]);
                    top_tokens++;
                    if(tokens[top_tokens]->type == GETINTTK){
                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = call i32 @getint()\n");//%3 = call i32 @getint()
                        new_llvm_r();
                        if(find_llvm_r_in_global(std::string(tokens[temp_location_1]->value))){
                            switch(judge_s_dim(temp_location_1)){
                                case 0:
                                    writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* @"+std::string(tokens[temp_location_1]->value)+"\n");//store i32 %3, i32* %1
                                    break;
                                case 1:
                                    switch(L_real_dim){
                                        case 0:
                                            writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+"x i32]* @"+tokens[temp_location_1]->value+"\n");//store i32* %a31, [3 x i32]* @bb
                                            break;
                                        case 1:
                                            writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a28, i32* %a26
                                            break;
                                        }
                                    break;
                                case 2:
                                    switch(L_real_dim){
                                        case 0:
                                            writeTollvmFile("    store ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+" x ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]]* @"+tokens[temp_location_1]->value+"\n");//store [3 x i32]* %a45, [3 x [3 x i32]]* @bbb
                                            break;
                                        case 1:
                                            writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(store_location)+"\n");//store i32* %a35, [5 x i32]* %a33
                                            break;
                                        case 2:
                                            writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a30, i32* %a28
                                            break;
                                    }
                                    break;
                            }
                            //writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* @"+std::string(tokens[temp_location]->value)+"\n");//store i32 %3, i32* %1
                        }else{
                            switch(judge_s_dim(temp_location_1)){
                                case 0:
                                    writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");//store i32 %3, i32* %1
                                    break;
                                case 1:
                                    switch(L_real_dim){
                                        case 0:
                                            if(get_s_dim1(temp_location_1) != -2){
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+"x i32]* %s"+std::to_string(store_location)+"\n");//store i32* %a31, [3 x i32]* @bb
                                            }else if(get_s_dim1(temp_location_1) == -2){
                                                //store i32* %a10, i32** %a5
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", i32** %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");
                                            }
                                            break;
                                        case 1:
                                            writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a28, i32* %a26
                                            break;
                                        }
                                    break;
                                case 2:
                                    switch(L_real_dim){
                                        case 0:
                                            if(get_s_dim1(temp_location_1) != -2){
                                                writeTollvmFile("    store ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+" x ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]]* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");//store [3 x i32]* %a45, [3 x [3 x i32]]* @bbb
                                            }else if(get_s_dim1(temp_location_1) == -2){
                                                //store [3 x i32]* %a10, [3 x i32]** %a6
                                                writeTollvmFile("    store ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]** %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");
                                            }
                                            break;
                                        case 1:
                                            writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(store_location)+"\n");//store i32* %a35, [5 x i32]* %a33
                                            break;
                                        case 2:
                                            writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a30, i32* %a28
                                            break;
                                    }
                                    break;
                            }
                            //writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location]->value)))+"\n");//
                        }
                        print_token(tokens[top_tokens]);
                        top_tokens++;
                        if(tokens[top_tokens]->type == LPARENT){
                            print_token(tokens[top_tokens]);
                            top_tokens++;
                        }else{
                            //error;
                        }
                        if(tokens[top_tokens]->type == RPARENT){
                            print_token(tokens[top_tokens]);
                            top_tokens++;
                        }else{
                            error('j',top_tokens-1);
                        }
                        if(tokens[top_tokens]->type == SEMICN){
                            print_token(tokens[top_tokens]);
                            top_tokens++;
                        }else{
                            error('i',top_tokens-1);
                        }
                    }else{
                        int temp_location_2 = top_tokens;
                        int temp_local_llvm = llvm_r;
                        node = parse_exp(tokens);
                        if(tokens[top_tokens]->type == SEMICN){
                            print_token(tokens[top_tokens]);
                            top_tokens++;
                        }else{
                            error('i',top_tokens-1);
                        }
                        //printf("%s,%d,%d\n",tokens[top_tokens-2]->value,local_llvm_r_1,llvm_r);
                        if(temp_local_llvm == llvm_r){
                            temp_location_2 = skip_plus(temp_location_2);
                            if(find_llvm_r_in_global(std::string(tokens[temp_location_1]->value))){
                                switch(judge_s_dim(temp_location_1)){
                                    case 0:
                                        writeTollvmFile("    store i32 "+std::string(tokens[temp_location_2]->value)+", i32* @"+std::string(tokens[temp_location_1]->value)+"\n");//store i32 %3, i32* %1
                                        break;
                                    case 1:
                                        switch(L_real_dim){
                                            case 0:
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+"x i32]* @"+tokens[temp_location_1]->value+"\n");//store i32* %a31, [3 x i32]* @bb
                                                break;
                                            case 1:
                                                writeTollvmFile("    store i32 "+std::string(tokens[temp_location_2]->value)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a28, i32* %a26
                                                break;
                                            }
                                        break;
                                    case 2:
                                        switch(L_real_dim){
                                            case 0:
                                                writeTollvmFile("    store ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+" x ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]]* @"+tokens[temp_location_1]->value+"\n");//store [3 x i32]* %a45, [3 x [3 x i32]]* @bbb
                                                break;
                                            case 1:
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(store_location)+"\n");//store i32* %a35, [5 x i32]* %a33
                                                break;
                                            case 2:
                                                writeTollvmFile("    store i32 "+std::string(tokens[temp_location_2]->value)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a30, i32* %a28
                                                break;
                                        }
                                        break;
                                }
                                //writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* @"+std::string(tokens[temp_location]->value)+"\n");//store i32 %3, i32* %1
                            }else{
                                switch(judge_s_dim(temp_location_1)){
                                    case 0:
                                        writeTollvmFile("    store i32 "+std::string(tokens[temp_location_2]->value)+", i32* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");//store i32 %3, i32* %1
                                        break;
                                    case 1:
                                        switch(L_real_dim){
                                            case 0:
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+"x i32]* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");//store i32* %a31, [3 x i32]* @bb
                                                break;
                                            case 1:
                                                writeTollvmFile("    store i32 "+std::string(tokens[temp_location_2]->value)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a28, i32* %a26
                                                break;
                                            }
                                        break;
                                    case 2:
                                        switch(L_real_dim){
                                            case 0:
                                                writeTollvmFile("    store ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+" x ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]]* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");//store [3 x i32]* %a45, [3 x [3 x i32]]* @bbb
                                                break;
                                            case 1:
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(store_location)+"\n");//store i32* %a35, [5 x i32]* %a33
                                                break;
                                            case 2:
                                                writeTollvmFile("    store i32 "+std::string(tokens[temp_location_2]->value)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a30, i32* %a28
                                                break;
                                        }
                                        break;
                                }
                                //writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location]->value)))+"\n");//
                            }
                            // temp_location_2 = skip_plus(temp_location_2);
                            // if(find_llvm_r_in_global(tokens[temp_location_2]->value)){
                            //     writeTollvmFile("    store i32 " +std::string(tokens[temp_location_2]->value)+", i32* @"+std::string(tokens[temp_location_1]->value)+"\n");//store i32 %2, i32* %1
                            // }else{
                            //     writeTollvmFile("    store i32 " +std::string(tokens[temp_location_2]->value)+", i32* %s"+std::to_string(local_llvm_r_1)+"\n");//store i32 %2, i32* %1
                            // }
                        }else{
                            if(find_llvm_r_in_global(std::string(tokens[temp_location_1]->value))){
                                switch(judge_s_dim(temp_location_1)){
                                    case 0:
                                        writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* @"+std::string(tokens[temp_location_1]->value)+"\n");//store i32 %3, i32* %1
                                        break;
                                    case 1:
                                        switch(L_real_dim){
                                            case 0:
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+"x i32]* @"+tokens[temp_location_1]->value+"\n");//store i32* %a31, [3 x i32]* @bb
                                                break;
                                            case 1:
                                                writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a28, i32* %a26
                                                break;
                                            }
                                        break;
                                    case 2:
                                        switch(L_real_dim){
                                            case 0:
                                                writeTollvmFile("    store ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+" x ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]]* @"+tokens[temp_location_1]->value+"\n");//store [3 x i32]* %a45, [3 x [3 x i32]]* @bbb
                                                break;
                                            case 1:
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(store_location)+"\n");//store i32* %a35, [5 x i32]* %a33
                                                break;
                                            case 2:
                                                writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a30, i32* %a28
                                                break;
                                        }
                                        break;
                                }
                                //writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* @"+std::string(tokens[temp_location]->value)+"\n");//store i32 %3, i32* %1
                            }else{
                            switch(judge_s_dim(temp_location_1)){
                                case 0:
                                    writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");//store i32 %3, i32* %1
                                    break;
                                case 1:
                                    switch(L_real_dim){
                                        case 0:
                                            if(get_s_dim1(temp_location_1) != -2){
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+"x i32]* %s"+std::to_string(store_location)+"\n");//store i32* %a31, [3 x i32]* @bb
                                            }else if(get_s_dim1(temp_location_1) == -2){
                                                //store i32* %a10, i32** %a5
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", i32** %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");
                                            }
                                            break;
                                        case 1:
                                            writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a28, i32* %a26
                                            break;
                                        }
                                    break;
                                case 2:
                                    switch(L_real_dim){
                                        case 0:
                                            if(get_s_dim1(temp_location_1) != -2){
                                                writeTollvmFile("    store ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+" x ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]]* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");//store [3 x i32]* %a45, [3 x [3 x i32]]* @bbb
                                            }else if(get_s_dim1(temp_location_1) == -2){
                                                //store [3 x i32]* %a10, [3 x i32]** %a6
                                                writeTollvmFile("    store ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]** %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");
                                            }
                                            break;
                                        case 1:
                                            writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(store_location)+"\n");//store i32* %a35, [5 x i32]* %a33
                                            break;
                                        case 2:
                                            writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a30, i32* %a28
                                            break;
                                    }
                                    break;
                            }
                            //writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location]->value)))+"\n");//
                        }
                            //new_llvm_r();
                        }
                    }
                }else{
                    node = parse_exp(tokens);
                        if(tokens[top_tokens]->type == SEMICN){
                            print_token(tokens[top_tokens]);
                            top_tokens++;
                        }else{
                            error('i',top_tokens-1);
                        }
                }
            }
            break;
        }
        case LPARENT:{
            node = parse_exp(tokens);
            if(tokens[top_tokens]->type == SEMICN){
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else{
                error('i',top_tokens-1);
            }
            break;
        }
        case PLUS:{
            node = parse_exp(tokens);
            if(tokens[top_tokens]->type == SEMICN){
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else{
                error('i',top_tokens-1);
            }
            break;
        }
        case MINU:{
            node = parse_exp(tokens);
            if(tokens[top_tokens]->type == SEMICN){
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else{
                error('i',top_tokens-1);
            }
            break;
        }
        case NOT:{
            node = parse_exp(tokens);
            if(tokens[top_tokens]->type == SEMICN){
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else{
                error('i',top_tokens-1);
            }
            break;
        }
        case INTCON:{
            node = parse_exp(tokens);
            if(tokens[top_tokens]->type == SEMICN){
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else{
                error('i',top_tokens-1);
            }
            break;
        }
        case SEMICN:{
            print_token(tokens[top_tokens]);
            top_tokens++;
            break;
        }
    }
    printff("<Stmt>");
    return NULL;
}
TreeNode *parse_forstmt(Token **tokens){
    TreeNode *node = NULL;
                    int flag_tokens_1 = top_tokens+1;
                    int temp_location_1 = top_tokens;
                    //printf("%s\n",tokens[top_tokens]->value);
                    int temp_use=0;
                    int L_real_dim = 0;
                    //printf("i see u!\n");
                    while(tokens[flag_tokens_1]->type == LBRACK || temp_use!=0){
                        if(temp_use == 0){
                            L_real_dim++;
                        }
                        while(tokens[flag_tokens_1]->type != RBRACK){
                            if(tokens[flag_tokens_1]->type == LBRACK){
                                temp_use++;
                            }
                            flag_tokens_1++;
                            if(tokens[flag_tokens_1]->type == ASSIGN){
                                break;
                            }
                        }
                        if(tokens[flag_tokens_1]->type == ASSIGN){
                            break;
                        }
                        temp_use--;
                        flag_tokens_1++;
                    }
                    int temp_location = top_tokens;
                    int local_llvm_r_1;
                    if(find_llvm_r_in_global(tokens[top_tokens]->value)){
                        local_llvm_r_1 = llvm_r;
                    }else{
                        local_llvm_r_1 = get_llvm_r(std::string(tokens[top_tokens]->value));
                    }
                    //int local_llvm_r_1 = get_llvm_r(std::string(tokens[top_tokens]->value));
                    // if(find_llvm_r_in_global(tokens[top_tokens]->value)){
                    //     writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32, i32* @"+std::string(tokens[top_tokens]->value)+"\n");//%18 = load i32, i32* @a
                    //     new_llvm_r();
                    // }else{
                    //     writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32, i32* %s"+std::to_string(get_llvm_r(std::string(tokens[top_tokens]->value)))+"\n");//%13 = load i32, i32* %7
                    //     new_llvm_r();
                    // }
    node = parse_lval(tokens);
    int store_location = llvm_r-1;
    if(tokens[top_tokens]->type == ASSIGN){
        print_token(tokens[top_tokens]);
        top_tokens++;
    }else{
        //error;
    }
    //node = parse_exp(tokens);
                        //writeTollvmFile(";"+std::to_string(judge_s_dim(temp_location_1))+"\n");
                        int temp_location_2 = top_tokens;
                        int temp_local_llvm = llvm_r;
                        node = parse_exp(tokens);
                        //printf("%s,%d,%d\n",tokens[top_tokens-6]->value,temp_local_llvm,judge_s_dim(temp_location_1));
                        if(temp_local_llvm == llvm_r){
                            temp_location_2 = skip_plus(temp_location_2);
                            if(find_llvm_r_in_global(std::string(tokens[temp_location_1]->value))){
                                switch(judge_s_dim(temp_location_1)){
                                    case 0:
                                        writeTollvmFile("    store i32 "+std::string(tokens[temp_location_2]->value)+", i32* @"+std::string(tokens[temp_location_1]->value)+"\n");//store i32 %3, i32* %1
                                        break;
                                    case 1:
                                        switch(L_real_dim){
                                            case 0:
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+"x i32]* @"+tokens[temp_location_1]->value+"\n");//store i32* %a31, [3 x i32]* @bb
                                                break;
                                            case 1:
                                                writeTollvmFile("    store i32 "+std::string(tokens[temp_location_2]->value)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a28, i32* %a26
                                                break;
                                            }
                                        break;
                                    case 2:
                                        switch(L_real_dim){
                                            case 0:
                                                writeTollvmFile("    store ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+" x ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]]* @"+tokens[temp_location_1]->value+"\n");//store [3 x i32]* %a45, [3 x [3 x i32]]* @bbb
                                                break;
                                            case 1:
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(store_location)+"\n");//store i32* %a35, [5 x i32]* %a33
                                                break;
                                            case 2:
                                                writeTollvmFile("    store i32 "+std::string(tokens[temp_location_2]->value)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a30, i32* %a28
                                                break;
                                        }
                                        break;
                                }
                                //writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* @"+std::string(tokens[temp_location]->value)+"\n");//store i32 %3, i32* %1
                            }else{
                                switch(judge_s_dim(temp_location_1)){
                                    case 0:
                                        writeTollvmFile("    store i32 "+std::string(tokens[temp_location_2]->value)+", i32* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");//store i32 %3, i32* %1
                                        break;
                                    case 1:
                                        switch(L_real_dim){
                                            case 0:
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+"x i32]* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");//store i32* %a31, [3 x i32]* @bb
                                                break;
                                            case 1:
                                                writeTollvmFile("    store i32 "+std::string(tokens[temp_location_2]->value)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a28, i32* %a26
                                                break;
                                            }
                                        break;
                                    case 2:
                                        switch(L_real_dim){
                                            case 0:
                                                writeTollvmFile("    store ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+" x ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]]* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");//store [3 x i32]* %a45, [3 x [3 x i32]]* @bbb
                                                break;
                                            case 1:
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(store_location)+"\n");//store i32* %a35, [5 x i32]* %a33
                                                break;
                                            case 2:
                                                writeTollvmFile("    store i32 "+std::string(tokens[temp_location_2]->value)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a30, i32* %a28
                                                break;
                                        }
                                        break;
                                }
                                //writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location]->value)))+"\n");//
                            }
                            // temp_location_2 = skip_plus(temp_location_2);
                            // if(find_llvm_r_in_global(tokens[temp_location_2]->value)){
                            //     writeTollvmFile("    store i32 " +std::string(tokens[temp_location_2]->value)+", i32* @"+std::string(tokens[temp_location_1]->value)+"\n");//store i32 %2, i32* %1
                            // }else{
                            //     writeTollvmFile("    store i32 " +std::string(tokens[temp_location_2]->value)+", i32* %s"+std::to_string(local_llvm_r_1)+"\n");//store i32 %2, i32* %1
                            // }
                        }else{
                            if(find_llvm_r_in_global(std::string(tokens[temp_location_1]->value))){
                                switch(judge_s_dim(temp_location_1)){
                                    case 0:
                                        writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* @"+std::string(tokens[temp_location_1]->value)+"\n");//store i32 %3, i32* %1
                                        break;
                                    case 1:
                                        switch(L_real_dim){
                                            case 0:
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+"x i32]* @"+tokens[temp_location_1]->value+"\n");//store i32* %a31, [3 x i32]* @bb
                                                break;
                                            case 1:
                                                writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a28, i32* %a26
                                                break;
                                            }
                                        break;
                                    case 2:
                                        switch(L_real_dim){
                                            case 0:
                                                writeTollvmFile("    store ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+" x ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]]* @"+tokens[temp_location_1]->value+"\n");//store [3 x i32]* %a45, [3 x [3 x i32]]* @bbb
                                                break;
                                            case 1:
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(store_location)+"\n");//store i32* %a35, [5 x i32]* %a33
                                                break;
                                            case 2:
                                                writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a30, i32* %a28
                                                break;
                                        }
                                        break;
                                }
                                //writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* @"+std::string(tokens[temp_location]->value)+"\n");//store i32 %3, i32* %1
                            }else{
                            switch(judge_s_dim(temp_location_1)){
                                case 0:
                                    writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");//store i32 %3, i32* %1
                                    break;
                                case 1:
                                    switch(L_real_dim){
                                        case 0:
                                            if(get_s_dim1(temp_location_1) != -2){
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+"x i32]* %s"+std::to_string(store_location)+"\n");//store i32* %a31, [3 x i32]* @bb
                                            }else if(get_s_dim1(temp_location_1) == -2){
                                                //store i32* %a10, i32** %a5
                                                writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", i32** %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");
                                            }
                                            break;
                                        case 1:
                                            writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a28, i32* %a26
                                            break;
                                        }
                                    break;
                                case 2:
                                    switch(L_real_dim){
                                        case 0:
                                            if(get_s_dim1(temp_location_1) != -2){
                                                writeTollvmFile("    store ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim1(temp_location_1))+" x ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]]* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");//store [3 x i32]* %a45, [3 x [3 x i32]]* @bbb
                                            }else if(get_s_dim1(temp_location_1) == -2){
                                                //store [3 x i32]* %a10, [3 x i32]** %a6
                                                writeTollvmFile("    store ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]** %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location_1]->value)))+"\n");
                                            }
                                            break;
                                        case 1:
                                            writeTollvmFile("    store i32* %s"+std::to_string(llvm_r-1)+", ["+std::to_string(get_s_dim2(temp_location_1))+" x i32]* %s"+std::to_string(store_location)+"\n");//store i32* %a35, [5 x i32]* %a33
                                            break;
                                        case 2:
                                            writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(store_location-1)+"\n");//store i32 %a30, i32* %a28
                                            break;
                                    }
                                    break;
                            }
                            //writeTollvmFile("    store i32 %s"+std::to_string(llvm_r-1)+", i32* %s"+std::to_string(get_llvm_r(std::string(tokens[temp_location]->value)))+"\n");//
                        }
                            //new_llvm_r();
                        }
    printff("<ForStmt>");
    return NULL;
}
TreeNode *parse_exp(Token **tokens){
    TreeNode *node = NULL;
    node = parse_addexp(tokens);
    printff("<Exp>");
    //printf("end exp\n");
    return NULL;
}
TreeNode *parse_cond(Token **tokens){
    TreeNode *node = NULL;
    node = parse_lorexp(tokens);
    printff("<Cond>");
    return NULL;
}
TreeNode *parse_lval(Token **tokens){
    int first_token = top_tokens;
    //printf("%s,vs,%s\n",tokens[first_token]->value,tokens[top_tokens]->value);
    int real_dim = 0;
    int real_dim_num[3] = {0};
    int b[3] = {0};
            // if(now_sts != 0){
            //     if(find_llvm_r_in_global(tokens[top_tokens]->value)){
            //         writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32, i32* @"+std::string(tokens[top_tokens]->value)+"\n");//%18 = load i32, i32* @a
            //         new_llvm_r();
            //     }else{
            //         writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32, i32* %s"+std::to_string(get_llvm_r(std::string(tokens[top_tokens]->value)))+"\n");//%13 = load i32, i32* %7
            //         new_llvm_r();
            //     }
            // }
    TreeNode *node = NULL;
    int tool_t_judge_fpt=-1;
    int temp_c;
    int temp_f_or_brack = 0;
    int inwhile = 0;
    //printf("lval\n");
    if(tokens[top_tokens]->type == IDENFR){
        temp_c=top_tokens;
        tool_t_judge_fpt++;
        print_token(tokens[top_tokens]);
        top_tokens++;
        while(tokens[top_tokens]->type == LBRACK){
            real_dim++;
            inwhile = 1;
            if(f_or_brack != 2){
                temp_f_or_brack = f_or_brack;
                f_or_brack = 2;
            }
            tool_t_judge_fpt++;
            print_token(tokens[top_tokens]);
            top_tokens++;
            int local_llvm = llvm_r - 1;
            node = parse_exp(tokens);//数组梦魇
            if(local_llvm == llvm_r - 1){
                b[real_dim] = 1;//为数字
                real_dim_num[real_dim] = std::stoi(tokens[top_tokens-1]->value);
            }else{
                real_dim_num[real_dim] = llvm_r - 1;
            }
            if(tokens[top_tokens]->type == RBRACK){
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else{
                error('k',top_tokens-1);
            }
        }
        if(inwhile == 1){f_or_brack = temp_f_or_brack;}
    }
    //printf("lval\n");
    judge_fpt = alu_judge_fpt(temp_c,tool_t_judge_fpt);
    if(!find_c_error(temp_c)){
        find_h_error(temp_c,top_tokens);
    }
    find_e_error();
    if(is_return){
        error_f=0;
    }
    //printf("top tokens:%d\n",top_tokens);
    //printf("%d,%d,%d,%d\n",now_sts,judge_s_dim(first_token),real_dim,b[1]);
            if(now_sts != 0){
                if(find_llvm_r_in_global(tokens[first_token]->value)){
                    switch(judge_s_dim(first_token)){
                        case 0:
                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32, i32* @"+std::string(tokens[first_token]->value)+"\n");//%18 = load i32, i32* @a
                            new_llvm_r();
                            break;
                        case 1:
                            switch(real_dim){
                                case 0:
                                    //%a17 = getelementptr [3 x i32], [3 x i32]* @aa, i32 0, i32 0
                                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x i32], ["+std::to_string(get_s_dim1(first_token))+" x i32]* @"+tokens[first_token]->value+", i32 0, i32 0\n");
                                    new_llvm_r();
                                    break;
                                case 1:
                                    if(b[1] == 0){ 
                                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x i32], ["+std::to_string(get_s_dim1(first_token))+" x i32]* @"+tokens[first_token]->value+", i32 0, i32 %s"+std::to_string(real_dim_num[1])+"\n");//%a4 = getelementptr [3 x i32], [3 x i32]* @a, i32 0, i32 %a3
                                    }else{
                                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x i32], ["+std::to_string(get_s_dim1(first_token))+" x i32]* @"+tokens[first_token]->value+", i32 0, i32 "+std::to_string(real_dim_num[1])+"\n");
                                    }
                                    new_llvm_r();
                                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32, i32* %s"+std::to_string(llvm_r-1)+"\n");//%a5 = load i32, i32* %a4
                                    new_llvm_r();
                                    break;
                            }
                            break;
                        case 2:
                            switch(real_dim){
                                case 0:
                                    //%a33 = getelementptr [3 x [3 x i32]], [3 x [3 x i32]]* @aaa, i32 0, i32 0
                                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]], ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]]* @"+tokens[first_token]->value+", i32 0, i32 0\n");
                                    new_llvm_r();
                                    break;
                                case 1:
                                    if(b[1] == 0){
                                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]], ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]]* @"+tokens[first_token]->value+", i32 0, i32 %s"+std::to_string(real_dim_num[1])+"\n");//%a31 = getelementptr [3 x [3 x i32]], [3 x [3 x i32]]* @aaa, i32 0, i32 0
                                        new_llvm_r();
                                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim2(first_token))+" x i32], ["+std::to_string(get_s_dim2(first_token))+" x i32]* %s"+std::to_string(llvm_r-1)+", i32 0, i32 0\n");//%a32 = getelementptr [3 x i32], [3 x i32]* %a31, i32 0, i32 0
                                    }else{
                                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]], ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]]* @"+tokens[first_token]->value+", i32 0, i32 "+std::to_string(real_dim_num[1])+"\n");//%a31 = getelementptr [3 x [3 x i32]], [3 x [3 x i32]]* @aaa, i32 0, i32 0
                                        new_llvm_r();
                                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim2(first_token))+" x i32], ["+std::to_string(get_s_dim2(first_token))+" x i32]* %s"+std::to_string(llvm_r-1)+", i32 0, i32 0\n");//%a32 = getelementptr [3 x i32], [3 x i32]* %a31, i32 0, i32 0
                                    }
                                    new_llvm_r();
                                    break;
                                case 2:
                                    if(b[1] == 0 && b[2] == 0){
                                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]], ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]]* @"+tokens[first_token]->value+", i32 0, i32 %s"+std::to_string(real_dim_num[1])+", i32 %s"+std::to_string(real_dim_num[2])+"\n");//%a26 = getelementptr [3 x [3 x i32]], [3 x [3 x i32]]* @aaa, i32 0, i32 %0, i32 %0
                                    }else if(b[1] == 0 && b[2] == 1){
                                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]], ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]]* @"+tokens[first_token]->value+", i32 0, i32 %s"+std::to_string(real_dim_num[1])+", i32 "+std::to_string(real_dim_num[2])+"\n");
                                    }else if(b[1] == 1 && b[2] == 0){
                                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]], ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]]* @"+tokens[first_token]->value+", i32 0, i32 "+std::to_string(real_dim_num[1])+", i32 %s"+std::to_string(real_dim_num[2])+"\n");
                                    }else if(b[1] == 1 && b[2] == 1){
                                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]], ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]]* @"+tokens[first_token]->value+", i32 0, i32 "+std::to_string(real_dim_num[1])+", i32 "+std::to_string(real_dim_num[2])+"\n");
                                    }
                                    new_llvm_r();
                                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32, i32* %s"+std::to_string(llvm_r-1)+"\n");//%a27 = load i32, i32* %a26
                                    new_llvm_r();
                                    break;
                            }
                            break;
                    }
                }else{
                    switch(judge_s_dim(first_token)){
                        case 0:
                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32, i32* %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+"\n");//%18 = load i32, i32* @a
                            new_llvm_r();
                            break;
                        case 1:
                            switch(real_dim){
                                case 0:
                                    //%a17 = getelementptr [3 x i32], [3 x i32]* @aa, i32 0, i32 0
                                    if(get_s_dim1(first_token) != -2){
                                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x i32], ["+std::to_string(get_s_dim1(first_token))+" x i32]* %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+", i32 0, i32 0\n");
                                    }else if(get_s_dim1(first_token) == -2){
                                        //%a31 = load i32*, i32** %a5
                                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32*, i32** %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+"\n");
                                    }
                                    new_llvm_r();
                                    break;
                                case 1:
                                    if(get_s_dim1(first_token) != -2){
                                        if(b[1] == 0){ 
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x i32], ["+std::to_string(get_s_dim1(first_token))+" x i32]* %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+", i32 0, i32 %s"+std::to_string(real_dim_num[1])+"\n");//%a4 = getelementptr [3 x i32], [3 x i32]* @a, i32 0, i32 %a3
                                        }else{
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x i32], ["+std::to_string(get_s_dim1(first_token))+" x i32]* %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+", i32 0, i32 "+std::to_string(real_dim_num[1])+"\n");
                                        }
                                    }else if(get_s_dim1(first_token) == -2){
                                        // %a12 = load i32*, i32** %a5
	                                    // %a13 = getelementptr i32, i32* %a12, i32 1
                                        if(b[1] == 0){
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32*, i32** %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+"\n");
                                            new_llvm_r();
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr i32 , i32* %s"+std::to_string(llvm_r-1)+", i32 %s"+std::to_string(real_dim_num[1])+"\n");
                                        }else{
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32*, i32** %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+"\n");
                                            new_llvm_r();
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr i32, i32* %s"+std::to_string(llvm_r-1)+", i32 "+std::to_string(real_dim_num[1])+"\n");
                                        }
                                    }
                                    new_llvm_r();
                                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32, i32* %s"+std::to_string(llvm_r-1)+"\n");//%a5 = load i32, i32* %a4
                                    new_llvm_r();
                                    break;
                            }
                            break;
                        case 2:
                            switch(real_dim){
                                case 0:
                                    //%a33 = getelementptr [3 x [3 x i32]], [3 x [3 x i32]]* @aaa, i32 0, i32 0
                                    if(get_s_dim1(first_token) != -2){
                                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]], ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]]* %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+", i32 0, i32 0\n");
                                    }else if(get_s_dim1(first_token) == -2){
                                        //%a11 = load [3 x i32]*, [3 x i32]** %a6
                                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load ["+std::to_string(get_s_dim2(first_token))+" x i32]*, ["+std::to_string(get_s_dim2(first_token))+" x i32]** %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+"\n");
                                    }
                                    new_llvm_r();
                                    break;
                                case 1:
                                    if(get_s_dim1(first_token) != -2){
                                        if(b[1] == 0){
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]], ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]]* %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+", i32 0, i32 %s"+std::to_string(real_dim_num[1])+"\n");//%a31 = getelementptr [3 x [3 x i32]], [3 x [3 x i32]]* @aaa, i32 0, i32 0
                                            new_llvm_r();
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim2(first_token))+" x i32], ["+std::to_string(get_s_dim2(first_token))+" x i32]* %s"+std::to_string(llvm_r-1)+", i32 0, i32 0\n");//%a32 = getelementptr [3 x i32], [3 x i32]* %a31, i32 0, i32 0
                                        }else{
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]], ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]]* %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+", i32 0, i32 "+std::to_string(real_dim_num[1])+"\n");//%a31 = getelementptr [3 x [3 x i32]], [3 x [3 x i32]]* @aaa, i32 0, i32 0
                                            new_llvm_r();
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim2(first_token))+" x i32], ["+std::to_string(get_s_dim2(first_token))+" x i32]* %s"+std::to_string(llvm_r-1)+", i32 0, i32 0\n");//%a32 = getelementptr [3 x i32], [3 x i32]* %a31, i32 0, i32 0
                                        }
                                    }else if(get_s_dim1(first_token) == -2){
                                        if(b[1] == 0){
                                            // %a12 = load [3 x i32]*, [3 x i32]** %a6
                                            // %a13 = getelementptr [3 x i32], [3 x i32]* %a12, i32 3
                                            // %a14 = getelementptr [3 x i32], [3 x i32]* %a13, i32 0, i32 0    
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load ["+std::to_string(get_s_dim2(first_token))+" x i32]*, ["+std::to_string(get_s_dim2(first_token))+" x i32]** %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+"\n");
                                            new_llvm_r();                                    
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim2(first_token))+" x i32], ["+std::to_string(get_s_dim2(first_token))+" x i32]* %s"+std::to_string(llvm_r-1)+", i32 %s"+std::to_string(real_dim_num[1])+"\n");
                                            new_llvm_r();
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim2(first_token))+" x i32], ["+std::to_string(get_s_dim2(first_token))+" x i32]* %s"+std::to_string(llvm_r-1)+", i32 0, i32 0\n");
                                        }else{
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load ["+std::to_string(get_s_dim2(first_token))+" x i32]*, ["+std::to_string(get_s_dim2(first_token))+" x i32]** %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+"\n");
                                            new_llvm_r();                                    
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim2(first_token))+" x i32], ["+std::to_string(get_s_dim2(first_token))+" x i32]* %s"+std::to_string(llvm_r-1)+", i32 "+std::to_string(real_dim_num[1])+"\n");
                                            new_llvm_r();
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim2(first_token))+" x i32], ["+std::to_string(get_s_dim2(first_token))+" x i32]* %s"+std::to_string(llvm_r-1)+", i32 0, i32 0\n");
                                        }
                                    }
                                    new_llvm_r();
                                    break;
                                case 2:
                                    if(get_s_dim1(first_token) != -2){
                                        if(b[1] == 0 && b[2] == 0){
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]], ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]]* %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+", i32 0, i32 %s"+std::to_string(real_dim_num[1])+", i32 %s"+std::to_string(real_dim_num[2])+"\n");//%a26 = getelementptr [3 x [3 x i32]], [3 x [3 x i32]]* @aaa, i32 0, i32 %0, i32 %0
                                        }else if(b[1] == 0 && b[2] == 1){
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]], ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]]* %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+", i32 0, i32 %s"+std::to_string(real_dim_num[1])+", i32 "+std::to_string(real_dim_num[2])+"\n");
                                        }else if(b[1] == 1 && b[2] == 0){
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]], ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]]* %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+", i32 0, i32 "+std::to_string(real_dim_num[1])+", i32 %s"+std::to_string(real_dim_num[2])+"\n");
                                        }else if(b[1] == 1 && b[2] == 1){
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]], ["+std::to_string(get_s_dim1(first_token))+" x ["+std::to_string(get_s_dim2(first_token))+" x i32]]* %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+", i32 0, i32 "+std::to_string(real_dim_num[1])+", i32 "+std::to_string(real_dim_num[2])+"\n");
                                        }
                                    }else if(get_s_dim1(first_token) == -2){
                                        // %a13 = load [3 x i32]*, [3 x i32]** %a6
	                                    // %a14 = getelementptr [3 x i32], [3 x i32]* %a13, i32 2, i32 1
                                        if(b[1] == 0 && b[2] == 0){
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load ["+std::to_string(get_s_dim2(first_token))+" x i32]*, ["+std::to_string(get_s_dim2(first_token))+" x i32]** %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+"\n");
                                            new_llvm_r();
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim2(first_token))+" x i32], ["+std::to_string(get_s_dim2(first_token))+" x i32]* %s"+std::to_string(llvm_r-1)+", i32 %s"+std::to_string(real_dim_num[1])+", i32 %s"+std::to_string(real_dim_num[2])+"\n");
                                        }else if(b[1] == 0 && b[2] == 1){
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load ["+std::to_string(get_s_dim2(first_token))+" x i32]*, ["+std::to_string(get_s_dim2(first_token))+" x i32]** %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+"\n");
                                            new_llvm_r();
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim2(first_token))+" x i32], ["+std::to_string(get_s_dim2(first_token))+" x i32]* %s"+std::to_string(llvm_r-1)+", i32 %s"+std::to_string(real_dim_num[1])+", i32 "+std::to_string(real_dim_num[2])+"\n");
                                        }else if(b[1] == 1 && b[2] == 0){
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load ["+std::to_string(get_s_dim2(first_token))+" x i32]*, ["+std::to_string(get_s_dim2(first_token))+" x i32]** %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+"\n");
                                            new_llvm_r();
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim2(first_token))+" x i32], ["+std::to_string(get_s_dim2(first_token))+" x i32]* %s"+std::to_string(llvm_r-1)+", i32 "+std::to_string(real_dim_num[1])+", i32 %s"+std::to_string(real_dim_num[2])+"\n");
                                        }else if(b[1] == 1 && b[2] == 1){
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load ["+std::to_string(get_s_dim2(first_token))+" x i32]*, ["+std::to_string(get_s_dim2(first_token))+" x i32]** %s"+std::to_string(get_llvm_r(std::string(tokens[first_token]->value)))+"\n");
                                            new_llvm_r();
                                            writeTollvmFile("    %s"+std::to_string(llvm_r)+" = getelementptr ["+std::to_string(get_s_dim2(first_token))+" x i32], ["+std::to_string(get_s_dim2(first_token))+" x i32]* %s"+std::to_string(llvm_r-1)+", i32 "+std::to_string(real_dim_num[1])+", i32 "+std::to_string(real_dim_num[2])+"\n");
                                        }
                                    }
                                    new_llvm_r();
                                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32, i32* %s"+std::to_string(llvm_r-1)+"\n");//%a27 = load i32, i32* %a26
                                    new_llvm_r();
                                    break;
                            }
                            break;
                    }
                }
            }
    printff("<LVal>");
    return NULL;
}
TreeNode *parse_primaryexp(Token **tokens){
    TreeNode *node = NULL;
    switch(tokens[top_tokens]->type){
        case LPARENT:{
            print_token(tokens[top_tokens]);
            top_tokens++;
            node = parse_exp(tokens);
            if(tokens[top_tokens]->type == RPARENT){
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else{
                error('j',top_tokens);
            }
            break;
        }
        case IDENFR:{
            //printf("here\n");
            // if(now_sts != 0){
            //     if(find_llvm_r_in_global(tokens[top_tokens]->value)){
            //         writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32, i32* @"+std::string(tokens[top_tokens]->value)+"\n");//%18 = load i32, i32* @a
            //         new_llvm_r();
            //     }else{
            //         writeTollvmFile("    %s"+std::to_string(llvm_r)+" = load i32, i32* %s"+std::to_string(get_llvm_r(std::string(tokens[top_tokens]->value)))+"\n");//%13 = load i32, i32* %7
            //         new_llvm_r();
            //     }
            // }
            node = parse_lval(tokens);
            break;
        }
        case INTCON:{
            node = parse_number(tokens);
            break;
        }
    }
    printff("<PrimaryExp>");
    return NULL;
}
TreeNode *parse_number(Token **tokens){
    //printf("in number\n");
    TreeNode *node = NULL;
    print_token(tokens[top_tokens]);
    top_tokens++;
    judge_fpt=0;
    if(is_return){
        error_f = 0;
    }
    find_e_error();
    //judge_fpt = -1;
    printff("<Number>");
    return NULL;
}
TreeNode *parse_unaryexp(Token **tokens){
    TreeNode *node = NULL;
    int temp_c=-1;
    int temp_de=-1;
    int temp_jf=-1;
    int temp_jpnum=-1;
    int temp_f_or_brack = 0;
    //printf("judge_f:%d\n",judge_f);
    //printf("here!!\n");
    //printf("%s\n",tokens[top_tokens]->value);
    //print_token(tokens[top_tokens]);
    if(tokens[top_tokens]->type == IDENFR){
        temp_c=top_tokens;
        temp_de=top_tokens;
        flag_tokens_1=top_tokens+1;
        int error_j=0;
        if(tokens[flag_tokens_1]->type == LPARENT){
            int f_param_list[100],f_param_list_type[100],f_param_list_num = 0;
            // if(judge_f_type(top_tokens)){
            //     writeTollvmFile("    %s"+std::to_string(llvm_r)+" = call i32 @"+std::string(tokens[top_tokens]->value)+"(");//%4 = call i32 @getint()
            //     new_llvm_r();
            // }else{
            //     writeTollvmFile("    call void @"+std::string(tokens[top_tokens]->value)+"(");//call void @ab()
            // }
            if(f_or_brack != 1){
                temp_f_or_brack = f_or_brack;
                f_or_brack = 1;
            }
            //printf("judge_f:%s,judge_fpnum:%d\n",tokens[judge_f]->value,judge_fpnum);
            if(judge_fpnum!=-1){
                temp_jpnum = judge_fpnum;
                judge_fpnum = -1;
            }
            if(judge_f!=-1 && judge_fpnum==-1){
                std::string front;
                Symbol end;
                for(auto p: st[0].Hashmap){
                    front = p.first;   //key
                    end = p.second;   //value
                    if(strcmp(front.c_str(),tokens[top_tokens]->value) == 0){
                        //printf("paramNum:%d,judge_fpnum:%d\n",p.second.paramNum,judge_fpnum);
                        if(p.second.retype == 0){
                            error_f=0;
                        }
                    }
                }
            }
            print_token(tokens[top_tokens]);
            print_token(tokens[flag_tokens_1]);
            top_tokens=flag_tokens_1+1;
            //printf("%s\n",tokens[top_tokens]->value);
            if(tokens[top_tokens]->type == RPARENT){
                f_or_brack = temp_f_or_brack;
                if(judge_f!=-1){
                    judge_fpnum=temp_jpnum;
                    find_e_f_error(temp_de);
                    judge_fpnum=-1;
                    temp_jf=judge_f;
                }
                judge_f=temp_de;
                judge_fpnum++;
                print_token(tokens[top_tokens]);
                top_tokens++;
            }else if(tokens[top_tokens]->type == SEMICN){
                if(judge_f!=-1){
                    temp_jf = judge_f;
                }
                f_or_brack = temp_f_or_brack;
                error_j=1;
                error('j',top_tokens-1);
            }else{
                //printf("%d\n",judge_fpnum);
                if(judge_f!=-1){
                    judge_fpnum=temp_jpnum;
                    //printf("wuwu\n");
                    find_e_f_error(temp_de);
                    //printf("wuwu\n");
                    judge_fpnum=-1;
                    temp_jf=judge_f;
                }
                judge_fpnum++;
                judge_f=temp_de;
                //printf("%d\n",judge_fpnum);
                //node = parse_funcrparams(tokens);//原本为一个子节点的递归下降，为了实现局部变量的效果直接挪用
                                judge_fpnum++;
                                int local_llvm_r_0 = llvm_r -1;
                                node = parse_exp(tokens);
                                int local_llvm_r_1 = llvm_r - 1;
                                if(local_llvm_r_0 == local_llvm_r_1){
                                    f_param_list_type[f_param_list_num] = 0;
                                    f_param_list[f_param_list_num] = std::stoi(tokens[top_tokens-1]->value);
                                    f_param_list_num++;
                                }else{
                                    f_param_list_type[f_param_list_num] = 1;
                                    f_param_list[f_param_list_num] = llvm_r - 1;
                                    f_param_list_num++;
                                }
                                while(tokens[top_tokens]->type == COMMA){
                                    judge_fpnum++;
                                    print_token(tokens[top_tokens]);
                                    top_tokens++;
                                    local_llvm_r_0 = llvm_r -1;
                                    node = parse_exp(tokens);
                                    local_llvm_r_1 = llvm_r - 1;
                                    if(local_llvm_r_0 == local_llvm_r_1){
                                        f_param_list_type[f_param_list_num] = 0;
                                        f_param_list[f_param_list_num] = std::stoi(tokens[top_tokens-1]->value);
                                        f_param_list_num++;
                                    }else{
                                        f_param_list_type[f_param_list_num] = 1;
                                        f_param_list[f_param_list_num] = llvm_r - 1;
                                        f_param_list_num++;
                                    }
                                }
                                printff("<FuncRParams>");
                //printf("%d\n",judge_fpnum);
                if(tokens[top_tokens]->type == RPARENT){
                    f_or_brack = temp_f_or_brack;
                    print_token(tokens[top_tokens]);
                    top_tokens++;
                }else{
                    f_or_brack = temp_f_or_brack;
                    error('j',top_tokens-1);
                }
            }
            if(judge_f_type(temp_c)){//%7 = call i32 @aaa(i32 %5, i32 %6)
                Symbol temp_symbol = get_f_symbol(judge_f);
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = call i32 @"+std::string(tokens[temp_c]->value)+"(");
                for(int j=0;j<f_param_list_num;j++){
                    if(temp_symbol.paramTypeList[j] == 0){
                        if(j == 0){
                            if(f_param_list_type[j] == 1){
                                writeTollvmFile("i32 %s"+std::to_string(f_param_list[j]));
                            }else{
                                writeTollvmFile("i32 "+std::to_string(f_param_list[j]));
                            }
                        }else{
                            if(f_param_list_type[j] == 1){
                                writeTollvmFile(", i32 %s"+std::to_string(f_param_list[j]));
                            }else{
                                writeTollvmFile(", i32 "+std::to_string(f_param_list[j]));
                            }
                        }
                    }else if(temp_symbol.paramTypeList[j] == 1){
                        if(j == 0){
                            if(f_param_list_type[j] == 1){
                                writeTollvmFile("i32* %s"+std::to_string(f_param_list[j]));
                            }else{
                                writeTollvmFile("i32 "+std::to_string(f_param_list[j]));
                            }
                        }else{
                            if(f_param_list_type[j] == 1){
                                writeTollvmFile(", i32* %s"+std::to_string(f_param_list[j]));
                            }else{
                                writeTollvmFile(", i32 "+std::to_string(f_param_list[j]));
                            }
                        }
                    }else if(temp_symbol.paramTypeList[j] == 2){
                        if(j == 0){
                            if(f_param_list_type[j] == 1){
                                writeTollvmFile("["+std::to_string(temp_symbol.paramTypeList_help[j])+" x i32]* %s"+std::to_string(f_param_list[j]));
                            }else{
                                writeTollvmFile("i32 "+std::to_string(f_param_list[j]));
                            }
                        }else{
                            if(f_param_list_type[j] == 1){
                                writeTollvmFile(", ["+std::to_string(temp_symbol.paramTypeList_help[j])+" x i32]* %s"+std::to_string(f_param_list[j]));
                            }else{
                                writeTollvmFile(", i32 "+std::to_string(f_param_list[j]));
                            }
                        }
                    }
                }
                new_llvm_r();
            }else{//call void @ab()
                Symbol temp_symbol = get_f_symbol(judge_f);
                writeTollvmFile("    call void @"+std::string(tokens[temp_c]->value)+"(");
                for(int j=0;j<f_param_list_num;j++){
                    if(temp_symbol.paramTypeList[j] == 0){
                        if(j == 0){
                            if(f_param_list_type[j] == 1){
                                writeTollvmFile("i32 %s"+std::to_string(f_param_list[j]));
                            }else{
                                writeTollvmFile("i32 "+std::to_string(f_param_list[j]));
                            }
                        }else{
                            if(f_param_list_type[j] == 1){
                                writeTollvmFile(", i32 %s"+std::to_string(f_param_list[j]));
                            }else{
                                writeTollvmFile(", i32 "+std::to_string(f_param_list[j]));
                            }
                        }
                    }else if(temp_symbol.paramTypeList[j] == 1){
                        if(j == 0){
                            if(f_param_list_type[j] == 1){
                                writeTollvmFile("i32* %s"+std::to_string(f_param_list[j]));
                            }else{
                                writeTollvmFile("i32 "+std::to_string(f_param_list[j]));
                            }
                        }else{
                            if(f_param_list_type[j] == 1){
                                writeTollvmFile(", i32* %s"+std::to_string(f_param_list[j]));
                            }else{
                                writeTollvmFile(", i32 "+std::to_string(f_param_list[j]));
                            }
                        }
                    }else if(temp_symbol.paramTypeList[j] == 2){
                        if(j == 0){
                            if(f_param_list_type[j] == 1){
                                writeTollvmFile("["+std::to_string(temp_symbol.paramTypeList_help[j])+" x i32]* %s"+std::to_string(f_param_list[j]));
                            }else{
                                writeTollvmFile("i32 "+std::to_string(f_param_list[j]));
                            }
                        }else{
                            if(f_param_list_type[j] == 1){
                                writeTollvmFile(", ["+std::to_string(temp_symbol.paramTypeList_help[j])+" x i32]* %s"+std::to_string(f_param_list[j]));
                            }else{
                                writeTollvmFile(", i32 "+std::to_string(f_param_list[j]));
                            }
                        }
                    }
                }
            }
            writeTollvmFile(")\n");
            if(temp_jf!=-1){
                judge_f=temp_jf;
            }else{
                judge_f=-1;
            }
            if(temp_c!=-1){
                if(!find_c_error(temp_c) && error_j ==0){
                    find_d_error(temp_de);
                }
                error_j = 0;
            }
            if(temp_jpnum!=-1){
                judge_fpnum=temp_jpnum;
            }else{
                judge_fpnum=-1;
            } 
        }else{
            node = parse_primaryexp(tokens);
        }
    }else if(tokens[top_tokens]->type == LPARENT||tokens[top_tokens]->type == INTCON){
        node = parse_primaryexp(tokens);
    }else if(tokens[top_tokens]->type == PLUS||tokens[top_tokens]->type == MINU||tokens[top_tokens]->type == NOT){
        int local_top_1 = top_tokens;
        node = parse_unaryop(tokens);
        int local_top_2 = top_tokens;
        int ababa = llvm_r - 1;
        node = parse_unaryexp(tokens);
        int local_llvm_r_2 = llvm_r-1;
        if(in_main == 1|| judge_f != -1){
            if(tokens[local_top_1]->type == MINU){
                while(tokens[local_top_2]->type == PLUS){
                    local_top_2++;
                }
                if(tokens[local_top_2]->type == INTCON){
                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = sub i32 0, "+tokens[local_top_2]->value+"\n");
                    new_llvm_r();
                }else{
                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = sub i32 0, "+"%s"+std::to_string(local_llvm_r_2)+"\n");
                    new_llvm_r();
                }
            }
        }
        if(tokens[local_top_1]->type == NOT){
            if(ababa != local_llvm_r_2){
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = icmp eq i32 %s"+std::to_string(llvm_r-1)+", 0\n");//%a3 = icmp eq i32 %a2, 0
                new_llvm_r();
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = zext i1 %s"+std::to_string(llvm_r-1)+" to i32\n");//%x1 = zext i1 %x to i32
                new_llvm_r();
            }else{
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = icmp eq i32 "+tokens[local_top_2]->value+", 0\n");//%a3 = icmp eq i32 %a2, 0
                new_llvm_r();
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = zext i1 %s"+std::to_string(llvm_r-1)+" to i32\n");//%x1 = zext i1 %x to i32
                new_llvm_r();
            }
        }
    }
    //print_token(tokens[top_tokens]);
    printff("<UnaryExp>");
    //printf("end unaryexp\n");
    return NULL;
}
TreeNode *parse_unaryop(Token **tokens){
    TreeNode *node = NULL;
    switch(tokens[top_tokens]->type){
        case PLUS:{
            print_token(tokens[top_tokens]);
            top_tokens++;
            break;
        }
        case MINU:{
            print_token(tokens[top_tokens]);
            top_tokens++;
            break;
        }
        case NOT:{
            print_token(tokens[top_tokens]);
            top_tokens++;
            break;
        }
    }
    printff("<UnaryOp>");
    return NULL;
}
TreeNode *parse_funcrparams(Token **tokens){
    TreeNode *node = NULL;
    judge_fpnum++;
    node = parse_exp(tokens);
    while(tokens[top_tokens]->type == COMMA){
        judge_fpnum++;
        print_token(tokens[top_tokens]);
        top_tokens++;
        node = parse_exp(tokens);
    }
    printff("<FuncRParams>");
    return NULL;
}
TreeNode *parse_mulexp(Token **tokens){
    if(in_main == 1 || judge_f != -1){
        TreeNode *node = NULL;
        int b1 = 1,b2 = 1;
        int local_llvm_r_0 = llvm_r-1;
        int local_token = top_tokens;
        node = parse_unaryexp(tokens);
        //writeTollvmFile(";"+std::to_string(llvm_r-1)+"\n");
        int local_llvm_r_1 = llvm_r-1;
        if(local_llvm_r_0 == local_llvm_r_1){
            //writeTollvmFile(";"+std::to_string(local_llvm_r_0)+";"+std::to_string(local_llvm_r_1)+"\n");
            local_token = skip_plus_lck(local_token);
            if(tokens[local_token]->type == INTCON){
                b1 = 0;
                local_llvm_r_1 = std::stoi(tokens[local_token]->value);
            }
        }
        local_llvm_r_0 = llvm_r - 1;
        while(tokens[top_tokens]->type == MULT||tokens[top_tokens]->type == DIV||tokens[top_tokens]->type == MOD){
            int temp_use = top_tokens;
            printff("<MulExp>");
            print_token(tokens[top_tokens]);
            top_tokens++;
            local_token = top_tokens;
            node = parse_unaryexp(tokens);
            int local_llvm_r_2 = llvm_r - 1;
            //writeTollvmFile(";"+std::to_string(local_llvm_r_0)+";"+std::to_string(local_llvm_r_2)+"\n");
            if(local_llvm_r_2 == local_llvm_r_0){
                local_token = skip_plus_lck(local_token);
                if(tokens[local_token]->type == INTCON){
                    b2 = 0;
                    local_llvm_r_2 = std::stoi(tokens[local_token]->value);
                }
            }
            if(tokens[temp_use]->type == MULT){
                alu_writetollvmfile("mul",local_llvm_r_1,local_llvm_r_2,b1,b2);
            }else if(tokens[temp_use]->type == DIV){
                alu_writetollvmfile("sdiv",local_llvm_r_1,local_llvm_r_2,b1,b2);
            }else{
                alu_writetollvmfile("srem",local_llvm_r_1,local_llvm_r_2,b1,b2);
            }
            if(tokens[top_tokens]->type == MULT||tokens[top_tokens]->type == DIV||tokens[top_tokens]->type == MOD){
                local_llvm_r_1 = llvm_r - 1;
                b1 = 1;
                local_llvm_r_0 = llvm_r -1;
                b2 = 1;
            }
            
        }
    }else{
        TreeNode *node = NULL;
        node = parse_unaryexp(tokens);
        while(tokens[top_tokens]->type == MULT||tokens[top_tokens]->type == DIV||tokens[top_tokens]->type == MOD){
            printff("<MulExp>");
            print_token(tokens[top_tokens]);
            top_tokens++;
            node = parse_unaryexp(tokens);
        }
    }
    printff("<MulExp>");
    return NULL;
}
TreeNode *parse_addexp(Token **tokens){
    //printf("inadd,%s,%s,%s\n",tokens[top_tokens-1]->value,tokens[top_tokens]->value,tokens[top_tokens+1]->value);
    if(in_main == 1 || judge_f != -1){
        TreeNode *node = NULL;
        int b1 = 1,b2 = 1;
        int local_llvm_r_0 = llvm_r-1;
        int local_token = top_tokens;
        node = parse_mulexp(tokens);
        int local_llvm_r_1 = llvm_r-1;
        int judge_local_llvm_r_1 = llvm_r-1;
        if(local_llvm_r_0 == local_llvm_r_1){
            local_token = skip_plus_lck(local_token);
            if(tokens[local_token]->type == INTCON){
                b1 = 0;
                local_llvm_r_1 = std::stoi(tokens[local_token]->value);
            }
        }
        local_llvm_r_0 = llvm_r - 1;
        while(tokens[top_tokens]->type == PLUS||tokens[top_tokens]->type == MINU){
            int temp_use = top_tokens;
            printff("<AddExp>");
            //printf("flag\n");
            print_token(tokens[top_tokens]);
            top_tokens++;
            local_token = top_tokens;
            node = parse_mulexp(tokens);
            int local_llvm_r_2 = llvm_r - 1;
            //printf("%d::%d\n",local_llvm_r_1,local_llvm_r_2);
            if(local_llvm_r_2 == local_llvm_r_0){
                local_token = skip_plus_lck(local_token);
                if(tokens[local_token]->type == INTCON){
                    b2 = 0;
                    local_llvm_r_2 = std::stoi(tokens[local_token]->value);
                }
            }
            if(tokens[temp_use]->type == PLUS){
                //printf("%d\n",b2);
                alu_writetollvmfile("add",local_llvm_r_1,local_llvm_r_2,b1,b2);
            }else if(tokens[temp_use]->type == MINU){
                alu_writetollvmfile("sub",local_llvm_r_1,local_llvm_r_2,b1,b2);
            }
            if(tokens[top_tokens]->type == PLUS||tokens[top_tokens]->type == MINU){
                local_llvm_r_1 = llvm_r - 1;
                local_llvm_r_0 = llvm_r - 1;
                b1 = 1;
                b2 = 1;
            }
        }
    }else{
        TreeNode *node = NULL;
        node = parse_mulexp(tokens);
        while(tokens[top_tokens]->type == PLUS||tokens[top_tokens]->type == MINU){
            printff("<AddExp>");
            print_token(tokens[top_tokens]);
            top_tokens++;
            node = parse_mulexp(tokens);
        }
    }
    printff("<AddExp>");
    return NULL;
}
TreeNode *parse_relexp(Token **tokens){
    if(in_main == 1 || judge_f != -1){
        int tool = 0;
        TreeNode *node = NULL;
        int b1 = 1,b2 = 1;
        int local_llvm_r_0 = llvm_r-1;
        int local_token = top_tokens;
        node = parse_addexp(tokens);
        int local_llvm_r_1 = llvm_r-1;
        int judge_local_llvm_r_1 = llvm_r-1;
        if(local_llvm_r_0 == local_llvm_r_1){
            while(tokens[local_token]->type == PLUS){
                local_token++;
            }
            if(tokens[local_token]->type == INTCON){
                b1 = 0;
                local_llvm_r_1 = std::stoi(tokens[local_token]->value);
            }
        }
        local_llvm_r_0 = llvm_r -1;
        while(tokens[top_tokens]->type == LSS||tokens[top_tokens]->type == LEQ||tokens[top_tokens]->type == GRE||tokens[top_tokens]->type == GEQ){
            int temp_use = top_tokens;
            printff("<RelExp>");
            print_token(tokens[top_tokens]);
            top_tokens++;
            local_token = top_tokens;
            node = parse_addexp(tokens);
            int local_llvm_r_2 = llvm_r - 1;
            //printf("%d::%d\n",local_llvm_r_1,local_llvm_r_2);
            if(local_llvm_r_2 == local_llvm_r_0){
                local_token = skip_plus_lck(local_token);
                if(tokens[local_token]->type == INTCON){
                    b2 = 0;
                    local_llvm_r_2 = std::stoi(tokens[local_token]->value);
                }
            }
            if(tool == 1){
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = zext i1 %s"+std::to_string(local_llvm_r_0)+" to i32\n");//%s305 = zext i1 %s304 to i32
                new_llvm_r();
                local_llvm_r_1 = llvm_r - 1;
            }
            if(tokens[temp_use]->type == GRE){
                //printf("%d\n",b2);
                cond_writetollvmfile("sgt",local_llvm_r_1,local_llvm_r_2,b1,b2);
            }else if(tokens[temp_use]->type == GEQ){
                cond_writetollvmfile("sge",local_llvm_r_1,local_llvm_r_2,b1,b2);
            }else if(tokens[temp_use]->type == LSS){
                cond_writetollvmfile("slt",local_llvm_r_1,local_llvm_r_2,b1,b2);
            }else if(tokens[temp_use]->type == LEQ){
                cond_writetollvmfile("sle",local_llvm_r_1,local_llvm_r_2,b1,b2);
            }
            if(tokens[top_tokens]->type == LSS||tokens[top_tokens]->type == LEQ||tokens[top_tokens]->type == GRE||tokens[top_tokens]->type == GEQ){
                local_llvm_r_1 = llvm_r - 1;
                local_llvm_r_0 = llvm_r - 1;
                tool = 1;
                b1 = 1;
                b2 = 1;
            }
        }
    }else{
        TreeNode *node = NULL;
        node = parse_addexp(tokens);
        while(tokens[top_tokens]->type == LSS||tokens[top_tokens]->type == LEQ||tokens[top_tokens]->type == GRE||tokens[top_tokens]->type == GEQ){
            printff("<RelExp>");
            print_token(tokens[top_tokens]);
            top_tokens++;
            node = parse_addexp(tokens);
        }
    }
    printff("<RelExp>");
    return NULL;
}
TreeNode *parse_eqexp(Token **tokens){
                int local_llvm = llvm_r;
                int flag_to_zext = 0;
                if(!judge_A_if(top_tokens)){
                    flag_to_zext = 1;
                }    
    if(in_main == 1 || judge_f != -1){
        TreeNode *node = NULL;
        int b1 = 1,b2 = 1;
        int local_llvm_r_0 = llvm_r-1;
        int local_token = top_tokens;
        int help_1 =for_solve_eq_bug(top_tokens),help_2; 
        node = parse_relexp(tokens);
        int local_llvm_r_1 = llvm_r-1;
        int judge_local_llvm_r_1 = llvm_r-1;
        if(local_llvm_r_0 == local_llvm_r_1){
            while(tokens[local_token]->type == PLUS){
                local_token++;
            }
            if(tokens[local_token]->type == INTCON){
                b1 = 0;
                local_llvm_r_1 = std::stoi(tokens[local_token]->value);
            }
        }
        local_llvm_r_0 = llvm_r - 1;
        while(tokens[top_tokens]->type == NEQ||tokens[top_tokens]->type == EQL){
            int temp_use = top_tokens;
            printff("<EqExp>"); 
            print_token(tokens[top_tokens]);
            top_tokens++;
            local_token = top_tokens;
            help_2 = for_solve_eq_bug(top_tokens);
            node = parse_relexp(tokens);
            int local_llvm_r_2 = llvm_r - 1;
            //printf("%d::%d\n",local_llvm_r_1,local_llvm_r_2);
            if(local_llvm_r_2 == local_llvm_r_0){
                local_token = skip_plus_lck(local_token);
                if(tokens[local_token]->type == INTCON){
                    b2 = 0;
                    local_llvm_r_2 = std::stoi(tokens[local_token]->value);
                }
            }
            if(help_1 == 1){
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = zext i1 %s"+std::to_string(local_llvm_r_1)+" to i32\n");//%t2 = zext i1 %s51 to i32
                new_llvm_r();
                local_llvm_r_1 = llvm_r - 1;
            }
            if(help_2 == 1){
                writeTollvmFile("    %s"+std::to_string(llvm_r)+" = zext i1 %s"+std::to_string(local_llvm_r_2)+" to i32\n");//%t2 = zext i1 %s51 to i32
                new_llvm_r();
                local_llvm_r_2 = llvm_r - 1;
            }
            if(tokens[temp_use]->type == EQL){
                //printf("%d\n",b2);
                cond_writetollvmfile("eq",local_llvm_r_1,local_llvm_r_2,b1,b2);
            }else if(tokens[temp_use]->type == NEQ){
                cond_writetollvmfile("ne",local_llvm_r_1,local_llvm_r_2,b1,b2);
            }
            if(tokens[top_tokens]->type == EQL||tokens[top_tokens]->type == NEQ){
                local_llvm_r_1 = llvm_r - 1;
                local_llvm_r_0 = llvm_r - 1;
                help_1 = 1;
                b1 = 1;
                b2 = 1;
            }
        }
    }else{
        TreeNode *node = NULL;
        node = parse_relexp(tokens);
        while(tokens[top_tokens]->type == EQL||tokens[top_tokens]->type == NEQ){
            // printf("EQL\n");
            printff("<EqExp>");
            print_token(tokens[top_tokens]);
            top_tokens++;
            node = parse_relexp(tokens);
        }
    }
    printff("<EqExp>");
                if(local_llvm == llvm_r){
                    if(tokens[top_tokens-1]->type != SEMICN){
                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = icmp ne i32 "+std::string(tokens[top_tokens-1]->value)+", 0\n");
                    }else{
                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = icmp sge i32 1, 1\n");
                    }
                    new_llvm_r();
                }else{
                    if(flag_to_zext == 1){
                        writeTollvmFile("    %s"+std::to_string(llvm_r)+" = zext i1 %s"+std::to_string(llvm_r - 1)+" to i32\n");//%x1 = zext i1 %x to i32
                        new_llvm_r();
                    }
                    writeTollvmFile("    %s"+std::to_string(llvm_r)+" = icmp ne i32 %s"+std::to_string(llvm_r - 1)+", 0\n");
                    new_llvm_r(); 
                }

    return NULL;
}
TreeNode *parse_landexp(Token **tokens){
    TreeNode *node = NULL;    
    int temp = top_tokens;
    node = parse_eqexp(tokens);
    int in_and = 0;
    while(tokens[top_tokens]->type == AND){
        in_and = 1;
        andstack.push(llvm_and_r);
        llvm_and_r++;
        switch(get_eq_type(top_tokens)){
            case 1:
                if(in_if_or_for == 1){
                    writeTollvmFile("    br i1 %s"+std::to_string(llvm_r-1)+", label %A"+std::to_string(andstack.top())+", label %O"+std::to_string(llvm_or_r)+"\n");
                }else if(in_if_or_for == 2){
                    writeTollvmFile("    br i1 %s"+std::to_string(llvm_r-1)+", label %A"+std::to_string(andstack.top())+", label %O"+std::to_string(llvm_or_r)+"\n");
                }
                break;
            case 2:
                if(in_if_or_for == 1){
                    writeTollvmFile("    br i1 %s"+std::to_string(llvm_r-1)+", label %if"+std::to_string(ifstack.top())+", label %O"+std::to_string(llvm_or_r)+"\n");
                }else if(in_if_or_for == 2){
                    writeTollvmFile("    br i1 %s"+std::to_string(llvm_r-1)+", label %S"+std::to_string(sstack.top())+", label %O"+std::to_string(llvm_or_r)+"\n");
                }
                break;
            case 3:
                if(in_if_or_for == 1){
                    writeTollvmFile("    br i1 %s"+std::to_string(llvm_r-1)+", label %A"+std::to_string(andstack.top())+", label %if"+std::to_string(get_stack_if_2_top())+"\n");
                }else if(in_if_or_for == 2){
                    writeTollvmFile("    br i1 %s"+std::to_string(llvm_r-1)+", label %A"+std::to_string(andstack.top())+", label %B"+std::to_string(bstack.top())+"\n");
                }
                break;
            case 4:
                if(in_if_or_for == 1){
                    writeTollvmFile("    br i1 %s"+std::to_string(llvm_r-1)+", label %if"+std::to_string(ifstack.top())+", label %if"+std::to_string(get_stack_if_2_top())+"\n");
                }else if(in_if_or_for == 2){
                    writeTollvmFile("    br i1 %s"+std::to_string(llvm_r-1)+", label %S"+std::to_string(sstack.top())+", label %B"+std::to_string(bstack.top())+"\n");
                }
                break;
        }
        // if(go_o(top_tokens)){
        //     writeTollvmFile("    br i1 %s"+std::to_string(llvm_r-1)+", label %A"+std::to_string(andstack.top())+", label %O"+std::to_string(llvm_or_r)+"\n");
        // }else{
        //     writeTollvmFile("    br i1 %s"+std::to_string(llvm_r-1)+", label %A"+std::to_string(andstack.top())+", label %if"+std::to_string(ifstack.top())+"\n");
        // }
        printff("<LAndExp>");
        print_token(tokens[top_tokens]);
        writeTollvmFile("A"+std::to_string(andstack.top())+":\n");
        andstack.pop();
        top_tokens++;
        node = parse_eqexp(tokens);
    }
    printff("<LAndExp>");
    return NULL;
}
TreeNode *parse_lorexp(Token **tokens){
    //printf("%d\n",ifstack.top());
    TreeNode *node = NULL;
    node = parse_landexp(tokens);
    while(tokens[top_tokens]->type == OR){
        orstack.push(llvm_or_r);
        llvm_or_r++;
        if(in_if_or_for == 1){
            writeTollvmFile("    br i1 %s"+std::to_string(llvm_r-1)+", label %if"+std::to_string(ifstack.top())+", label %O"+std::to_string(orstack.top())+"\n");
        }else{
            writeTollvmFile("    br i1 %s"+std::to_string(llvm_r-1)+", label %S"+std::to_string(sstack.top())+", label %O"+std::to_string(orstack.top())+"\n");
        }
        printff("<LOrExp>");
        print_token(tokens[top_tokens]);
        writeTollvmFile("O"+std::to_string(orstack.top())+":\n");
        orstack.pop();
        top_tokens++;
        node = parse_landexp(tokens);
    }
    printff("<LOrExp>");
    return NULL;
}
TreeNode *parse_constexp(Token **tokens){
    TreeNode *node = NULL;
    node = parse_addexp(tokens);
    printff("<ConstExp>");
    return NULL;
}


int main() {
    const char *filename = "testfile.txt";
    const char *input = readTextFile(filename);
    Token *token;
    int token_count = 0;
    while ((token = get_next_token(&input)) != NULL) {
        if(token->value!=NULL){tokens[token_count] = token;token_count++;}
    }
    num_print=1;
    tokens[token_count] = NULL; 
    TreeNode *AST = parse_compunit(tokens);
    return 0;
}
