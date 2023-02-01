#ifndef _HB_TREE_H_
#define _HB_TREE_H_

#include <stdio.h>

#define MIN(a,b) ((a < b) ? (a) : (b))
#define MAX(a,b) ((a > b) ? (a) : (b))
#define SWAP(a,b,temp) do {temp = (a); (a) = (b); (b) = (v)} while(0)


typedef int  (*compare_func) (const void*, const void*);
typedef void (*delete_func)  (void*, void*);


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

hb_node_t*      node_new(const void *user, int fd);

hb_tree_t*      hb_tree_new();




#endif








