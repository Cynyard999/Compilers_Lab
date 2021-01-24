#include "syntax.tab.h"
#include "syntaxTree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Node* create_node(int lineNumber, char *name, int type, void *val){
    Node* node = malloc(sizeof(Node));
    node->lineNo = lineNumber;
    //node->name = strdup(name);
    node->name = malloc(strlen(name) + 1);
    strcpy(node->name, name);
    node->type = type;
    if (type == INT)
    {
        node->val.i = *((int *)val);
    }
    else if (type == FLOAT)
    {
        // node->val.f = malloc(strlen((char *)val) + 1);
        // strcpy(node->val.f, (char *)val);
        node->val.f = *((double *)val);
    }
    else if (type == TYPE || type == ID || type == RELOP)
    {
        //node->val.s = strdup((char *) val);
        node->val.s = malloc(strlen((char *)val) + 1);
        strcpy(node->val.s, (char *)val);
    }
    else
    {
        node->val.i = -1;
    }
    return node;
}

void add_sibling(Node *child, Node *newChild){
    if (child->nextSibling!=NULL)
    {
        add_sibling(child->nextSibling, newChild);
    }
    else
    {
        child->nextSibling = newChild;
    }
}

void add_child(Node *parent, Node *child){
    if (parent->firstChild == NULL)
    {
        parent->firstChild = child;
    }
    else
    {
        add_sibling(parent->firstChild,child);
    }
}

void print_sub_tree(Node* root, int level){
    for (int i = 0; i < level; i++)
    {
        fprintf(stderr,"  ");
    }
    if (root->type == FLOAT)
    {
        fprintf(stderr,"%s: %f\n",root->name,root->val.f);
        //fprintf(stderr,"%s: %s\n",root->name,root->val.f);
    }
    else if(root->type == INT){
        fprintf(stderr,"%s: %u\n", root->name, root->val.i);
    }
    else if(root->type == ID || root->type == TYPE){
        fprintf(stderr,"%s: %s\n", root->name, root->val.s);
    }
    else if (root->type == 0)
    {
        fprintf(stderr,"%s (%d)\n",root->name,root->lineNo);
    }
    else
    {
        fprintf(stderr,"%s\n",root->name);
    }
    if (root->firstChild!=NULL)
    {
        Node* child = root->firstChild;
        while (child != NULL)
        {
            print_sub_tree(child, level+1);
            child = child->nextSibling;
        }
    }
}

void print_tree(){
    print_sub_tree(root,0);
}
