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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
//#include <string>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
uint32_t buf_index = 0;

static uint32_t choose(uint32_t n);
static void gen_num();
static void gen(char c);
static void gen_rand_op();
/*
$ gcc ./gen-expr.c -o ,.gen-expr
$ ./gen-expr 3 > input 
*/
static void gen_rand_expr() {
  if (buf_index > 65535) assert(0);
  switch (choose(3)) {
    // case 0, then case 1: 76(48) can't be calculated.
    case 0: 
      gen_num(); 
      break;
    case 1: 
      gen('('); 
      gen_rand_expr(); 
      gen(')'); 
      break;
    default: 
      gen_rand_expr(); 
      gen_rand_op(); 
      gen_rand_expr(); 
      break;
  }
}

static uint32_t choose(uint32_t n) {
  uint32_t flag = rand() % n;
  // printf("buf_index: %u, flag: %u", buf_index, flag);
  return flag;
}
/*
static void gen_num() {
  // trans random int to char*
  uint32_t num = rand() % 100;
  std::string str = std::to_string(num);
  for (int i = 0; i < str.length() - 1; i++) {
    buf[buf_index++] = str[i];
  }
}
*/
static void gen_num() {
  int num = rand() % 100;
  int num_size = 0, num_tmp = num;
  while(num_tmp) {
    num_tmp /= 10;
    num_size++;
  }
  int x = 1;
  while(num_size > 1) {
    x *= 10;
    num_size--;
  }
  // x: 100, 10, 1
  while(num) {
    char c = num / x + '0';
    num %= x;
    x /= 10;
    buf[buf_index++] = c;
  }
}
static void gen(char c) {
  buf[buf_index++] = c;
}

static void gen_rand_op() {
  char op[4] = {'+', '-', '*', '/'};
  uint32_t flag = rand() % 4;
  buf[buf_index++] = op[flag];
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
