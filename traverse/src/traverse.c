#include <stdio.h>
#include "traverse.h"

void traverse(const Node *n)
{
  if (n == NULL) return;

  // 以下に木を操作する手順を実装する
  // どうすればよいか？
  // 表示は printf("value = %d\n", n->value); のような形で良い

  printf("value - %d\n",n->value);

  traverse(n->left);

  traverse(n->right);

}
