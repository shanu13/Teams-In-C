
#include <stdlib.h>
#include <string.h>

#include "../serverHeaders/hb_tree.h"

void
swap_int(int *a, int *b) 
{
    int temp;
    temp = *a;
    *a = *b;
    *b = temp;
    return;
}


int
str_cmp_func(const void* k1, const void* k2)
{
    const char* a = k1;
    const char* b = k2;

    for (; ;) {
        char p = *a++, q= *b++;
        if (!p || p!=q) {
            return (p>q) - (p<q);
        }
    }

}

void 
rotate_right(hb_tree_t *tree, hb_node_t *node)
{
    hb_node_t *nl = node->llink;
    if ((node->llink = nl->rlink) != NULL) {
        nl->llink->parent = node;
    }
    nl->rlink = node;

    hb_node_t *p = node->parent;
    node->parent = nl;
    
    if ((nl->parent = p) != NULL) {
        if (p->llink == node) {
            p->llink = nl;
        }else {
            p->rlink = nl;
        }    
    }else {
        tree->root = nl;
    }

    node->bal += 1 - MIN(nl->bal, 0);
    nl->bal   += 1 + MAX(node->bal, 0);

    return;
}

void
rotate_left(hb_tree_t *tree, hb_node_t *node)
{
    hb_node_t *nr = node->rlink;
    if ((node->rlink = nr->llink) != NULL) {
        node->rlink->parent = node;
    }
    nr->llink = node;

    hb_node_t *p = node->parent;
    node->parent = nr;

    if ((nr->parent = p) != NULL) {
        if (p->llink == node) {
            p->llink = nr;
        }else {
            p->rlink = nr;
        }
    }else{
        tree->root = nr;
    }
    
    node->bal -= 1 + MAX(nr->bal, 0);
    nr->bal   -= 1 - MIN(node->bal, 0);

}

hb_node_t*
new_node(const void  *user, int fd)
{
    hb_node_t *node = (hb_node_t *)malloc(sizeof(hb_node_t));
    node->key = (char*)calloc(sizeof(user),sizeof(char*));
    strcpy(node->key,user);
    node->fd = fd;
    node->parent = NULL;
    node->llink = NULL;
    node->rlink = NULL;

    return node;
}

hb_tree_t*
hb_tree_new()
{
    hb_tree_t *tree = (hb_tree_t*)malloc(sizeof(hb_tree_t));
    tree->root = NULL;
    tree->count = 0;
    tree->rotation_count = 0;

    return tree;
}

void*
hb_tree_insert(hb_tree_t* tree, void* key, int fd)
{
    hb_node_t *parent = NULL;
    hb_node_t *node = tree->root;
    hb_node_t *q = NULL;
    int cmp = 0;

    while(node){
      cmp = str_cmp_func(key, node->key);
      if (cmp < 0) {
          parent = node;
          node = node->llink;
      }else {
          parent = node;
          node = node->rlink;
      }

      if (parent->bal) {
          q = parent;
      }
    }

    hb_node_t node =new_node(key,fd);

    if (!(node->parent = parent)) {
        tree->root = node;
    }else {
        if (cmp < 0 ) {
            parent->llink = node;
        }else{
            parent->rlink = node;
        }
    }

    while (parent != q) {
        parent->bal = (node == parent->rlink)*2 -1;
        node = parent;
        parent = node->parent;
    }

    unsigned rotations = 0;

    if (q) {
        if (q->llink == node) {
            if (--q->bal == -2) {
                if (q->llink->bal > 0) {
                    rotate_left(tree, q->llink);
                    rotations++;

                }
                
                rotate_right(tree, q);
                rotations++;
            }
        }else{
            if (++q->bal == 2) {    
                if (q->rlink->bal < 0) {
                    rotate_right(tree, q->rlink);
                    rotations++;
                }

                rotate_left(tree, q);
                rotations++;
            }
        }
    }

    tree->rotation_count += rotations;
    
    return node;
 }


bool 
hb_tree_remove(hb_tree_t* tree, const void* key)
{
    hb_node_t* node = tree->root;
    hb_node_t* parent = NULL;

    while (node) {
        int cmp = str_cmp_func(key,node->key);
        if (cmp < 0) {
            parent = node;
            node = node->llink;
        } else {
            parent = node;
            node = node->rlink;
        } else {
            break;
        }
    }

    if (!node) {
       return false;
    } 

    if (node->llink && node->rlink) {
        hb_node_t* out = NULL;
        if (node->bal > 0) {
            out = node->rlink;
            while (out->llink) {
                out = out->llink;
            }
        } else {
            out = node->llink;
            while (out->rlink) {
                out = out->rlink;
            }
        }

        void *temp;
        SWAP(node->key,out->key,temp);
        swap_int(&node->fd,&out->fd);
        node = out;
        parent = out->parent;
    }




}
