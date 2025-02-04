#include "redis.h"
extern Node root;

int32 handle_set(Client *, int8 *, int8 *);
int32 handle_get(Client *, int8 *, int8 *);

bool scontinuation;
bool ccontinuation;

CmdHandler handlers[] = {
    {(int8 *)"SET", handle_set},
    {(int8 *)"GET", handle_get}};

Callback getcmd(int8 *cmd)
{
    Callback cb;
    int16 n, arrlen;

    if (sizeof(handlers) < 16)
    {
        return 0;
    }

    arrlen = (sizeof(handlers)) / 16;

    cb = 0;
    for (n = 0; n < arrlen; n++)
    {
        if (!strcmp((char *)cmd, (char *)handlers[n].cmd))
        {
            cb = handlers[n].handler;
        }
    }

    return cb;
}

int32 handle_set(Client *cli, int8 *full_path, int8 *value_str)
{
    int8 parent_path[256], key[128], value[128];
    Node *parent_node;
    Leaf *new;

    // 1. Split full_path into parent path and key
    strncpy((char *)parent_path, (char *)full_path, sizeof(parent_path) - 1);
    parent_path[sizeof(parent_path) - 1] = '\0'; // Ensure null termination

    // Find last '/' to separate parent path and key
    char *last_slash = strrchr((char *)parent_path, '/');
    if (!last_slash || last_slash == (char *)parent_path)
    { // Handle root or invalid paths
        dprintf(cli->s, "Error: Invalid path format\n");
        return -1;
    }

    // Extract key (portion after last '/')
    *last_slash = '\0'; // Terminate parent path at last slash
    strncpy((char *)key, last_slash + 1, sizeof(key) - 1);
    key[sizeof(key) - 1] = '\0';

    // 2. Parse value (handle quotes)
    strncpy((char *)value, (char *)value_str, sizeof(value) - 1);
    value[sizeof(value) - 1] = '\0';

    // Remove surrounding quotes if present
    size_t val_len = strlen((char *)value);
    if (val_len >= 2 && value[0] == '"' && value[val_len - 1] == '"')
    {
        memmove(value, value + 1, val_len - 2); // Remove quotes
        value[val_len - 2] = '\0';
    }

    // 3. Find/Create parent node
    parent_node = find_node_linear((int8 *)parent_path);
    if (!parent_node)
    {
        parent_node = create_node(&root, (int8 *)parent_path);
        if (!parent_node)
        {
            dprintf(cli->s, "Error: Failed to create parent directory\n");
            return -1;
        }
    }

    // 4. Create leaf (key-value pair)
    new = create_leaf(parent_node, key, value, (int16)strlen((char *)value));
    if (!new)
    {
        dprintf(cli->s, "Error: Failed to create key-value pair\n");
        return -1;
    }

    dprintf(cli->s, "OK\n");
    print_tree(cli->s, (Tree *)&root); // Show updated tree
    return 0;
}

int32 handle_get(Client *cli, int8 *path, int8 *key)
{
    int8 *value = lookup_linear(path, key);
    printf("Value: %s\n", value);
    if (value)
    {
        dprintf(cli->s, "%s->", key);
        dprintf(cli->s, "%s\n", value);
    }
    else
    {
        dprintf(cli->s, "Error: Key not found\n");
    }
    return 0;
}

void zero(int8 *buf, int16 size)
{
    int8 *p;
    int16 n;

    for (n = 0, p = buf; n < size; n++, p++)
        *p = 0;

    return;
}

