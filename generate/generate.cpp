#include "generate.h"


enum class token_types{VAR, VAR_CREATE, FUNCTION_START, FUNCTION, FUNCTION_CALL, MATH, LOGIC, NUMBER, END_SPECIAL_SCOPE, IF, WHILE, RETURN, POP};
enum class math_tokens{EMPTY, PLUS, MINUS, BSL, BSR, OR, XOR, AND, NOT};
enum class logic_tokens{ASSIGN, NOT_EQ, LESS_EQ, LESS, MORE_EQ, MORE, EQUAL};

int scope_num = 0;

void load_math(Node& base, std::string& asm_str);
void gen_function(Node& base, std::string& asm_str);
void gen_base(Node& base, std::string& asm_str);

void load_val(Node& base, std::string& asm_str, int addr){
    if(base.lower_nodes[addr - 1]->token[0] == (int)token_types::NUMBER)
        asm_str += "IMM " + std::to_string(addr) + " " + std::to_string(base.lower_nodes[addr - 1]->token[1]) + " //imm num to param " + std::to_string(addr) + "\n";
    else if(base.lower_nodes[addr - 1]->token[0] == (int)token_types::VAR) {
        asm_str += "IMM 4 " + std::to_string(base.lower_nodes[addr - 1]->token[1]) + " //imm var addr\n";
        asm_str += "MEMR 4 1 " + std::to_string(addr) + " //load var to param " + std::to_string(addr) + "\n";
    }else if(base.lower_nodes[addr - 1]->token[0] == (int)token_types::MATH){
        load_math(*base.lower_nodes[addr - 1], asm_str);
        asm_str += "MOV 3 NOP " + std::to_string(addr) + " //mov res to param " + std::to_string(addr) + "\n";
    }else if(base.lower_nodes[addr - 1]->token[0] == (int)token_types::FUNCTION){
        gen_function(*base.lower_nodes[addr - 1], asm_str);
        asm_str += "GETSTACKPTR NOP NOP 4 //get stack_top\n";
        asm_str += "IMM 5 num //returned val addr offset\n";
        asm_str += "ADD 4 5 6 //calc ret val addr\n";
        asm_str += "MEMR 6 0 " + std::to_string(addr) + " //load ret val to param " + std::to_string(addr) + "\n";
    }
}

void load_vals(Node& base, std::string& asm_str){
    load_val(base, asm_str, 1);
    load_val(base, asm_str, 2);
}

void load_math(Node& base, std::string& asm_str){
    load_vals(base, asm_str);
    asm_str += std::to_string(base.token[1]) + " 1 2 3 //do math based on token\n";
}

void gen_function(Node& base, std::string& asm_str){
    asm_str += "PUSH NOP 0 NOP //push reg_0\n";//push all
    asm_str += "PUSH NOP 1 NOP //push reg_1\n";
    asm_str += "PUSH NOP 2 NOP //push reg_2\n";
    asm_str += "PUSH NOP 3 NOP //push reg_3\n";
    asm_str += "PUSH NOP 4 NOP //push reg_4\n";
    asm_str += "PUSH NOP 5 NOP //push reg_5\n";
    asm_str += "PUSH NOP 6 NOP //push reg_6\n";
    asm_str += "PUSH NOP 7 NOP //push reg_7\n";
    asm_str += "GETSTACKPTR NOP NOP 7 //save ret_addr addr\n";
    asm_str += "PUSH NOP 2 NOP //push rand ret_addr\n";
    asm_str += "GETSTACKPTR NOP NOP 6 //save new stack_base\n";


    for(auto param : base.lower_nodes){
        if(param->token[0] == (int)token_types::VAR){
            asm_str += "IMM 4 " + std::to_string(param->token[1]) + " //imm var addr\n";
            asm_str += "MEMR 4 1 1 //load var to param 1\n";
            asm_str += "PUSH NOP 1 NOP //push reg_1\n";
        }else if(param->token[0] == (int)token_types::NUMBER){
            asm_str += "IMM 1 " + std::to_string(param->token[1]) + " //imm num to param 1\n";
            asm_str += "PUSH NOP 1 NOP //push reg_1\n";
        }
    }

    asm_str += "GETPC NOP NOP 0 //get curr_addr\n";
    asm_str += "IMM 1 14 //get curr_addr\n";
    asm_str += "ADD 0 1 2 //calculate ret_addr\n";
    asm_str += "MEMW 7 2 1 //write real ret_addr\n";
    asm_str += "MOV 6 NOP 0 //set new stack_base\n";
    asm_str += "IMM 1 REF_FN_" + std::to_string(base.token[1]) + " //imm fn addr\n";
    asm_str += "JMP 1 1 NOP //call fn\n";


    asm_str += "Pop 1 NOP 0 //push reg_0\n";//pop all
    asm_str += "Pop 1 NOP 1 //push reg_1\n";
    asm_str += "Pop 1 NOP 2 //push reg_2\n";
    asm_str += "Pop 1 NOP 3 //push reg_3\n";
    asm_str += "Pop 1 NOP 4 //push reg_4\n";
    asm_str += "Pop 1 NOP 5 //push reg_5\n";
    asm_str += "Pop 1 NOP 6 //push reg_6\n";
    asm_str += "Pop 1 NOP 7 //push reg_7\n";
}

