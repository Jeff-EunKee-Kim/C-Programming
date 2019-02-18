#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define PI 3.14159265358979323846

struct pData{
  char name[64];
  float ppd;
  struct pData* next;
};

void swap(pData* a, pData* b){
  float temp1;
  char temp2[64];
  temp1 = a->ppd;
  a->ppd = b->ppd;
  b->ppd = temp1;
  strcpy(temp2, a->name);
  strcpy(a->name, b->name);
  strcpy(b->name, temp2);
  a->next = b;
}

void sort(pData* list){
  bool sorted = false;
  struct pData *temp;
  temp = list;
  while(sorted != true){
    sorted = true;
    list = temp;
    while(list->next != NULL){
      if(list->ppd < list->next->ppd){
        swap(list, list->next);
        sorted = false;
      }
      if(list->ppd == list->next->ppd){
        if(strcmp(list->name, list->next->name) > 0){
          swap(list, list->next);
        }
      }
      list = list->next;
    }
  }
}

void printlist(pData* list){
  while(list != NULL){
    printf("%s %f\n", list->name, list->ppd);
    list = list->next;
  }
}

void freelist(pData* list){
  while (list != NULL) {
    pData* temp = list;
    list = list->next;
    free(temp);
  }
}

int main(int argc, char* argv[]){
  char* name = (char*) malloc (64);
  float diameter, dollar, ppd;
  struct pData *head = NULL;
  struct pData *temp = NULL;

  FILE *file = fopen(argv[1], "r");

  while(true){
    struct pData *curr;
    curr = (struct pData*)malloc(sizeof(struct pData));

    if(fscanf(file, "%s", name) == EOF){
      printf("PIZZA FILE IS EMPTY\n");
      free(curr);
      free(name);
      fclose(file);
      return 0;
    }

    if(strcmp(name, "DONE") == 0){
      free(curr);
      break;
    }

    fscanf(file, "%f %f", &diameter, &dollar);

    strcpy(curr->name, name);
    if(dollar == 0){
      curr->ppd = 0;
    }
    else{
      curr->ppd = (0.25 * PI * diameter * diameter) / dollar;
    }
    curr->next = NULL;

    if(head == NULL){
      head = curr;
      temp = curr;
    }
    else{
      temp->next = curr;
      temp = temp->next;
    }
  }

  sort(head);
  printlist(head);
  freelist(head);
  free(name);
  fclose(file);

  return 0;
}