void childloop(Client *cli)
{
    int8 buf[256];
    int16 n;
    int8 *p, *f;
    int8 cmd[256], folder[256], args[256];
    Callback cb;

    zero(buf, 256);
    read(cli->s, (char *)buf, 255);
    n = (int16)strlen((char *)buf);
    if (n > 254)
    {
        n = 254;
    }

    for (p = buf; (*p) && (n--) && (*p != ' ') && (*p != '\n') && (*p != '\r'); p++)
        ;

    zero(cmd, 256);
    zero(folder, 256);
    zero(args, 256);

    // for command
    if (!(*p) || (!n))
    {
        strncpy((char *)cmd, (char *)buf, 255);
        goto done;
    }
    else if ((*p == '\n') || (*p == '\r'))
    {
        *p = 0;
        snprintf((char *)cmd, 256, "%s", (char *)buf);
        goto done;
    }
    else if (
        (*p == ' '))
    {
        *p = 0;
        snprintf((char *)cmd, 256, "%s", (char *)buf);
    }

    for (p++, f = p; (*p) && (n--) && (*p != ' ') && (*p != '\n') && (*p != '\r'); p++)
        ;

    // for folder name
    if (!(*p) || (!n))
    {
        strncpy((char *)folder, (char *)f, 255);
        goto done;
    }
    else if ((*p == ' ') || (*p == '\n') || (*p == '\r'))
    {
        *p = 0;
        strncpy((char *)folder, (char *)f, 255);
    }

    p++;

    if (*p)
    {
        strncpy((char *)args, (char *)p, 255);
        for (p = args; (*p) && (*p != '\n' && (*p != '\r')); p++)
            ;

        *p = 0;
    }
done:
    dprintf(cli->s, "cmd:\t%s\n", cmd);
    dprintf(cli->s, "folder:\t%s\n", folder);
    dprintf(cli->s, "args:\t%s\n", args);
    cb = getcmd(cmd);
    if (!cb)
    {
        dprintf(cli->s, "!400 Command not found: %s\n", cmd);
        return;
    }
    else
    {
        cb(cli, folder, args);
        return;
    }

    return;
}

void mainloop(int s)
{
    struct sockaddr_in cli;
    int32 len;
    int s2;
    char *ip;
    int16 port;
    Client *client;
    pid_t pid;

    s2 = accept(s, (struct sockaddr *)&cli, (unsigned int *)&len);
    if (s2 < 0)
    {
        return;
    }

    port = (int16)htons((int)cli.sin_port);
    ip = inet_ntoa(cli.sin_addr);

    printf("Connection from %s:%d\n", ip, port);

    client = (Client *)malloc(sizeof(struct s_client));
    assert(client);

    zero((int8 *)client, sizeof(struct s_client));
    client->s = s2;
    client->port = port;
    strncpy(client->ip, ip, 15);

    pid = fork();
    if (pid)
    {
        free(client);

        return;
    }
    else
    {
        dprintf(s2, "100 Connected to redis server\n");
        ccontinuation = true;
        while (ccontinuation)
            childloop(client);

        close(s2);
        free(client);

        return;
    }

    return;
}

int initserver(int16 port)
{
    struct sockaddr_in sock;
    int s;

    sock.sin_family = AF_INET;
    sock.sin_port = htons(port);
    sock.sin_addr.s_addr = inet_addr(HOST);

    s = socket(AF_INET, SOCK_STREAM, 0);
    assert(s > 0);

    errno = 0;
    if (bind(s, (struct sockaddr *)&sock, sizeof(sock)))
    {
        assert_perror(errno);
    }

    errno = 0;
    if (listen(s, 20))
    {
        assert_perror(errno);
    }

    printf("Server listening on %s:%d\n", HOST, port);
    fflush(stdout);
    return s;
}

int main(int argc, char *argv[])
{
    char *sport;
    int16 port;
    int s;

    // int8 *p;
    // int16 sz;
    // p = (int8 *)"true";
    // sz = (int16)strlen((char *)p);

    // n = create_node(&root, (int8 *)"/Users/");
    // printf("n\t%p\n", n);

    // n2 = create_node(n, (int8 *)"/Users/job");
    // printf("n2\t%p\n", n2);

    // l = create_leaf(n, (int8 *)"loggedin", p, sz);
    // printf("l\t%p\n", l);

    // free(n2);
    // free(n);
    // exit(0);

    if (argc < 2)
    {
        sport = PORT;
    }
    else
    {
        sport = argv[1];
    }

    port = (int16)atoi(sport);

    s = initserver(port);

    scontinuation = true;
    while (scontinuation)
    {
        mainloop(s);
    }

    printf("shutting down ...\n");
    close(s);
    return 0;
}