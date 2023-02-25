#include "generate.h"
#define REGS_SIZE 7

int scope_num = 0, last_var = -1;

int registers[REGS_SIZE][2];//0 for num, 1 for var, 2 for temp_result, -1 for empty

void genScope(Node& base, std::string& asm_str, int num_of_vars);

int writeRealReg(int reg){
  return reg == 0 ? reg : reg + 1;
}

void saveVars(std::string& asm_str, unsigned int max_addr = -1){
  for(int i = 0; i < REGS_SIZE; i++){
    if(registers[i][0] == 1 && registers[i][1] < max_addr)
      asm_str += "MEMW " + std::to_string(registers[i][1]) + "    " + std::to_string(writeRealReg(i)) + "    1    //save var before if/while\n";
  }
}

void cleanRegs(std::string& asm_str, unsigned int max_addr = -1){
  saveVars(asm_str, max_addr);
  for(auto e : registers)
    e[0] = -1;
}

void writeVarForOverwrite(int reg, std::string& asm_str){
  asm_str += "MEMW " + std::to_string(registers[reg][1]) + "    " + std::to_string(writeRealReg(reg)) + "    1    //save var before overwriting\n";
}

int findEmptyReg(std::string& asm_str){
  for(int i = REGS_SIZE - 1; i >= 0; i--){
    if(registers[i][0] == -1)
      return i;
  }
  for(int i = REGS_SIZE - 1; i >= 0; i--){
    if(registers[i][0] == 0)
      return i;
  }
  for(int i = REGS_SIZE - 1; i >= 0; i--){
    if(registers[i][0] == 1) {//here also write the var to mem
      writeVarForOverwrite(i, asm_str);
      return i;
    }
  }
  return -1;
}

int findReg(int type, int num){
  for(int i = 0; i < REGS_SIZE; i++){
    if(type == registers[i][0] && num == registers[i][1])
      return i;
  }
  return -1;
}

void resetRegs(){
  for(auto& reg : registers)
    reg[0] = -1;
}

int genExpression(Node& base, std::string& asm_str, bool overwrite = false);

int genNot(Node& base, std::string& asm_str){
  int n = genExpression(*base.lower_nodes[0], asm_str);
  int type = registers[n][0];
  registers[n][0] = 2;
  int free_reg = findEmptyReg(asm_str);
  registers[n][0] = type;
  registers[free_reg][0] = -1;
  asm_str += "NOT  " + std::to_string(writeRealReg(n)) + "         " + std::to_string(writeRealReg(free_reg)) + "    //binary negate\n";//add result
  return free_reg;
}

void genFnCall(Node& base, std::string& asm_str){
  int num = scope_num;
  scope_num++;
  int reg_saves[REGS_SIZE][2];

  saveVars(asm_str);

  for(int i = 0; i < REGS_SIZE; i++){
    if(registers[i][0] == 2){
      reg_saves[i][0] = registers[i][0];
      reg_saves[i][1] = registers[i][1];
    }
    else
      reg_saves[i][0] = -1;
  }

  for(int i = 0; i < REGS_SIZE + 1; i++){
    if(registers[i][0] == 2)
      asm_str += "PUSH NULL " + std::to_string(i) + "    NULL //save the register\n";
  }

  registers[1][0] = 2;
  asm_str += "PUSH NULL REF_SCOPE_" + std::to_string(num) + " NULL //save return addr\n";
  asm_str += "GETSTACKPTR NULL NULL 2 //get new stack base\n";
  for(auto e : base.lower_nodes){
    int reg = genExpression(*e, asm_str);
    asm_str += "PUSH NULL " + std::to_string(writeRealReg(reg)) + "NULL //write parameter\n";
  }
  asm_str += "MOV  2    NULL 1    //move stack base to reg 1\n";
  asm_str += "JMP REF_FN_" + std::to_string(base.token[1]) + " 0 NULL // jump to function\n";
  asm_str += "LABEL_SCOPE_" + std::to_string(num) + "\n";

  for(int i = REGS_SIZE - 1; i >= 0; i--){
    registers[i][0] = reg_saves[i][0];
    registers[i][1] = reg_saves[i][1];
    if(registers[i][0] == 2)
      asm_str += "POP  NULL NULL " + std::to_string(i) + "    //get back the registers\n";
  }
}

