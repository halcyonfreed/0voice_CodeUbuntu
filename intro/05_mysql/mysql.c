#include<mysql.h>
#include<stdio.h>
#include<string.h>

#define KING_DB_SERVER_IP "192.168.243.128"
#define KING_DB_SERVER_PORT 3306
#define KING_DB_PASSWORD "123456"
#define KING_DB_USERNAME "admin"
#define KING_DB_DEFAULTDB "KING_DB"
#define SQL_INSERT_TBL_USER "INSERT TBL_USER(U_NAME, U_GENDER) VALUES('King', 'man');"
// #define SQL_INSERT_TBL_USER "INSERT TBL_USER(U_NAME, U_GENDER) VALUES('qiuxiang', 'woman');"

#define SQL_SELECT_TBL_USER "SELECT * FROM TBL_USER;"

#define SQL_DELETE_TBL_USER "CALL PROC_DELETE_USER('King')"

#define SQL_INSERT_IMG_USER "INSERT TBL_USER(U_NAME, U_GENDER, U_IMG) VALUES('King', 'man', ?);"
//  ?是不确定的占位符！！
#define SQL_SELECT_IMG_USER		"SELECT U_IMG FROM TBL_USER WHERE U_NAME='King';"
#define FILE_IMAGE_LENGTH (64*1024)

int king_mysql_select(MYSQL *handle){
    // mysql_real_query--> sql
    if(mysql_real_query(handle, SQL_SELECT_TBL_USER, strlen(SQL_SELECT_TBL_USER))){
        printf("mysql_real_query: %s\n", mysql_error(handle));
        return -1;
    };

    // 把管道里的data 存起来store
    MYSQL_RES *res = mysql_store_result(handle);
    if(res ==NULL){
        printf("mysql_store_result: %s\n", mysql_error(handle));
        return -2;
    }

    // rows行/fields列 判断数据集合里
    int rows = mysql_num_rows(res);
    printf("row: %d\n", rows);
    int fields = mysql_num_fields(res);
    printf("fields: %d\n", fields);


    // fetch
    MYSQL_ROW row; //抓取数据，打印每行每列
    while(row= mysql_fetch_row(res)){
        for(int i = 0; i < fields; i ++){
            printf("%s\t", row[i]);
        }
        printf("\n");
    }
    
    mysql_free_result(res); //最后要释放，不释放会咋样
    return 0;

    
}


// 读图片并read
// filename : path + file name
// buffer : store image data
int read_image(char *filename, char *buffer){
    if(filename == NULL || buffer == NULL) return -1;

    FILE *fp = fopen(filename, "rb"); //二进制格式b
    if(fp ==NULL) {
        printf("fopen failed \n");
        return -2;
    }

    fseek(fp, 0, SEEK_END); //fopen文件指针fp是默认指向文件开始，现在用fseek放fp到最后
    int length = ftell(fp);  // filesize
    fseek(fp, 0, SEEK_SET);

    int size = fread(buffer, 1, length, fp);
    if(size != length) {
        printf("fread failed: %d \n", size);
        return -3;
    }

    fclose(fp);
    return size;
}

// 4写入磁盘
int write_image(char *filename, char *buffer, int length){
    if(filename == NULL || buffer == NULL || length <= 0) return -1;

    FILE *fp = fopen(filename, "wb+");//没有就创建
    if(fp ==NULL) {
        printf("fopen failed \n");
        return -2;
    }

    int size = fwrite(buffer,1, length, fp); //写到buffer里
    if(size != length) {
        printf("fwrite failed: %d\n",size);
        return -3;
    }
    fclose(fp);
    return size;
}


int mysql_write(MYSQL *handle, char *buffer, int length){
    if(handle == NULL || buffer == NULL || length <= 0) return -1;

    // handle类似管道
    // 创建statement储物间
    MYSQL_STMT *stmt = mysql_stmt_init(handle);

    // 0成功
    int ret = mysql_stmt_prepare(stmt, SQL_INSERT_IMG_USER , strlen(SQL_INSERT_IMG_USER));
    if(ret){
        printf("mysql_statement_prepare failed: %s\n", mysql_error(handle));
        return -2;
    }; //可以在参考手册chm里查用法)
    

    MYSQL_BIND param = {0}; //与stmt一一对应，把数据绑定到缓冲区
    param.buffer_type = MYSQL_TYPE_LONG_BLOB;
    param.buffer = NULL;
    param.is_null = 0;
    param.length = NULL;

    ret = mysql_stmt_bind_param(stmt, &param); 
    if(ret){
        printf("mysql_statement_bind_param: %s\n", mysql_error(handle));
        return -3;
    }   
    
    ret = mysql_stmt_send_long_data(stmt, 0, buffer, length);
    if(ret){
        printf("mysql_stmt_send_long_data: %s\n", mysql_error(handle));
        return -4;
    }

    ret = mysql_stmt_execute(stmt);
    if(ret){
        printf("mysql_stmt_execute: %s\n", mysql_error(handle));
        return -5;
    }

    ret = mysql_stmt_close(stmt); //有init就有close
    if(ret){
        printf(" mysql_stmt_close: %s\n", mysql_error(handle));
        return -6;
    }
    return ret;
}


