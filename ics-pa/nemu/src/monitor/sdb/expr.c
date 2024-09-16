/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, NUM

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {"[0-9]+", NUM},
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus, the first is the Escape Character in C and the second is in Regex
  {"\\-", '-'},
  {"\\*", '*'},
  {"\\/", '/'},
  {"\\(", '('},
  {"\\)", ')'},
  {"==", TK_EQ},        // equal
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  // Lexical Analyzer
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
	  case TK_NOTYPE:
	    //printf("SPACE\n");
	    break; 
	  case '+':
	    tokens[nr_token++].type = '+';
	    break;
	  case '-':
	    tokens[nr_token++].type = '-';
	    break;
	  case '*':
	    tokens[nr_token++].type = '*';
	    break;
	  case '/':
	    tokens[nr_token++].type = '/';
	    break;
	  case '(':
	    tokens[nr_token++].type = '(';
	    break;
	  case ')':
	    tokens[nr_token++].type = ')';
	    break;
	  case NUM:
	    tokens[nr_token].type = NUM;
	    strncpy(tokens[nr_token].str, e + position - substr_len, substr_len);
	    nr_token++;
	    // Log("In case NUM.");
	    break;	    
          default:
	    printf("i = %d no rules.\n", i);
	    break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q);
uint32_t eval(int p, int q);
int get_main_op_pos(int p, int q);

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //printf("%d\n", nr_token);
  //int length = strlen(e);
  //Log("The length of char *e is: %d", length);
  uint32_t res = eval(0, nr_token - 1);
  printf("The result is: %u.\n", res);

  return 0;
}

bool check_parentheses(int p, int q){
  //Log("check_parentheses is calling.");
  //Log("check_parentheses with p value: %d, q value: %d", p, q);
  // looks simple. Syntax error is weakness.
  if ((q - p) < 2 || tokens[p].type != '(' || tokens[q].type != ')'){
    return false;
  }
  int balance = 0;
  for (; p <= q; p++){
    if (tokens[p].type == '(') balance++;
    else if (tokens[p].type == ')') balance--;
    else if (balance < 0) assert(0); // syntax errors maybe happen
  }
  //Log("The value of balance: %d", balance);
  return balance == 0;
}

uint32_t eval(int p, int q){
  if (p > q){
    assert(0);
    return -1;
  }
  else if (p == q){
    // single token should be a number
    if (tokens[p].type == NUM) return atoi(tokens[p].str);
    else assert(0); 
  }
  else if (check_parentheses(p, q) == true){
    //Log("check_parentheses(p, q): OK");
    //Log("check_parentheses(p, q) with p value: %d, q value: %d", p, q);
    return eval(p + 1, q - 1);
  }
  else{
    int op_pos = get_main_op_pos(p, q);
    //Log("Get op_pos position: %d", op_pos);
    uint32_t value1 = eval(p, op_pos - 1);
    //Log("Get value1: %d", value1);
    uint32_t value2 = eval(op_pos + 1, q);
    //Log("Get value2: %d", value2);

    switch(tokens[op_pos].type){
      case '+':
	//Log("SUM!");
        return value1 + value2;
      case '-':
	return value1 - value2;
      case '*':
        return value1 * value2;
      case '/':
	if (value2 == 0) panic("Divide by zero\n");
	return value1 / value2;
      default: assert(0);
    }
  }
}

int get_main_op_pos(int p, int q){
  int precedence = false;
  if (p > q){
    assert(0);
    return -1;
  }
  bool ignorance = false;
  int pos = p;
  //Log("p value: %d, q value: %d", p, q);
  for (; p < q; p++){
    //Log("In get_main_op_pos *for* loop");
    //printf("ignorance: %d\n", ignorance);
    if (tokens[p].type == TK_NOTYPE) continue; 
    else if (tokens[p].type == '(') ignorance = true;
    else if (tokens[p].type == ')') ignorance = false;
    else if ((ignorance == false) && (p < q)){
      if ((tokens[p].type == '*' || tokens[p].type == '/') && (precedence == false)){
        pos = p;
      }
      else if (tokens[p].type == '+' || tokens[p].type == '-'){
        pos = p;
        precedence = true;	
      }
    }
  }
  return pos;
}