int genMath(Node& base, int reg_1, int reg_2, std::string& asm_str){
  int res;
  if(isToken(base, token_types::MATH))
    res = findEmptyReg(asm_str);
  else
    res = 0;
  if(isToken(base, math_tokens::PLUS))
    asm_str += "ADD  " + std::to_string(writeRealReg(reg_1)) + "    " + std::to_string(writeRealReg(reg_2)) + "    " + std::to_string(writeRealReg(res)) + "    //add 2 arguments\n";
  else if(isToken(base, math_tokens::MINUS))
    asm_str += "SUB  " + std::to_string(writeRealReg(reg_1)) + "    " + std::to_string(writeRealReg(reg_2)) + "    " + std::to_string(writeRealReg(res)) + "    //sub 2 arguments\n";
  else if(isToken(base, math_tokens::BSL))
    asm_str += "BSL  " + std::to_string(writeRealReg(reg_1)) + "    " + std::to_string(writeRealReg(reg_2)) + "    " + std::to_string(writeRealReg(res)) + "    //bsl 2 arguments\n";
  else if(isToken(base, math_tokens::BSR))
    asm_str += "BSR  " + std::to_string(writeRealReg(reg_1)) + "    " + std::to_string(writeRealReg(reg_2)) + "    " + std::to_string(writeRealReg(res)) + "    //bsr 2 arguments\n";
  else if(isToken(base, math_tokens::XOR))
    asm_str += "XOR  " + std::to_string(writeRealReg(reg_1)) + "    " + std::to_string(writeRealReg(reg_2)) + "    " + std::to_string(writeRealReg(res)) + "    //xor 2 arguments\n";
  else if(isToken(base, math_tokens::AND))
    asm_str += "AND  " + std::to_string(writeRealReg(reg_1)) + "    " + std::to_string(writeRealReg(reg_2)) + "    " + std::to_string(writeRealReg(res)) + "    //and 2 arguments\n";
  else if(isToken(base, math_tokens::OR))
    asm_str += "OR   " + std::to_string(writeRealReg(reg_1)) + "    " + std::to_string(writeRealReg(reg_2)) + "    " + std::to_string(writeRealReg(res)) + "    //or 2 arguments\n";
  else if(isToken(base, logic_tokens::NOT_EQ))
    asm_str += "CMP  " + std::to_string(writeRealReg(reg_1)) + "    " + std::to_string(writeRealReg(reg_2)) + "    3    //!= 2 arguments\n";
  else if(isToken(base, logic_tokens::EQUAL))
    asm_str += "CMP  " + std::to_string(writeRealReg(reg_1)) + "    " + std::to_string(writeRealReg(reg_2)) + "    4    //== 2 arguments\n";
  else if(isToken(base, logic_tokens::MORE))
    asm_str += "CMP  " + std::to_string(writeRealReg(reg_1)) + "    " + std::to_string(writeRealReg(reg_2)) + "    1    //> 2 arguments\n";
  else if(isToken(base, logic_tokens::LESS))
    asm_str += "CMP  " + std::to_string(writeRealReg(reg_1)) + "    " + std::to_string(writeRealReg(reg_2)) + "    2    //< 2 arguments\n";
  else if(isToken(base, logic_tokens::MORE_EQ))
    asm_str += "CMP  " + std::to_string(writeRealReg(reg_1)) + "    " + std::to_string(writeRealReg(reg_2)) + "    5    //>= 2 arguments\n";
  else if(isToken(base, logic_tokens::LESS_EQ))
    asm_str += "CMP  " + std::to_string(writeRealReg(reg_1)) + "    " + std::to_string(writeRealReg(reg_2)) + "    6    //sub 2 arguments\n";
  else
    throw "incorrect expression format";
  return res;
}