int mysql_read(MYSQL *handle, char *buffer, int length){
    if(handle == NULL || buffer == NULL || length <= 0) return -1;

    MYSQL_STMT *stmt = mysql_stmt_init(handle);
    int ret = mysql_stmt_prepare(stmt, SQL_SELECT_IMG_USER , strlen(SQL_SELECT_IMG_USER));
    if(ret){
        printf("mysql_statement_prepare failed: %s\n", mysql_error(handle));
        return -2;
    }; 

    MYSQL_BIND result = {0}; //与stmt一一对应，把数据绑定到缓冲区
    result.buffer_type = MYSQL_TYPE_LONG_BLOB;
    unsigned long total_length = 0;
    result.length = &total_length;

    // 和write一样 下面不绑定参数，绑定结果
    ret = mysql_stmt_bind_result(stmt, &result);
    if(ret){
        printf("mysql_stmt_bind_result: %s\n", mysql_error(handle));
        return -3;
    }

    ret = mysql_stmt_execute(stmt);
    if(ret){
        printf("mysql_stmt_execute: %s\n", mysql_error(handle));
        return -4;
    }
    

    ret = mysql_stmt_store_result(stmt);
    if(ret){
        printf("mysql_stmt_store_result: %s\n", mysql_error(handle));
        return -5;
    }
    while(1){
        ret = mysql_stmt_fetch(stmt);
        if(ret !=0 && ret != MYSQL_DATA_TRUNCATED) break;
            
        int start = 0;
        while(start < (int) total_length){
            // 抓取图片长度
            result.buffer = buffer + start; //不断更新buffer自己大小，不要再开辟新的空间了
            result.buffer_length = 1; //类似每次取一个byte
            mysql_stmt_fetch_column(stmt, &result, 0 , start); //第0列，从偏移量start开始
            start += result.buffer_length;
        }
    }

    mysql_stmt_close(stmt);
    return total_length;

}

// C R U D增删改查 create update read delete
int main(){
    // 1 初始化
    MYSQL mysql;
    if(mysql_init(&mysql) == NULL){
        printf("mysql_init: %s\n", mysql_error(&mysql));
        return -1;
    };

    // 2 node server和db server连接
    if (!mysql_real_connect(&mysql, KING_DB_SERVER_IP, KING_DB_USERNAME, KING_DB_PASSWORD,
        KING_DB_DEFAULTDB, KING_DB_SERVER_PORT, NULL, 0)){
        
        printf("mysql_real_connect: %s\n", mysql_error(&mysql));
        goto Exit;
    };

    // mysql->insert 0成功 
    // 编译器编译忽略，执行才直接
    printf("case : mysql--->insert\n");
#if 1
    if(mysql_real_query(&mysql, SQL_INSERT_TBL_USER, strlen(SQL_INSERT_TBL_USER))){
        printf("mysql_real_query: %s\n", mysql_error(&mysql));
        goto Exit;
    };
#endif
    king_mysql_select(&mysql); //添加以后查询

    // mysql->delete
    printf("case : mysql--->delete\n");
#if 1
    if(mysql_real_query(&mysql, SQL_DELETE_TBL_USER, strlen(SQL_DELETE_TBL_USER))){
        printf("mysql_real_query: %s\n", mysql_error(&mysql));
        goto Exit;
    };
#endif
    king_mysql_select(&mysql); //删除以后查询


    printf("case: mysql-->read image and write mysql\n");
    char buffer[FILE_IMAGE_LENGTH] = {0};
    int length = read_image("0voice.jpg", buffer);
    if(length<0) goto Exit;

    mysql_write(&mysql, buffer, length);


    printf("case: mysql-->read mysql and write image\n");
    memset(buffer,0, FILE_IMAGE_LENGTH); //让初始buffer干净没有数据
    length = mysql_read(&mysql, buffer, FILE_IMAGE_LENGTH);
    
    write_image("a.jpg", buffer, length);


Exit:
    mysql_close(&mysql);


    return 0;
}
