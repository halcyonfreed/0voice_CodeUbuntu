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
    if (fp == NULL)  return -1; //��C�����У�FILE ��һ���������ͣ����ڱ�ʾ�ļ�����file stream������ͨ�����ڴ򿪡���ȡ��д���ļ���

    // һ�����ַ����룬�ļ���βEOF
    char c; 
    int word = 0;
    while( (c = fgetc(fp)) != EOF){
        if(split(c)){
            status = OUT;
        }
        else if (status == OUT){
            // ����Ҫд��������Ϊͳ�Ƶ��ʣ�������ĸ����
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
