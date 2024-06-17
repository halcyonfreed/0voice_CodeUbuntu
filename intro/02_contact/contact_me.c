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
    int count; //��������
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
    LIST_INSERT(ps, *ppeople); //ps�嵽people��, **people��ָ���ָ��
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
        if(!strcmp(name, item->name)) break;//һ������0��!strcmp = ��0��break
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
        fflush(fp); //�ӻ���ˢ��������
    }
    fclose(fp);
}


//������װ��һ���Ժ������name��phone����
int parser_token(char *buffer, int length, char *name, char *phone){
    // ��ȡname
    if(buffer ==NULL) return -1;
    if(length <MIN_TOKEN_LENGTH) return -2; //�Ͳ�������

    int i = 0,j =0,  status = 0; // Ĭ��0�� �����ո�ʼ��ȡ�����1
    for(i = 0; buffer[i]!= ','; i++){
        if(buffer[i] ==' ') status =1;
        else if(status ==1) name[j++] = buffer[i];
    }

    // ��ȡphone
    status = 0, j =0;
    for(; i< length; i++){
        if(buffer[i] == ' ') status =1;
        else if(status == 1) phone[j++]= buffer[i];
    }

    INFO("file token: %s --> %s\n", name, phone);
    return 0;
}


//countҪ�ģ����ô���
int load_file(struct person **ppeople, int* count, const char *filename){
    FILE *fp= fopen(filename, "r");
    if(fp == NULL) return -1;

    while(!feof(fp)){
        char buffer[BUFFER_LENGTH] = {0};
        fgets(buffer, BUFFER_LENGTH, fp); //��һ�� �Ժ����
        int length = strlen(buffer);
        INFO("length: %d\n", length);

        char name[NAME_LENGTH] = {0};
        char phone[PHONE_LENGTH] ={0};

        //ʧ���˾�����
        if(0!= parser_token(buffer, length,name,phone)) continue;


        // ���룬����pereson_insert����
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


// ҵ������
int insert_entry(struct contacts *cts){
    if(cts == NULL) return -1;

    struct person *p = (struct person*)malloc(sizeof(struct person));
    if(p== NULL) return -2;
    //name
    INFO("please input name: \n");
    scanf("%s", p->name); //scanf������������⣬�16λ
    
    //phone
    INFO("please input phone: \n");
    scanf("%s", p->phone); 
    
    //add people
   if(0!= person_insert(&cts->people, p)){
//    if(0!= person_insert(cts->people, p)){
        //�쳣�ˣ����ͷ�
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
    scanf("%s", name); //ָ���ַ�����ĵ�һ��λ�õĵ�ַ

    
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
    scanf("%s", name); //ָ���ַ�����ĵ�һ��λ�õĵ�ַ

    
    //person
    struct person *ps = person_search(cts->people, name);
    if(ps == NULL) {
        INFO("person don't exist\n");
        return -2;
    }
    INFO("name : %s, phone: %s\n", ps->name, ps->phone);

    // �����ɾ��һ��
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


// �û�����
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