int genExpression(Node& base, std::string& asm_str, bool overwrite){//when using a var or num, temporarily assign it 2 (temp res) so it doesn't get overwritten, don't assign it to the result
  if(isToken(base, symbol_tokens::BRACKET_L))
    return genExpression(*base.lower_nodes[0], asm_str, overwrite);
  if(isToken(base, token_types::NUMBER)){
    int free_reg = findReg(0, base.token[1]);
    if(free_reg == -1) {
      free_reg = findEmptyReg(asm_str);
      asm_str += "IMM  " + std::to_string(writeRealReg(free_reg)) + "    " + std::to_string(base.token[1]) + "         //imm num from expression\n";
    }
    registers[free_reg][0] = 0;
    registers[free_reg][1] = base.token[1];
    return free_reg;
  }else if(isToken(base, token_types::VAR)){
    if(base.token[1] > last_var)
      throw "cannot use a variable in the same expression in which it is created";
    int var_reg = findReg(1, base.token[1]);
    if(var_reg == -1) {
      var_reg = findEmptyReg(asm_str);
      asm_str += "MEMR " + std::to_string(base.token[1]) + "    1    " + std::to_string(writeRealReg(var_reg)) + "    //imm num from expression\n";
      registers[var_reg][0] = 1;
      registers[var_reg][1] = base.token[1];
      return var_reg;
    }else{
      if(overwrite)
        asm_str += "MEMW " + std::to_string(base.token[0]) + "    "+ std::to_string(writeRealReg(var_reg)) + "    1    //save var that will be overwritten\n";
      return var_reg;
    }
  }else if(isToken(base, token_types::FUNCTION_CALL)){
    genFnCall(base, asm_str);
    int arg1 = findEmptyReg(asm_str);
    registers[arg1][0] = 2;
    int arg2 = findEmptyReg(asm_str);
    registers[arg2][0] = 2;
    int res = findEmptyReg(asm_str);
    asm_str += "GETSTACKPTR NULL NULL " + std::to_string(writeRealReg(arg1)) + " //get curr stack val\n";
    asm_str += "IMM  " + std::to_string(writeRealReg(arg2)) + "    9         //get returned val offset\n";
    asm_str += "ADD  " + std::to_string(writeRealReg(arg1)) + "    " + std::to_string(writeRealReg(arg2)) + "    " +
               std::to_string(writeRealReg(res)) + "    //calculate returned val addr\n";
    asm_str += "MEMR " + std::to_string(writeRealReg(res)) + "    0    " + std::to_string(writeRealReg(arg1)) + "    //get returned value\n";
    registers[res][0] = -1;
    registers[arg2][0] = -1;
    registers[arg1][0] = -1;
    return arg1;
  }else if(isToken(base, math_tokens::NOT)){
    return genNot(base, asm_str);
  }else{
    bool flipped = false;
    if(base.lower_nodes[1]->lower_nodes.size() > base.lower_nodes[0]->lower_nodes.size()) {
      flipped = true;
      base.lower_nodes.emplace_back(base.lower_nodes[0]);
      base.lower_nodes.erase(base.lower_nodes.begin(), base.lower_nodes.begin() + 1);
    }
    int arg_1 = genExpression(*base.lower_nodes[0], asm_str);
    int arg_2;
    int type_1 = registers[arg_1][0], type_2;
    if(!base.lower_nodes[1]->lower_nodes.empty()){
      asm_str += "PUSH NULL " + std::to_string(writeRealReg(arg_1)) + "    NULL //temporarily push 1 argument for operation";
      registers[arg_1][0] = -1;
      arg_2 = genExpression(*base.lower_nodes[1], asm_str);
      type_2 = registers[arg_2][0];
      registers[arg_2][0] = 2;
      arg_1 = findEmptyReg(asm_str);
      registers[arg_1][0] = 2;
      asm_str += "POP  NULL NULL " + std::to_string(writeRealReg(arg_1)) + "    //get back temporarily pushed arg";
    }else{
      registers[arg_1][0] = 2;
      arg_2 = genExpression(*base.lower_nodes[1], asm_str);
      type_2 = registers[arg_2][0];
      registers[arg_2][0] = 2;
    }
    if(flipped){
      int temp = arg_1;
      arg_1 = arg_2;
      arg_2 = temp;
      temp = type_1;
      type_1 = type_2;
      type_2 = temp;
    }
    int result = genMath(base, arg_1, arg_2, asm_str);
    registers[result][0] = -1;
    registers[arg_2][0] = type_2;
    registers[arg_1][0] = type_1;
    return result;
  }
}

bool genAssign(Node& base, std::string& asm_str){
  int reg = genExpression(*base.lower_nodes[1], asm_str, true);
  if(isToken(*base.lower_nodes[0], token_types::VAR_CREATE)){
    registers[reg][0] = 1;
    registers[reg][1] = base.lower_nodes[0]->token[1];
    asm_str += "PUSH NULL " + std::to_string(writeRealReg(reg)) + "    NULL //push new var\n";
    last_var++;
    return true;
  }else{
    int var_reg = findReg(1, base.lower_nodes[0]->token[1]);
    if(var_reg != -1)
      registers[var_reg][0] = -1;//if var is loaded, remove old location as it is overwritten
    registers[reg][0] = 1;//remember new var location
    registers[reg][1] = base.lower_nodes[0]->token[1];
    return false;
  }
}

