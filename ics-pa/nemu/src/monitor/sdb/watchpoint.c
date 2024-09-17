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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  bool utilized;
  char expr[200];
  int old_val;
  int new_val;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp() {
  for (WP* p = free_; p->next != NULL; p = p -> next) {
    if (p -> utilized == false) {
      p -> utilized = true;
      if (head == NULL) {
        head = p;
      }
      return p;
    }
  }
  panic("Out of NR_WP. No more free watchpoint for using.");
  assert(0);
  return NULL;
}

void free_wp(WP *wp) {
  if (head -> NO == wp -> NO) {
    head -> next = NULL;
    head -> utilized = false;
    printf("Successfully delete head watchpoint.\n");
  }
  for (WP* p = head; p -> next != NULL; p = p -> next) {
    if (p -> next -> NO == wp -> NO) { // previous watchpoint
      p -> next -> utilized = false;
      p -> next = wp -> next;
      printf("Successfully free watchpoint.\n"); 
    }
  }
}

void create_watchpoint(char *args) {
  WP *p = new_wp();
  strcpy(p -> expr, args);
  bool success = true;
  int tmp_val = expr(p -> expr, &success);
  printf("%s with value: %d\n", p -> expr, tmp_val);
  if (success) { 
    p -> old_val = tmp_val;
    Log("Create watchpoint No.%d successfully.", p -> NO);
  }
  else printf("Problem occurs when create the watchpoint.\n");
}
void delete_watchpoint(int no) {
  for (int i = 0; i < NR_WP; i++) {
    if (wp_pool[i].NO == no)  free_wp(&wp_pool[i]);
  }
}
void sdb_watchpoint_display() {
  for (int i = 0; i < NR_WP; i++) {
    if (wp_pool[i].utilized) {
      printf("Watchpoint.No: %d, expr = \"%s\", old_val = %d, new_val = %d.\n", wp_pool[i].NO, wp_pool[i].expr, wp_pool[i].old_val, wp_pool[i].new_val);
    }
  }
}
