#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#include "../serverHeaders/hb_tree.h"

#include <unistd.h>


#define __STDC_FORMAT_MACROS
#include <inttypes.h>

//#define a 20

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

//#define SWAP(a,b,temp) do {temp = (a); (a) = (b); (b) = temp;} while(0)



typedef struct sample_s {
    uint64_t id;
    char name[21];
    int data;
} sample_t;


typedef struct message_header_s{
uint64_t                magic;
uint16_t                len;
uint16_t                message_type;
}message_header_t;

typedef struct user_s{
    uint16_t           len;
    char              *user;
    char              *password;
}user_t;

void serialize(sample_t *s, uint8_t *buff, uint32_t *buff_used){
    uint32_t offset =0;
    *((uint64_t *)(buff)) = htonll(s->id);
    offset += sizeof(s->id);

    memcpy(buff+offset,s->name,sizeof(s->name));
    offset += sizeof(s->name);

    *((int *)(buff+offset)) = htonl(s->data);
    offset+= sizeof(s->data);

    *buff_used = offset;

    return;

} 



sample_t *deserialize(uint8_t *buff, uint32_t *buff_used){
    sample_t *d = (sample_t *)malloc(sizeof(sample_t));
    uint32_t offset =0;

    d->id = ntohll(*((uint64_t *)(buff+offset)));
    offset += sizeof(d->id);

    memcpy(d->name,(buff+offset),sizeof(d->name));
    offset+= sizeof(d->name);

    d->data = ntohl(*((int *)(buff+offset)));
    offset += sizeof(d->data);

    return d;

}

void ser(user_t *user,uint8_t *buff, uint32_t *buff_used){
    uint32_t offset = 0;

   *((uint16_t *)(buff+offset)) = htons(user->len);
    offset += sizeof(user->len);

    memcpy(buff+offset,user->user,sizeof(user->user));
    offset+=sizeof(user->user);

    memcpy(buff+offset,user->password,sizeof(user->password));
    offset+=sizeof(user->password);

    *buff_used = offset;

    return;

} 

user_t *deser(uint8_t *buff){
    user_t *user = (user_t *)malloc(sizeof(user_t));

    uint32_t offset = 0;

    user->len = ntohs(*((uint16_t *)(buff+offset)));
    offset+= sizeof(user->len);

    for(int i=0; i<user->len; i++){
        printf("%c ",*(buff+offset+i));
    }

    char *data = (char *)malloc(user->len);
    memcpy(data,buff+offset,user->len);
    offset+=user->len;
    
    printf("%c\n",data[8]); 
    

    //user->user = strtok(data,'\0');
    //user->password = strtok(data,'\0');

    return user;
}


//void
//swap_int(int *a, int *b) 
//{
//    int temp;
//    temp = *a;
//    *a = *b;
//    *b = temp;
//    return;
//}

void 
inorder(hb_node_t* root)
{
    if (root == NULL) {
        return;
    }

    inorder(root->llink);
    printf("user : %s with fd : %d\n",(char*)root->key,root->fd);
    inorder(root->rlink);
    
    return;
}

