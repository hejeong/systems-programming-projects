struct head {
  struct node* row;
  struct head* next;
};

struct node{
   void *value;
   struct node* next;
};

struct node* createNode(){
 struct node* newNode = (struct node*)malloc(sizeof(struct node));

 newNode->next = NULL;
 return newNode;
}
