# In-Memory Database (Tree-Based Key-Value Store)

## 📌 Overview
This is a simple **tree-based in-memory database** that organizes data hierarchically, similar to a filesystem.  
- **Nodes (directories)** represent hierarchical paths (`/a`, `/a/b`).  
- **Leaves (key-value pairs)** store actual data (`/a/b/c = "MOHIT"`).  
- **Dynamic node creation** ensures missing parent directories are automatically created.  
- **Supports TCP communication** for client-server interaction.

---

## 🏗️ **How It Works**
The database is structured as a **tree**, where:
- **Root (`/`)** is the starting point.
- **Each directory (`/a`, `/a/b`) is a node.**
- **Each key-value pair (`/a/b/c = "MOHIT"`) is a leaf.**

### 🔹 **Example Structure**
```
(root)
  ├── /a
       ├── /a/b
            ├── c -> "MOHIT"
```

---

## ⚙️ **Core Components**
| Component         | Description |
|------------------|-------------|
| `tree.c`        | Implements the in-memory database using a tree structure. |
| `redis.c`       | Handles TCP communication and processes client commands. |
| `Client`        | Sends commands like `ADD` and `FIND` to interact with the database. |

---

## 📝 **How Data is Stored**
### **1️⃣ Creating Nodes (Directories)**
Nodes are created dynamically when a user adds a new path.
```c
Node *create_node(Node *parent, int8 *path);
```
#### **Example**
```c
Node *users = create_node((Node *)&root, "/users");
```
Creates `/users` under root.

---

### **2️⃣ Storing Key-Value Pairs**
Key-value pairs are stored as **leaves** under a node.
```c
Leaf *create_leaf(Node *parent, int8 *key, int8 *value, int16 count);
```
#### **Example**
```c
create_leaf(users, "john", "25", 2);
```
Stores `john -> 25` under `/users`.

---

### **3️⃣ Retrieving Data**
Users can retrieve values using **linear lookup**.
```c
int8 *lookup_linear(int8 *path, int8 *key);
```
#### **Example**
```c
lookup_linear("/users", "john");
```
🔹 **Returns:** `"25"`

---

## 🛠️ **TCP Communication**
Users interact with the database over **TCP** using commands.

### **Supported Commands**
| Command | Description |
|---------|------------|
| `ADD /a/b key value` | Adds a key-value pair under `/a/b`. |
| `FIND /a/b key` | Retrieves the value of `key` under `/a/b`. |

### **Example TCP Requests**
#### **Client Sends**
```
add /users/john age 25
```
#### **Server Response**
```
Added: john -> 25 under /users
```
#### **Client Sends**
```
find /users john
```
#### **Server Response**
```
Found: john -> 25
```

---

## 🚀 **Handling Missing Parent Nodes**
The system **automatically creates missing parent directories** when adding data.

### **Before Fix**
❌ **Fails if `/a` is missing** before `/a/b` is created.

### **After Fix**
✅ **Automatically creates `/a` when adding `/a/b`**.
```c
Node *create_node(Node *parent, int8 *path)
{
    if (!parent) {
        char parent_path[256];
        strncpy(parent_path, path, 255);
        char *last_slash = strrchr(parent_path, '/');
        if (last_slash) {
            *last_slash = '\0';
            parent = find_node_linear(parent_path);
            if (!parent) {
                parent = create_node((Node *)&root, parent_path);
            }
        }
    }
    ...
}
```


## 🎯 **How to Run**
### **1️⃣ Compile the Code**
```sh
make
```

### **2️⃣ Run the Server**
```sh
./redis 
```

### **3️⃣ Connect Using a Client (e.g., Netcat)**
```sh
telnet localhost 6379
```

### **4️⃣ Send Commands**
```
add /users/john age 25
find /users john
```

---

## 📝 **Conclusion**
- **Tree-based hierarchical database**
- **Stores key-value pairs inside nodes**
- **Handles TCP requests for `ADD` and `FIND`**
- **Automatically creates missing parent directories**
- **Optimizable for larger datasets**

Would you like **hash-based lookups** or **disk storage integration** next? 🚀

