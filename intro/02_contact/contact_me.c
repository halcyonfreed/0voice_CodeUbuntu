#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define NAME_LENGTH 16
#define PHONE_LENGTH 32
#define BUFFER_LENGTH 128
#define MIN_TOKEN_LENGTH 5


#define INFO printf

#define LIST_INSERT(item, list) do{ \
    item->prev= NULL;               \
    item->next= list;               \
    if ((list) != NULL) (list)->prev = item; \
    (list)= item;                    \
} while(0)                          \

#define LIST_REMOVE(item, list) do{ \
    if(item->prev != NULL)  item->prev->next= item->next; \
    if (item->next !=NULL)  item->next->prev= item->prev;\
    if( item == list) list = item->next; \
    item->next= item->prev= NULL;   \
}while(0)                           \


struct person{
    char name[NAME_LENGTH];
    char phone[PHONE_LENGTH];

    struct person *next;
    struct person *prev;
};

struct contacts{
    struct person *people;
    int count; //数多少人
};

enum{
    OPER_INSERT = 1,
    OPER_PRINT,
    OPER_DELETE,
    OPER_SEARCH,
    OPER_SAVE,
    OPER_LOAD    
};


// define interface
int person_insert(struct person **ppeople, struct person *ps){
//int person_insert(struct person *ppeople, struct person *ps){

    if(ps  == NULL)return -1;
    LIST_INSERT(ps, *ppeople); //ps插到people里, **people是指针的指针
    return 0;
}

int person_delete(struct person **ppeople, struct person *ps){
//int person_delete(struct person *people, struct person *ps){
    if(ps == NULL) return -1;
    if (ppeople ==NULL) return -2;
    //if (people == NULL) return -2;
    LIST_REMOVE(ps, *ppeople);
    //LIST_REMOVE(ps, people);
    return 0;
}

struct person* person_search(struct person *people, const char *name){
    struct person *item = NULL; //next= null
    for(item = people; item != NULL; item = item->next){
        if(!strcmp(name, item->name)) break;//一样返回0，!strcmp = 非0就break
    }
    return item;
}

int person_traversal(struct person *people){
    struct person *item = NULL;
	for (item = people;item != NULL;item = item->next) {
        INFO("name: %s, phone: %s\n", item->name, item->phone);
    }
	return 0;
}


int save_file(struct person *people, const char *filename){
    FILE *fp = fopen(filename, "w");
    if(fp == NULL) return -1;

    struct person *item = NULL;
    for(item = people; item != NULL; item = item->next){
        fprintf(fp, "name: %s, phone: %s\n", item->name, item->phone);
        fflush(fp); //从缓存刷到磁盘里
    }
    fclose(fp);
}


//用来封装读一行以后解析出name和phone过程
int parser_token(char *buffer, int length, char *name, char *phone){
    // 提取name
    if(buffer ==NULL) return -1;
    if(length <MIN_TOKEN_LENGTH) return -2; //就不解析了

    int i = 0,j =0,  status = 0; // 默认0， 遇到空格开始提取，变成1
    for(i = 0; buffer[i]!= ','; i++){
        if(buffer[i] ==' ') status =1;
        else if(status ==1) name[j++] = buffer[i];
    }

    // 提取phone
    status = 0, j =0;
    for(; i< length; i++){
        if(buffer[i] == ' ') status =1;
        else if(status == 1) phone[j++]= buffer[i];
    }

    INFO("file token: %s --> %s\n", name, phone);
    return 0;
}


//count要改，引用传递
int load_file(struct person **ppeople, int* count, const char *filename){
    FILE *fp= fopen(filename, "r");
    if(fp == NULL) return -1;

    while(!feof(fp)){
        char buffer[BUFFER_LENGTH] = {0};
        fgets(buffer, BUFFER_LENGTH, fp); //读一行 以后解析
        int length = strlen(buffer);
        INFO("length: %d\n", length);

        char name[NAME_LENGTH] = {0};
        char phone[PHONE_LENGTH] ={0};

        //失败了就跳了
        if(0!= parser_token(buffer, length,name,phone)) continue;


        // 插入，借助pereson_insert函数
        struct person *p = (struct person *) malloc(sizeof (struct person));
        if(p== NULL) return  -2;

        memcpy(p->name, name, NAME_LENGTH);
        memcpy(p->phone, phone, PHONE_LENGTH);

        person_insert(ppeople, p);
        (*count)++;
    }
    fclose(fp);
    return 0;
}