int main(){
   //sample_t *s = (sample_t *)malloc(sizeof(sample_t));
   //s->data = 200;

   //void *ptr =  s;

   //printf("Sample : %p\n", s);
   //printf("Ptr : %p\n",((sample_t *)ptr));

   //printf("Data %d", ((sample_t *)ptr)->data);
   
    //uint8_t *c = NULL;
    //char b = 'a';

    //c = &b;


    //printf("%c\n",(*c));
    //printf("%ld\n",sizeof(&b));
    //printf("%ld\n",sizeof(b));   
    

   // char buff[1024] = "hello world";


   // uint8_t *ptr = (uint8_t *)malloc(11);
   // ptr = buff;


   // for(int i=0; i<11; i++){
   //     printf("%c",(*ptr));
   //     ptr++;
   // }
   // 

   // printf("\n%u\n",*ptr);
   // printf("%hhu\n",*ptr);

   // uint64_t magic = 0xABCDDCBA;
   // printf("magic : %lu\n",magic);

   // uint64_t network = htonl(magic);
   // printf("network : %lu\n",network);

   // uint64_t host = ntohl(network);
   // printf("host : %lu\n",host);

   // printf("%lu\n",(magic & 0xFFFFFFFF)<<32);

   // printf("%lu\n",(magic >> 32));

   // printf("%lu\n",((uint64_t)htonl((magic) & 0xFFFFFFFF) << 32));

   // printf("%lu\n",(uint64_t)htonl((magic) >> 32));

   // printf("%lu\n",((uint64_t)htonl((magic) & 0xFFFFFFFF) << 32) | htonl((magic) >> 32));

   // printf("%lu\n",((uint64_t)htonl((magic) & 0xFFFFFFFF) << 32) | htonl((magic) >> 32));

    //char buff[1024];

    //bzero(buff,0);

    //int ret;

//    ret = read(0,buff,sizeof(buff));
//
//    printf("%d----------------------ret\n",ret);
//    printf("%s s------------------------ret\n",buff);

   // while((ret = read(0,buff,sizeof(buff))) != EOF );

   // printf("%s\n",buff);
 
//      printf("%lu\n",sizeof(sample_t)); 
//      uint8_t *buff = (uint8_t *)malloc(sizeof(sample_t)*sizeof(uint8_t));
//      uint32_t buff_used=0;
//      sample_t *s = (sample_t *)malloc(sizeof(sample_t));
//      s->id = 1900182;
//     // s->name = "shantanu";
//      memcpy(s->name,"shantanu",8);
//      s->data = 22;
//
//      printf("%ld\n",sizeof(*s)); 
//      serialize(s,buff,&buff_used);
//      printf("%d\n",buff_used);
//
//      printf("%c\n",*(buff+8));
//
//      //printf("%"PRIu64"\n", *buff);
//
//      printf("%u\n",*buff);
//
//      sample_t *s_d = deserialize(buff,&buff_used);
//
//      printf("%lu\n",s_d->id);
//      printf("%s\n",s_d->name);
//      printf("%d\n",s_d->data);
//
//      printf("%lu\n",sizeof(message_header_t));


   // user_t *user = (user_t *)malloc(sizeof(user_t));

   // scanf("%ms",&user->user);
   // scanf("%ms",&user->password);

   // printf("%ld\n",strlen(user->user));
   // user->user[strlen(user->user)] = '\0';
   // printf("%ld\n",sizeof(user->user));
   // printf("%s\n",user->user);

   // user->len = strlen(user->user)+strlen(user->password)+2;
 
   // uint8_t *buff = (uint8_t *)malloc(sizeof(user_t));
   // uint32_t buff_used;

   // ser(user,buff,&buff_used);

   // user_t *ud = deser(buff);

   // printf("%u\n",ud->len);
   // printf("user : %s\n",ud->user);
   // printf("password : %s\n",ud->password);

    //hb_tree_t* tree = hb_tree_new();

    //hb_tree_insert(tree, "duser", 7);
    //hb_tree_insert(tree, "cuser", 8);
    //hb_tree_insert(tree, "buser", 19);
    //hb_tree_insert(tree, "auser", 21);
    //hb_tree_insert(tree, "eukser",14);
    //hb_tree_insert(tree, "quser", 13);
    //hb_tree_insert(tree, "fuser", 18);
    //hb_tree_insert(tree, "ruser", 43);
    //hb_tree_insert(tree, "guser", 67);
    //hb_tree_insert(tree, "zuser", 45);
    //hb_tree_insert(tree, "huser", 76);
    //hb_tree_insert(tree, "wuser", 89);
    //hb_tree_insert(tree, "iuser", 19);
    //hb_tree_insert(tree, "juser", 99);
    //hb_tree_insert(tree, "suser", 32);
    //hb_tree_insert(tree, "kuser", 67);
    //hb_tree_insert(tree, "tuser", 69);
    //hb_tree_insert(tree, "luser", 72);
    //hb_tree_insert(tree, "muser", 56);
    //hb_tree_insert(tree, "nuser", 27);
    //hb_tree_insert(tree, "yuser", 3);

    //inorder(tree->root);

    //hb_tree_remove(tree,"buser");
    //hb_tree_remove(tree,"luser");
    //hb_tree_remove(tree,"fuser");
    //printf("\n\n");

    //inorder(tree->root);
    //printf("\n\n");

    //printf("search user fd : %d\n",hb_tree_search(tree,"zuser"));

  // char* line;
  // size_t size = 0;
  // uint16_t  len =0,len2 = 0;;
  // char* user;
  //  //scanf("%ms", &user);
  //  len = getline(&user,&size,stdin);
  //  len =0;
  //  size = 0;

  //  printf("user : %s\n",user);
  //  //line[strlen(line)] = '\0';

  //  len = getline(&line,&size,stdin);

  //  printf("len : %d\n", len);
  //  printf("size : %zu\n", size);
  //  line[len-1] = '\0';

  //  printf("string  : %s\n", line);

  //  printf("size of %zu\n",sizeof(line));

  //  printf("\n");

  //   void* ptr ;
  //   int a = 6;
  //   ptr = &a;

  //   int* b = ptr;

  //   printf("%d\n",*b);


    char* str1 = (char*)malloc(20);
    strcpy(str1,"shantanu");
    char* str2 = (char*)malloc(20);
    strcpy(str2,str1);

    for (int i=0; i<strlen(str1); i++) {
        printf("%c",*(str1+i));
    }

    printf("\n");

    free(str1);
    free(str2);


    return 0;
}