void genWhile(Node& base, std::string& asm_str, int num_of_vars){
  int scope_label = scope_num;
  scope_num++;
  int start_label = scope_num;
  scope_num++;
  base.lower_nodes[0]->lower_nodes[0]->token[1] = base.lower_nodes[0]->lower_nodes[0]->token[1] * -1 + 7;
  cleanRegs(asm_str);
  asm_str += "LABEL_SCOPE_" + std::to_string(start_label) + "       //beginning of while statement\n";
  genExpression(*base.lower_nodes[0]->lower_nodes[0], asm_str);//expression is true/false, save to reg 0
  registers[0][0] = 2;
  int free_reg = findEmptyReg(asm_str);
  registers[0][0] = -1;
  registers[free_reg][0] = -1;
  asm_str += "IMM  " + std::to_string(writeRealReg(free_reg)) + "    REF_SCOPE_" + std::to_string(scope_label) + " //set jump addr\n";
  asm_str += "JMP  " + std::to_string(writeRealReg(free_reg)) + "    0    NULL //jump when if cond not satisfied\n";
  genScope(*base.lower_nodes[1], asm_str, num_of_vars);
  free_reg = findEmptyReg(asm_str);
  asm_str += "IMM  " + std::to_string(writeRealReg(free_reg)) + "    REF_SCOPE_" + std::to_string(start_label) + " //set jump addr\n";
  asm_str += "JMP  " + std::to_string(writeRealReg(free_reg)) + "    1    NULL //jump to beginning of while\n";
  cleanRegs(asm_str, num_of_vars);
  asm_str += "LABEL_SCOPE_" + std::to_string(scope_label) + "       //end of while statement\n";
}

void genIf(Node& base, std::string& asm_str, int num_of_vars){
  int scope_label = scope_num;
  scope_num++;
  base.lower_nodes[0]->lower_nodes[0]->token[1] = base.lower_nodes[0]->lower_nodes[0]->token[1] * -1 + 7;
  genExpression(*base.lower_nodes[0]->lower_nodes[0], asm_str);//expression is true/false, save to reg 0
  registers[0][0] = 2;
  int free_reg = findEmptyReg(asm_str);
  registers[0][0] = -1;
  registers[free_reg][0] = -1;
  saveVars(asm_str);
  asm_str += "IMM  " + std::to_string(writeRealReg(free_reg)) + "    REF_SCOPE_" + std::to_string(scope_label) + " //set jump addr\n";
  asm_str += "JMP  " + std::to_string(writeRealReg(free_reg)) + "    0    NULL //jump when if cond not satisfied\n";
  genScope(*base.lower_nodes[1], asm_str, num_of_vars);
  cleanRegs(asm_str, num_of_vars);
  asm_str += "LABEL_SCOPE_" + std::to_string(scope_label) + "       //end of if statement\n";
}

void genScope(Node& base, std::string& asm_str, int num_of_vars){
  int stack_pushes = 0;
  for(auto token : base.lower_nodes){
    if(isToken(*token, symbol_tokens::SCOPE_BRACKET_L)){
      genScope(*token, asm_str, num_of_vars + stack_pushes);
    } else if(isToken(*token, logic_tokens::ASSIGN)){
      if(genAssign(*token, asm_str))
        stack_pushes++;
    } else if(isToken(*token, token_types::IF))
      genIf(*token, asm_str, num_of_vars + stack_pushes);
    else if(isToken(*token, token_types::WHILE))
      genWhile(*token, asm_str, num_of_vars);
    //during a fn call remember registers and then put them back
    else if(isToken(*token, token_types::RETURN)){
      if(!token->lower_nodes.empty()){
        int reg = genExpression(*token->lower_nodes[0], asm_str);
        asm_str += "MEMW 0    " + std::to_string(writeRealReg(reg)) + "    1    //write return value\n";
      }
      asm_str += "SETSTACKPTR 1 NULL NULL //set stack ptr to base\n";
      return;
    }else if(isToken(*token, token_types::FUNCTION_CALL))
      genFnCall(*token, asm_str);
  }


  for(auto arr : registers)
    if(arr[0] == 1 && arr[1] >= num_of_vars)
      arr[0] = -1;
  int free_pop = findEmptyReg(asm_str);
  registers[free_pop][0] = -1;

  //maybe do if stack pushes > 4 => imm stack pushes, get stack ptr and subtract, then set it
  for(int i = 0; i < stack_pushes; i++){
    asm_str += "POP  NULL NULL " + std::to_string(writeRealReg(free_pop)) + "    //remove vars from inner scope at the end of said scope\n";
  }
}

std::string genFn(Node& base){//already has all vars pushed
  resetRegs();
  last_var = -1;
  while(isToken(*base.lower_nodes[0], token_types::VAR)){
    last_var++;
    base.lower_nodes.erase(base.lower_nodes.begin(), base.lower_nodes.begin() + 1);
  }
  std::string asm_str = "LABEL_FN_" + std::to_string(base.token[1]) + "          //start of a function\n";

  genScope(base, asm_str, 0);

  asm_str += "POP  NOP  NOP  0    //get ret addr\n";
  asm_str += "JMP  0    NOP  NOP  //jump to ret addr\n";
  return asm_str;
}

std::string generate(Node& root){
  std::string asm_str;
  for(auto fn : root.lower_nodes){
    asm_str += genFn(*fn);
  }
  return asm_str;
}