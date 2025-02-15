#include "tree.h"
Nullptr null_ptr = 0;
Tree root = {.n = {
                 .tag = (TagRoot | TagNode),
                 .north = (Node *)&root,
                 .west = 0,
                 .east = 0,
                 .path = "/"}};

void print_tree(int fd, Tree *_root)
{
    int8 indentation;
    int8 buf[256];
    int16 size;
    Node *n;
    Leaf *l, *last;

    indentation = 0;
    for (n = (Node *)_root; n; n = n->west)
    {
        Print(indent(indentation++));
        Print(n->path);
        Print("\n");
        if (n->east)
        {
            last = find_last(n);
            if (last)
                for (l = last; (Node *)l->west != n; l = (Leaf *)l->west)
                {
                    Print(indent(indentation));
                    Print(n->path);
                    Print("/");
                    Print(l->key);
                    Print(" -> '");
                    write(fd, (char *)l->value, (int)l->size);
                    Print("'\n");
                }
        }
    }
    return;
}

int8 *indent(int16 n)
{
    int16 i;
    static int8 buf[256];
    int8 *p;

    if (n < 1)
        return (int8 *)"";
    assert((n < 120));
    zero(buf, 256);

    for (i = 0, p = buf; i < n; i++, p += 2)
        strncpy((char *)p, "  ", 2);

    return buf;
}

static void zero(int8 *str, int16 size)
{
    int8 *p;
    int16 n;

    for (n = 0, p = str; n < size; p++, n++)
    {
        *p = 0;
    }

    return;
}

Node *create_node(Node *parent, int8 *path)
{
    if (!parent)
    {
        char parent_path[256];
        strncpy((char *)parent_path, (char *)path, 255);
        char *last_slash = strrchr(parent_path, '/');
        if (last_slash)
        {
            *last_slash = '\0';
            parent = find_node_linear((int8 *)parent_path);
            if (!parent)
            {
                parent = create_node((Node *)&root, (int8 *)parent_path);
            }
        }
    }

    Node *n = (Node *)malloc(sizeof(Node));
    zero((int8 *)n, sizeof(Node));

    parent->west = n;
    n->tag = TagNode;
    n->north = parent;
    strncpy((char *)n->path, (char *)path, 255);

    return n;
}

Node *find_node_linear(int8 *path)
{
    Node *p, *ret = NULL;

    for (p = (Node *)&root; p; p = p->west)
    {
        if (!strcmp((char *)p->path, (char *)path))
        {
            ret = p;
            break;
        }
    }
    return ret;
}

Leaf *find_leaf_linear(int8 *path, int8 *key)
{
    Node *n;
    Leaf *l, *ret = NULL;

    n = find_node(path);
    if (!n)
        return (Leaf *)0;

    for (ret = (Leaf *)0, l = n->east; l; l = l->east)
    {
        if (!strcmp((char *)l->key, (char *)key))
        {
            ret = l;
            break;
        }
    }
    return ret;
}

int8 *lookup_linear(int8 *path, int8 *key)
{
    Leaf *p;
    printf("lOOKING Linear\n");

    p = find_leaf_linear(path, key);
    if (p)
    {
        printf("FInd leaf linear:%p\n", p);
        return p->value;
    }
    return (int8 *)0;
}

Leaf *find_last_linear(Node *parent)
{
    Leaf *l;

    errno = NoError;
    assert(parent);

    if (!parent->east)
    {
        return (Leaf *)0;
    }
    for (l = parent->east; l->east; l = l->east)
        ;
    assert(l);
    return l;
}

Leaf *create_leaf(Node *parent, int8 *key, int8 *value, int16 count)
{
    Leaf *l, *new;
    int16 size;

    assert(parent);
    l = find_last(parent);

    size = sizeof(struct s_leaf);
    new = (Leaf *)malloc(size);
    assert(new);

    if (!l)
    {
        // we are directly connected to the parent
        parent->east = new;
    }
    else
    {
        // we are at the last leaf
        l->east = new;
    }

    zero((int8 *)new, size);
    new->tag = TagLeaf;
    new->west = (!l) ? (Tree *)parent : (Tree *)l;

    strncpy((char *)new->key, (char *)key, 127);
    new->value = (int8 *)malloc(count);
    zero(new->value, count);
    assert(new->value);
    strncpy((char *)new->value, (char *)value, count);
    new->size = count;

    return new;
}

int8 *example_path(int8 *path)
{
    int32 x;
    static int8 buf[256];
    int8 c;
    zero(buf, 256);
    for (c = 'a'; c <= *path; c++)
    {
        x = (int32)strlen((char *)buf);
        *(buf + x++) = '/';
        *(buf + x++) = c;
    }

    return buf;
}

int8 *example_duplicate(int8 *str)
{
    int16 n, x;
    static int8 buf[256];

    zero(buf, 256);
    strncpy((char *)buf, (char *)str, 255);
    n = (int16)strnlen((char *)buf, 255);
    x = (n * 2);
    if (x > 254)
    {
        return buf;
    }
    else
    {
        strncpy((char *)buf + n, strdup((char *)buf), 255);
    }
    return buf;
}

int32 example_leaves()
{

    FILE *fd;
    int32 x,
        y;
    int8 *path, *val;
    int8 buf[256];
    Node *n;

    fd = fopen(ExampleFile, "r");
    assert(fd);

    zero(buf, 256);
    y = 0;
    while (fgets((char *)buf, 255, fd))
    {
        x = (int32)strlen((char *)buf);
        *(buf + x - 1) = 0;
        path = example_path(buf);
        n = find_node(path);
        if (!n)
        {
            zero(buf, 256);
            continue;
        }

        val = example_duplicate(buf);
        create_leaf(n, buf, val, (int16)strlen((char *)val));
        y++;
        zero(buf, 256);
    }

    fclose(fd);

    return y;
}

Tree *example_tree()
{
    int8 c;
    Node *n, *p;
    int8 path[256];
    int32 x;

    zero(path, 256);

    n = (Node *)&root; // Ensure root is initialized

    for (c = 'a'; c <= 'z'; c++)
    {
        x = (int32)strlen((char *)path);

        if (x + 2 >= 256) // Prevent buffer overflow
            break;

        snprintf((char *)path + x, 256 - x, "/%c", c);
        printf("%s\n", path);

        p = n;
        n = create_node(p, path);

        if (!n) // Ensure node creation succeeded
        {
            fprintf(stderr, "Error: Failed to create node for %s\n", path);
            return NULL;
        }

        p->west = n; // Properly link the new node
    }

    return (Tree *)&root;
}

// int main()
// {
//     Tree *example;
//     int32 x;

//     example = example_tree();

//     x = example_leaves();
//     (void)x;

//     print_tree(1, (Tree *)&root);

//     int8 *value = lookup_linear("/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/u/v/w/x/y/z", "z-axis");
//     if (value)
//     {
//         printf("%s\n", value);
//     }
//     return 0;
// }

// #ifndef TREE_MAIN
// #define TREE_MAIN // Define TREE_MAIN only when tree.c runs separately

// int main()
// {
//     printf("Tree system initialized.\n");
//     return 0;
// }

// #endif