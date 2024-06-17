#include<stdio.h>


#define OUT 0
#define IN 1
#define INIT OUT

int split(char c){
    if((' ' == c) || ('\n' == c) || ('\t' == c) ||
			('\"' == c) || ('\'' == c) || ('+' == c) ||
			(',' == c) || (';' == c) || ('.' == c))
			return 1;
    else
        return 0;
}
int count_word(char *filename){
    int status = INIT;
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)  return -1; //在C语言中，FILE 是一个数据类型，用于表示文件流（file stream）。它通常用于打开、读取和写入文件。

    // 一个个字符读入，文件结尾EOF
    char c; 
    int word = 0;
    while( (c = fgetc(fp)) != EOF){
        if(split(c)){
            status = OUT;
        }
        else if (status == OUT){
            // 这里要写条件，因为统计单词，不是字母个数
            status = IN;
            word ++;
        }
            
    }
    return word;
}



int main(int argc, char* argv[]){
    if(argc < 2)  return -1;
    printf("word: %d\n", count_word(argv[1]));
}
