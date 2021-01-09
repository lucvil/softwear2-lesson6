#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "encode0.h"


// 各シンボルの数を数える為にポインタを定義
// 数を数える為に、1byteの上限+1で設定しておく
static const int nsymbols = 256 + 1; 
//int symbol_count[nsymbols];
int * symbol_count;
 

// ノードを表す構造体
typedef struct node
{
  int symbol;
  int count;
  int number[30];
  struct node *left;
  struct node *right;
} Node;

// このソースで有効なstatic関数のプロトタイプ宣言
// 一方で、ヘッダファイルは外部からの参照を許す関数の宣言のみ


// ファイルを読み込み、static配列の値を更新する関数
static void count_symbols(const char *filename);

// 与えられた引数でNode構造体を作成し、そのアドレスを返す関数
static Node *create_node(int symbol, int count, Node *left, Node *right);

// Node構造体へのポインタが並んだ配列から、最小カウントを持つ構造体をポップしてくる関数
// n は 配列の実効的な長さを格納する変数を指している（popするたびに更新される）
static Node *pop_min(int *n, Node *nodep[]);

// ハフマン木を構成する関数（根となる構造体へのポインタを返す）
static Node *build_tree(void);

// 木を深さ優先で操作する関数
static void traverse_tree(const int depth, Node *np,int haff[]);



// 以下 static関数の実装
static void count_symbols(const char *filename)
{
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    fprintf(stderr, "error: cannot open %s\n", filename);
    exit(1);
  }

  symbol_count = (int*)calloc(nsymbols, sizeof(int));
  
  int c = 0;
  while( ( c = fgetc(fp)) != EOF){
    symbol_count[c]++;
  }

  fclose(fp);
}

static Node *create_node(int symbol, int count, Node *left, Node *right)
{
  Node *ret = (Node *)malloc(sizeof(Node));
  *ret = (Node){ .symbol = symbol, .count = count, .left = left, .right = right};
  return ret;
}

static Node *pop_min(int *n, Node *nodep[])
{
  // Find the node with the smallest count
  // カウントが最小のノードを見つけてくる
  int argmin = 0;
  for (int i = 0; i < *n; i++) {
    if (nodep[i]->count < nodep[argmin]->count) {
      argmin = i;
    }
  }

  Node *node_min = nodep[argmin];

  // Remove the node pointer from nodep[]
  // 見つかったノード以降の配列を前につめていく
  for (int i = argmin; i < (*n) - 1; i++) {
    nodep[i] = nodep[i + 1];
  }
  // 合計ノード数を一つ減らす
  (*n)--;

  return node_min;
}

static Node *build_tree()
{
  int n = 0;
  Node *nodep[nsymbols];

  for (int i = 0; i < nsymbols; i++) {
    // カウントの存在しなかったシンボルには何もしない
    if (symbol_count[i] == 0) continue;

    // 以下は nodep[n++] = create_node(i, symbol_count[i], NULL, NULL); と1行でもよい
    nodep[n] = create_node(i, symbol_count[i], NULL, NULL);
    n++;
  }

  const int dummy = -1; // ダミー用のsymbol を用意しておく
  while (n >= 2) {
    Node *node1 = pop_min(&n, nodep);
    Node *node2 = pop_min(&n, nodep);

    // Create a new node
    // 選ばれた2つのノードを元に統合ノードを新規作成
    // 作成したノードはnodep にどうすればよいか?
    
    nodep[n++] = create_node(dummy,node1->count + node2->count,node1,node2);

  }
  // 気にした後は symbol_counts は free
  free(symbol_count);
  return (n==0)?NULL:nodep[0];
}

// Perform depth-first traversal of the tree
// 深さ優先で木を走査する
// 現状は何もしていない（再帰してたどっているだけ）
static void traverse_tree(const int depth,  Node *np, int haff[])
{			  
  if (np->left == NULL){
    for(int i = 0; i < 30; i++){
      np->number[i] = haff[i];
    }

    //ツリー構造表示
    //一行目
    //最後に一が出てくる場所を探す
    int number1 = -1; //最後に1が出る場所
    int a = 0;
    while(np->number[a] != -1){
      if(np->number[a] == 1){
        number1 = a;
      }
      a++;
    }

    a = 0;
    while(np->number[a] != -1){
      if( a < number1){
        if(np->number[a] == 0){
          printf("|   ");
        }else if(np->number[a] == 1){
          printf("    ");
        }
      }else if(a >= number1){
          if(np->number[a] == 1){
            printf("+-1-");
          }else if(np->number[a] == 0){
            printf("+-0-");
          }
      }
      

      if(np->number[a+1] == -1){
        if(np->symbol == 10){
          printf(" \e[32m改行\e[0m");
        }else{
          printf(" \e[32m%c\e[0m",(char)np->symbol);
        }
        
        printf(" \e[31m(\e[0m");
        int n = 0;
        while(np->number[n] != -1){
          printf("\e[31m%d\e[0m",np->number[n]);
          n++;
        }
        n = 0;
        printf("\e[31m)\e[0m");
      }
      a++;
    }
    printf("\n");

    //二行目
    a = 0;
    while(haff[a] != -1){
      if(haff[a] == 0){
        printf("|   ");
      }else if(haff[a] == 1){
        printf("    ");
      }
      a++;
    }
    printf("\n");

    return;
  }

  haff[depth] = 0;
  traverse_tree(depth + 1, np->left, haff);
  haff[depth] = -1;

  haff[depth] = 1;
  traverse_tree(depth + 1, np->right, haff);
  haff[depth] = -1;
}

// この関数のみ外部 (main) で使用される (staticがついていない)
int encode(const char *filename)
{
  count_symbols(filename);
  Node *root = build_tree();

  if (root == NULL){
    fprintf(stderr,"A tree has not been constructed.\n");
    return EXIT_FAILURE;
  }
  
  //ハフマン符号の記憶
  int haff[30];
  for( int i = 0; i < 30; i++){
    haff[i] = -1;
  }

  traverse_tree(0, root,haff);
  return EXIT_SUCCESS;
}