void gen_if(Node& token, std::string& asm_str){
    scope_num++;
    int local_scope_num = scope_num;
    load_vals(*token.lower_nodes[0], asm_str);
    asm_str += "CMP 1 2 " + std::to_string(token.lower_nodes[0]->token[1]) + " //compare nums\n";
    asm_str += "IMM 1 REF_SCOPE_" + std::to_string(local_scope_num) + " //imm end_if addr\n";
    asm_str += "JMP 1 NOP NOP //jump if not satisfied\n";


    gen_base(token, asm_str);

    asm_str += "LABEL_SCOPE_" + std::to_string(local_scope_num) + " //end if\n";
}

void gen_while(Node& token, std::string& asm_str){
    scope_num++;
    int start_while = scope_num;
    scope_num++;
    int exit_while = scope_num;
    asm_str += "LABEL_SCOPE_" + std::to_string(start_while) + " //loop starts here\n";
    load_vals(*token.lower_nodes[0], asm_str);
    asm_str += "CMP 1 2 " + std::to_string(token.lower_nodes[0]->token[1]) + " //compare nums\n";
    asm_str += "IMM 1 REF_SCOPE_" + std::to_string(exit_while) + " //imm end_while addr\n";
    asm_str += "JMP 1 NOP NOP //jump if not satisfied\n";


    gen_base(token, asm_str);

    asm_str += "IMM 1 REF_SCOPE_" + std::to_string(exit_while) + " //imm start_loop addr\n";
    asm_str += "JMP 1 1 NOP //jump\n";

    asm_str += "LABEL_SCOPE_" + std::to_string(exit_while) + " //end while\n";
}

void gen_base(Node& base, std::string& asm_str){
    for(auto token : base.lower_nodes){

        if(token->token[0] == (int)token_types::FUNCTION){
            gen_function(*token, asm_str);
        }

        if(token->token[0] == (int)token_types::LOGIC && token->token[1] == (int)logic_tokens::ASSIGN){
            if(token->lower_nodes[1]->token[0] == (int)token_types::VAR){
                asm_str += "IMM 4 " + std::to_string(token->lower_nodes[1]->token[1]) + " //imm var addr\n";
                asm_str += "MEMR 4 1 2 //load var to param 2\n";
            }else if(token->lower_nodes[1]->token[0] == (int)token_types::NUMBER){
                asm_str += "IMM 2 " + std::to_string(token->lower_nodes[1]->token[1]) + " //imm num to param 2\n";
            }else if(token->lower_nodes[1]->token[0] == (int)token_types::MATH)
                load_val(*token, asm_str, 2);
            else if(token->lower_nodes[1]->token[0] == (int)token_types::FUNCTION){
                gen_function(*token->lower_nodes[1], asm_str);
                asm_str += "GETSTACKPTR NOP NOP 4 //get stack_top\n";
                asm_str += "IMM 5 num //returned val addr offset\n";
                asm_str += "ADD 4 5 6 //calc ret val addr\n";
                asm_str += "MEMR 6 0 2 //load ret val to param 2\n";
            }

            if(token->lower_nodes[0]->token[0] == (int)token_types::VAR) {
                asm_str += "IMM 1 " + std::to_string(token->lower_nodes[0]->token[1]) + " //imm var addr\n";
                asm_str += "MEMW 1 2 1";
            }
            else
                asm_str += "PUSH NOP 2 NOP //push val to new var\n";
        }

        if(token->token[0] == (int)token_types::IF){
            gen_if(*token, asm_str);
        }

        if(token->token[0] == (int)token_types::WHILE){
            gen_while(*token, asm_str);
        }

        if(token->token[0] == (int)token_types::RETURN){//add return number functionality
            if(token->lower_nodes[0]->token[0] == (int)token_types::VAR){
                asm_str += "IMM 4 " + std::to_string(token->lower_nodes[0]->token[1]) + " //imm var addr\n";
                asm_str += "MEMR 4 1 2 //load var to param 2\n";
            }else if(token->lower_nodes[0]->token[0] == (int)token_types::NUMBER){
                asm_str += "IMM 2 " + std::to_string(token->lower_nodes[0]->token[1]) + " //imm num to param 2\n";
            }else if(token->lower_nodes[0]->token[0] == (int)token_types::MATH)
                load_val(*token, asm_str, 2);
            else if(token->lower_nodes[0]->token[0] == (int)token_types::FUNCTION){
                gen_function(*token->lower_nodes[0], asm_str);
                asm_str += "GETSTACKPTR NOP NOP 4 //get stack_top\n";
                asm_str += "IMM 5 num //returned val addr offset\n";
                asm_str += "ADD 4 5 6 //calc ret val addr\n";
                asm_str += "MEMR 6 0 2 //load ret val to param 2\n";
            }
            asm_str += "IMM 1 0 //set_ptr to var 0\n";
            asm_str += "MEMW 1 2 1 //save ret_val to 0\n";

            asm_str += "SETSTACKPTR 0 NOP NOP //set stack_ptr to stack_base\n";
            return;
        }
        if(token->token[0] == (int)token_types::VAR_CREATE)
            asm_str += "PUSH NOP 0 NOP //increase stack_ptr\n";
        if(token->token[0] == (int)token_types::POP)
            asm_str += "POP NOP NOP NOP //decrease stack_ptr\n";
    }
}


std::string gen_fn(Node& base){//already has all vars pushed
    std::string asm_str = "LABEL_FN_" + std::to_string(base.token[1]) + "\n";

    gen_base(base, asm_str);

    asm_str += "POP NOP NOP 0 //get ret addr\n";
    asm_str += "JMP 0 NOP NOP //jump to ret addr\n";
    return asm_str;
}


std::string generate(Node& root){
    std::string asm_str;
    for(auto fn : root.lower_nodes){
        asm_str += gen_fn(*fn);
    }
    return asm_str;
}