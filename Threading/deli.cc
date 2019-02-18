#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "thread.h"
using namespace std;

const int BOARD_FULL = 1;
const int BOARD_NOT_FULL =0;
const int LOCK_ON_BOARD = 2;
const int LOCK_ON_CASHIER = 3;
const int LOCK_ON_MONITOR = 4;

struct order_id {
  int cashier_id;
  int sandwich_id;
};

vector<order_id> board;

int *order_status;
int max_orders, number_of_cashier, open_cashier; 
bool full, finish;

void sandwich_maker() {
  int last_sandwich = -1;

  while(1) {
    int count = 0;
    int max = 1000;
    order_id next_sandwich;

    thread_lock(LOCK_ON_BOARD);
    while(!full){
      thread_wait(LOCK_ON_BOARD, BOARD_FULL);  
    }  
    if(finish){
      break;
    }

    for(int i = 0 ; i < board.size() ; i++){    
      int diff = abs(last_sandwich - board[i].sandwich_id);
      if(diff < max){
        max = diff;
        next_sandwich = board[i];
        count = i;
      }
    }

    board.erase(board.begin() + count); 
    last_sandwich = next_sandwich.sandwich_id; 
    order_status[next_sandwich.cashier_id] = 1; 

    thread_lock(LOCK_ON_MONITOR);
    cout << "READY: cashier " << next_sandwich.cashier_id << " sandwich " << next_sandwich.sandwich_id << endl; 
    thread_unlock(LOCK_ON_MONITOR);

    full = false;
    thread_broadcast(LOCK_ON_BOARD, BOARD_NOT_FULL); 
    thread_unlock(LOCK_ON_BOARD);
  }
  free(order_status);
}

void cashier_gen(FILE* file){
  int cid;

  thread_lock(LOCK_ON_CASHIER);
  cid = open_cashier++;
  thread_unlock(LOCK_ON_CASHIER);
  
  int sid = -1;
  order_status[cid] = 1;

  while(fscanf(file, "%d", &sid) != EOF){
    thread_lock(LOCK_ON_BOARD);               
    while(full || order_status[cid] == 0){
      thread_wait(LOCK_ON_BOARD, BOARD_NOT_FULL);
    } 
    order_id order;
    order.cashier_id = cid; 
    order.sandwich_id = sid; 
    board.push_back(order); 
    order_status[cid] = 0;

    thread_lock(LOCK_ON_MONITOR);
    cout << "POSTED: cashier " << cid << " sandwich " << sid << endl; 
    thread_unlock(LOCK_ON_MONITOR);

    if(board.size() == max_orders){
      full = true; 
      thread_signal(LOCK_ON_BOARD, BOARD_FULL);
    }
    thread_unlock(LOCK_ON_BOARD);
  }

  thread_lock(LOCK_ON_BOARD);
  while(order_status[cid] == 0){
    thread_wait(LOCK_ON_BOARD, BOARD_NOT_FULL);
  }
  thread_unlock(LOCK_ON_BOARD);

    thread_lock(LOCK_ON_CASHIER); 
    if(--open_cashier < max_orders){
      max_orders = open_cashier;  
      if(open_cashier == 0){
        finish = true;
      }
      thread_lock(LOCK_ON_BOARD);
      full = true;
      thread_signal(LOCK_ON_BOARD, BOARD_FULL);
      thread_unlock(LOCK_ON_BOARD);
    }
    thread_unlock(LOCK_ON_CASHIER);
  fclose(file);
}

void initialize(char* files[]){
  order_status = (int *) malloc(sizeof(int)* number_of_cashier);
  for(int i = 2 ; i < number_of_cashier + 2 ; i ++){
    thread_create((thread_startfunc_t) cashier_gen, fopen(files[i], "r"));
  }
  thread_create((thread_startfunc_t) sandwich_maker, NULL);
}

int main(int argc, char* argv[]){
  number_of_cashier = argc - 2;
  open_cashier = 0;
  full = false;
  finish = number_of_cashier <= 0;
  sscanf(argv[1], "%d", &max_orders);
  if(max_orders < number_of_cashier){
    max_orders = max_orders;
  }else{
    max_orders = number_of_cashier;
  }
  thread_libinit((thread_startfunc_t) initialize, argv);
  return 0;
}