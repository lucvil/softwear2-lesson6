#ifndef TRAVERSE_TRAVERSE_H
#define TRAVERSE_TRAVERSE_H

typedef struct node
{
  int value;
  struct node *left;
  struct node *right;
} Node;

// 木のルートを与えて操作する
void traverse(const Node *n);

#endif
