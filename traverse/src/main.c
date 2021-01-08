#include <stdio.h>
#include <stdlib.h>
#include "traverse.h"


int main(int argc, char **argv)
{
  // ノードを10個ほど定義
  Node n1  = { .value = 1, .left = NULL, .right = NULL};
  Node n2  = { .value = 2, .left = NULL, .right = NULL };
  Node n3  = { .value = 3, .left = NULL, .right = NULL };
  Node n4  = { .value = 4, .left = NULL, .right = NULL };
  Node n5  = { .value = 5, .left = NULL, .right = NULL };
  Node n6  = { .value = 6, .left = NULL, .right = NULL };
  Node n7  = { .value = 7, .left = NULL, .right = NULL };
  Node n8  = { .value = 8, .left = NULL, .right = NULL };
  Node n9  = { .value = 9, .left = NULL, .right = NULL };
  Node n10 = { .value =10, .left = NULL, .right = NULL };

  // 木を繋いでみる。木の形はスライド参照
  
  n1.right = &n2;
  n1.left  = &n7;
  n2.right = &n6;
  n2.left  = &n3;
  n3.right = &n5;
  n3.left  = &n4;
  n7.right = &n10;
  n7.left  = &n8;
  n8.left  = &n9;

  // 走査する
  traverse(&n1);

  return EXIT_SUCCESS;
}