// 业务层入口
int insert_entry(struct contacts *cts){
    if(cts == NULL) return -1;

    struct person *p = (struct person*)malloc(sizeof(struct person));
    if(p== NULL) return -2;
    //name
    INFO("please input name: \n");
    scanf("%s", p->name); //scanf可能有溢出问题，最长16位
    
    //phone
    INFO("please input phone: \n");
    scanf("%s", p->phone); 
    
    //add people
   if(0!= person_insert(&cts->people, p)){
//    if(0!= person_insert(cts->people, p)){
        //异常了，就释放
        free(p);
        return -3;
    }

    cts->count ++;
    INFO("insert success \n");
    return 0;
}


int print_entry(struct contacts *cts){
    if(cts == NULL) return -1;
    // cts->people
    person_traversal(cts->people);
}

int delete_entry(struct contacts *cts){
    if(cts == NULL) return -1;
    //name
    INFO("please input name \n");
    char name[NAME_LENGTH] = {0};
    scanf("%s", name); //指向字符数组的第一个位置的地址

    
    //person
    struct person *ps = person_search(cts->people, name);
    if(ps == NULL) {
        INFO("person don't exist\n");
        return -2;
    }
	INFO("name: %s, phone: %s\n", ps->name, ps->phone);

    //delete
    person_delete(&cts->people, ps);
    //person_delete(cts->people, ps);
    free(ps);
    
    return 0;
}

int search_entry(struct contacts *cts){
    if(cts == NULL) return -1;
    //name
    INFO("please input name \n");
    char name[NAME_LENGTH] = {0};
    scanf("%s", name); //指向字符数组的第一个位置的地址

    
    //person
    struct person *ps = person_search(cts->people, name);
    if(ps == NULL) {
        INFO("person don't exist\n");
        return -2;
    }
    INFO("name : %s, phone: %s\n", ps->name, ps->phone);

    // 上面和删除一样
    return 0;
}

int save_entry(struct contacts *cts) {
    printf("a");

	if (cts == NULL) return -1;
    

	INFO("Please Input Save Filename :\n");
	char filename[NAME_LENGTH] = {0};
	scanf("%s", filename);

	save_file(cts->people, filename);
	
}


int load_entry(struct contacts *cts){
    if(cts ==NULL) return -1;

	INFO("Please Input Load Filename :\n");
	char filename[NAME_LENGTH] = {0};
	scanf("%s", filename);
    load_file(&cts->people,&cts->count, filename);
}


// 用户操作
void menu_info(void) {

	INFO("\n\n********************************************************\n");
	INFO("***** 1. Add Person\t\t2. Print People ********\n");
	INFO("***** 3. Del Person\t\t4. Search Person *******\n");
	INFO("***** 5. Save People\t\t6. Load People *********\n");
	INFO("***** Other Key for Exiting Program ********************\n");
	INFO("********************************************************\n\n");

}


int main(){
    
    struct contacts *cts=(struct contacts *)malloc(sizeof(struct contacts));
    if(cts == NULL) return -1;

    memset(cts, 0 , sizeof (struct contacts));
    
    while(1){
        menu_info();
        int select = 0;
        scanf("%d", &select);
        
        switch (select){
            case OPER_INSERT:
                insert_entry(cts);
                break;
                
            case OPER_PRINT:
                print_entry(cts);
                break;
                
            case OPER_DELETE:
                delete_entry(cts);
                break;
                
            case OPER_SEARCH:
                search_entry(cts);
                break;
                
            case OPER_SAVE:
                save_entry(cts);
                break;
                
            case OPER_LOAD:
                load_entry(cts);
                break;

            default:
                goto exit;
        }
    }
exit:
    free(cts);
    return 0;
}

