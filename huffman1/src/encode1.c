#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include "encode1.h"


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

typedef struct haff
{
  int symbol;
  int number[30];
  int hafflong;
}Haff;

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
static void traverse_tree(const int depth, Node *np,int haff[],int *all);

//ハフマン符号で圧縮する
static void zip_huffman(const char *filename1, Node *huffman,int all);

//ハフマン符号で復元する
//static void unzip_huffman();

static void find_tree(const int depth,  Node *np, int c, int **answer);

static void symbol_tree(const int depth,  Node *np,FILE *fpout);

static void unzip_huffman(FILE *fp,Haff haffman[],int all);



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
  printf("%d",symbol_count[26]);
  symbol_count[26] = 1; //<-EOFとして扱う　huffman1で新たに追加

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
static void traverse_tree(const int depth,  Node *np, int haff[],int *all)
{			  
  if (np->left == NULL){
    *all += 1;
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
        }else if(np->symbol == 26){
          printf(" \e[32mEOF\e[0m");
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
  traverse_tree(depth + 1, np->left, haff,all);
  haff[depth] = -1;

  haff[depth] = 1;
  traverse_tree(depth + 1, np->right, haff,all);
  haff[depth] = -1;
}

// この関数のみ外部 (main) で使用される (staticがついていない)
int encode(const char *filename1, const char *control)
{
  if(strcmp(control,"zip") == 0){
    count_symbols(filename1);
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

    int all = 0;
    traverse_tree(0, root,haff,&all);
    zip_huffman(filename1, root,all);
  }else if( strcmp(control,"unzip") == 0){
    FILE *fp = fopen(filename1,"rb");
    if(fp == NULL){
      fprintf(stderr, "error: cannot open %s\n", filename1);
      exit(1);
    }

    //事前情報の取得
    int all;
    int binarylong;
    fread(&all,sizeof(int),1,fp); //①シンボルの種類数
    fread(&binarylong,sizeof(int),1,fp); //②バイナリコードに何個分のcharとしてハフマン符号をかくか
    Haff haffman[all];
    for(int i = 0; i < all; i++){
      fread(&(haffman[i].symbol),sizeof(int),1,fp); //③シンボル
      int hafflong;
      fread(&hafflong,sizeof(int),1,fp);//④シンボルのハフマン符号の長さ
      
      for(int j = 0; j < 30; j++){
        haffman[i].number[j] = -1;
      }
      unsigned char buf[binarylong];
      int copy[binarylong * 8];
      for(int j = 0; j < binarylong; j++){
        fread(&(buf[j]),sizeof(unsigned char),1,fp);
        for(int k = 0; k < 8; k++){
          copy[ 8*j + k ] = buf[j]/pow(2,7-k);
          buf[j] -= copy[ 8*j + k ] * pow(2,7-k);
        }
      }
      haffman[i].hafflong = hafflong;
      for(int j = 0; j < hafflong; j++){
        haffman[i].number[j] = copy[j];
      } //⑤ハフマン符号      
    }


    //ハフマン符号からデータを解凍する
    unzip_huffman(fp,haffman,all);
  }
        

  return EXIT_SUCCESS;
}

//
static void zip_huffman(const char *filename1, Node *root,int all){
  //ファイルを開ける
  FILE *fp;

  if( (fp = fopen( filename1,"rb")) == NULL){
    fprintf(stderr, "%sのオープンに失敗しました。\n", filename1);
    exit(EXIT_FAILURE);
  }
  
  //バイナリファイルに書き込む
  //事前情報の書き込み
  int c = 0;
  int *answer = NULL;
  char *outfilename = "sample.dat";
  FILE *fpout;
  if( (fpout = fopen(outfilename, "wb") ) == NULL){
    fprintf(stderr, "%sのオープンに失敗\n", outfilename);
    return;
  }
  
  fwrite(&all,sizeof(int),1,fpout);//①シンボルの種類数
  int binarylong = 3; 
  fwrite(&binarylong,sizeof(int),1,fpout); //②バイナリコードに何個分のcharとしてハフマン符号をかくか
  int depth1 = 0;
  symbol_tree(depth1,root,fpout);//③シンボル　④シンボルのハフマン符号の長さ　⑤ハフマン符号

  //本編の書き込み
  int buf[8];
  int j = 0;//bufの何個目をみているか

  
  while(1){
    c = fgetc(fp);
    if( c != EOF){
      int depth = 0;
      //answerにハフマンコードの配列の先頭ポインタを入れる
      find_tree(depth,root,c,&answer);

      int i = 0;//answerのnumberの何個目をみてるか
    
    
    
      while(answer[i] != -1){
        buf[j] = answer[i];

        if(j == 7){
          int mass = 0;
          for(int b = 0; b < 8; b++){
            mass += buf[7-b] * pow(2,b);
          }
          unsigned char output = (unsigned char)mass;
          fwrite(&output,sizeof(unsigned char),1,fpout);
          j = -1;
        }
        i++;
        j++;
      }

    }else if(( c = fgetc(fp)) == EOF){
      int depth = 0;
      int Eof = 26;
      find_tree(depth,root,Eof,&answer);
      int i = 0;
      while(answer[i] != -1){
        buf[j] = answer[i];
        if(j == 7){
          int mass = 0;
          for(int b = 0; b < 8; b++){
            mass += buf[7-b] * pow(2,b);
          }
          unsigned char output = (unsigned char)mass;
          fwrite(&output,sizeof(unsigned char),1,fpout);
          j = -1;
        }
        i++;
        j++;
      }
      //おしりの調整(EOFを入れた後一バイトの空いた部分に1を入れバイナリファイルに出力)
      for(int k = 0; k < 8-j;k++){
        buf[j] = 1;
        j++;
      }
      int mass = 0;
      for(int b = 0; b < 8; b++){
        mass += buf[7-b] * pow(2,b);
      }
      unsigned char output = (unsigned char)mass;
      fwrite(&output,sizeof(unsigned char),1,fpout);
      break;
    }
  } 
  
}

//
static void find_tree(const int depth,  Node *np, int c, int **answer)
{	
  if (np->left == NULL){
    if( c == np->symbol){
      *answer =  np->number;
      return;
    }else{
      return;
    }
  }  

  find_tree(depth + 1, np->left, c, answer);
  find_tree(depth + 1, np->right, c, answer);
  
}


//
static void symbol_tree(const int depth,  Node *np, FILE *fpout)
{			  
  if (np->left == NULL){
    fwrite(&(np->symbol),sizeof(int),1,fpout);//③シンボル

    int n = 0;
    int i = 0;
    while(np->number[i] != -1){
      n++;
      i++;
    }
    fwrite(&n,sizeof(int),1,fpout);//④シンボルのハフマン符号の長さ

    i = 0;
    int buf[8];
    int j = 0; //bufのどこをみているか
    while(i < 24){  //<-２４は②によって決まる。
      if(np->number[i] != -1){
        buf[j] = np->number[i];
      }else if(np->number[i] == -1){
        buf[j] = 0;
      }
      
      if(j == 7){
        int mass = 0;
        for(int b = 0; b < 8; b++){
          mass += buf[7-b] * pow(2,b);
        }
        unsigned char output = (unsigned char)mass;
        fwrite(&output,sizeof(unsigned char),1,fpout);//⑤ハフマン符号(3バイト)
        j = -1;
      }
      i++;
      j++;
    }
    return;
  }

  symbol_tree(depth + 1, np->left, fpout);

  symbol_tree(depth + 1, np->right, fpout);

}



static void unzip_huffman(FILE *fp,Haff haffman[],int all){
  int buf3[32];
  int buf2[8];
  char buf1;
  //最初にbuf3を埋めておく
  for(int i = 0; i < 4; i++){
    fread(&(buf1),sizeof(unsigned char),1,fp);
    for(int k = 0; k < 8; k++){
      buf2[k] = buf1/pow(2,7-k);
      buf1 -= buf2[k] * pow(2,7-k);
    }
    for(int k = 0; k < 8; k++){
      buf3[8*i + k] = buf2[k];
    }
  }

  int end = 31; //buf3の終わりはどこか

  //出力用のファイルを開いておく
  FILE *fpout;
  char *outfilename = "sample.txt"; 
  if( (fpout = fopen(outfilename, "w") ) == NULL){
    fprintf(stderr, "%sのオープンに失敗\n", outfilename);
    return;
  }

  while(1){
    //buf3の補充
    if(end <= 23){
      int c = fread(&(buf1),sizeof(unsigned char),1,fp);
      if(c != 0){
        for(int k = 0; k < 8; k++){
          buf2[k] = buf1/pow(2,7-k);
          buf1 -= buf2[k] * pow(2,7-k);
        }
        for(int k = 0; k < 8; k++){
          buf3[end + 1 + k] = buf2[k];
        } 
        end += 8;
      }
    }

    //解凍
    int c;
    int check2 = 0;
    int ilong;
    for(int i = 0; i < 30; i++){//haffmanの文字の多さ
      for(int j = 0; j < all;j++){//haffmanの番号
        if(haffman[j].hafflong == i+1){
          int check = 1;
          for(int k = 0; k <= i;k++){//haffmanのnumberの番号
            if(haffman[j].number[k] != buf3[k]){
              check = 0;
            }
          }
          if(check == 1){
            c = haffman[j].symbol;
            ilong = i+1;
            check2 = 1;
            break;
          }
        }
      }
      if(check2 == 1){
        check2 =0;
        break;
      }
    }

    if( c != 26){
      fputc(c,fpout);
    }else if(c == 26){
      break;
    }

    //buf3の文字数の調整
    for(int i = ilong;i <= end;i++){
      buf3[i-ilong] = buf3[i];
    }
    end = end - ilong;

  }

}