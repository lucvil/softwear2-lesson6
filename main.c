#include <stdio.h>

int count(){
    static int c = 0;

    c++;
    return c;
}

int main(){
    for( int i = 0; i< 10; i++){
        printf("%d\n",count());
    }
    return 0;
}