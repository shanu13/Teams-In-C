#ifndef _HB_TREE_H_
#define _HB_TREE_H_

#include <stdio.h>
#include <stdbool.h>


#define MIN(a,b) ((a < b) ? (a) : (b))
#define MAX(a,b) ((a > b) ? (a) : (b))
#define SWAP(a,b,temp) do {temp = (a); (a) = (b); (b) = (temp);} while(0)


//typedef int  (*compare_func) (const void*, const void*);
//typedef void (*delete_func)  (void*, void*);


typedef struct hb_node_s {
    void                       *key;
    int                         fd;
    struct hb_node_s           *parent;
    struct hb_node_s           *llink;
    struct hb_node_s           *rlink;
    signed char                 bal;
} hb_node_t;

typedef struct hb_tree_s {
    hb_node_t           *root; 
    size_t               count;
    size_t               rotation_count;
} hb_tree_t;

void            swap_int(int *a, int *b);

int             str_cmp_func(const void* k1, const void* k2);

void            del_func(hb_node_t* node);

bool            rotate_right(hb_tree_t* tree, hb_node_t* node);

bool            rotate_left(hb_tree_t* tree, hb_node_t* node);

hb_node_t*      new_node(const void* user, int fd);

hb_tree_t*      hb_tree_new();

void*           hb_tree_insert(hb_tree_t* tree, void* key, int fd);

bool            hb_tree_remove(hb_tree_t* tree, const void* key);

int             hb_tree_search(hb_tree_t *Tree, const void* key);


#endif





 


