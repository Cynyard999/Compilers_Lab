#ifndef _COMMON
#define _COMMON
typedef struct TreeNode
{
    int lineNo;
    int type;
    char *name;
    struct TreeNode* nextSibling;
    struct TreeNode* firstChild;
    union Value{
        int i;
        // char *f;
        // 直接用float 会在atof转换的时候失精（单精度到多精度） 本项目给出了两种解决方法 另一种方法被注释掉
        double f;
        char *s;
    }val;
}Node;

Node *root;
void print_tree();
Node *create_node(int lineNumber, char *name, int type, void *val);
void add_child(Node *parent, Node *child);


#endif