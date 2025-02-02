#include "tree.h"

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

void zero(int8 *str, int16 size)
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
    Node *n;
    int16 size;

    errno = NoError;
    assert(parent);
    size = sizeof(struct s_node);
    n = (Node *)malloc((int)size);
    zero((int8 *)n, size);

    parent->west = n;
    n->tag = TagNode;
    n->north = parent;
    strncpy((char *)n->path, (char *)path, 255);

    return n;
}

Node *find_name_linear(int8 *path)
{
    Node *p, *ret;

    for (ret = (Node *)0, p = (Node *)&root; p; p = p->west)
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
    Leaf *l, *ret;

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

    p = find_leaf_linear(path, key);
    if (p)
    {
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
    for (c = 'a'; c <= path; c++)
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
    n = (int16)strnlen((char *)buf);
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

    FILE *fd
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
        path = example_path(*buf);
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
    x = 0;
    for (n = (Node *)&root, c = 'a'; c <= 'z'; c++)
    {
        x = (int32)strlen((char *)path);
        *(path + x++) = '/';
        *(path + x) = c;
        printf("%s\n", path);

        p = n;
        n = create_node(p, path);
    }

    return (Tree *)&root;
}

int main()
{
    Tree *example;
    int32 x;

    example = example_tree();

    x = example_leaves();
    (void)x;

    print_tree(1, example);

    return 0;
}
