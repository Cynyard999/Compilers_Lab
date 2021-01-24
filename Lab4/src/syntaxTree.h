#ifndef _SYNTAX_TREE
#define _SYNTAX_TREE
typedef struct TreeNode
{
    int lineNo;
    int type;
    char *name;
    struct TreeNode* nextSibling;
    struct TreeNode* firstChild;
    union Value{
        int i;
        //char *f;
        double f;
        char *s;
    }val;
}Node;

Node *root;
void print_tree();
Node *create_node(int lineNumber, char *name, int type, void *val);
void add_child(Node *parent, Node *child);



#endif