# btree

Ligthweight and portable C AVL tree.

## Table of contents

- [Usage](#usage)
  - [Building](#building)
    - [Copying the sources](#copying-the-sources)
    - [Makefile](#makefile)
    - [btree example](#btree-example)
  - [Example](#example)
    - [Make a structure into a tree node](#make-a-structure-into-a-tree-node)
    - [Initializing the tree](#initializing-the-tree)
    - [Using the tree](#using-the-tree)

## Usage

### Building

#### Copying the sources

Copy the two files into your project:

- `src/btree.c`
- `include/btree.h`

#### Makefile

Overwrite the makefile variables as required to generate a static lib.

Example for an arm project cross compilation with your git submodules inside a `submodules` folder:

```bash
make -C submodules/btree CC=arm-none-eabi-gcc AR=arm-none-eabi-ar CLFAGS=$CFLAGS LDFLAGS=$LDFLAGS lib
```

Do not forget to add the include path to your compilation and link the library:

```makefile
CLFAGS += -Isubmodules/btree/include
LDFLAGS += -Lsubmodules/btree -lbtree
```

#### btree example

Simply compile the example:

```bash
make example
./test.exe
```

### Example

#### Make a structure into a tree node

It is as simple as adding a define at the begining of your structure:

```c
struct user {
	BTREE;

	int user_id;
	char name[32];
};
```

#### Initializing the tree

You need to create three functions depending on the key used in the tree to sort it:

- compare two nodes
- compare a node with a pointer to a key
- get the key pointer from a node

Return rule for the compare functions for ascending order is:

- $a < b \Rightarrow -1$
- $a > b \Rightarrow 1$
- $a = b \Rightarrow 0$

Continuing with the above structure and considering the key is `user_id` here are our three functions:

```c
static int compare_node(const void* a, const void* b)
{
	if (((const struct user*)a)->user_id == ((const struct user*)b)->user_id) {
		return 0;
	}
	return ((const struct user*)a)->user_id < ((const struct user*)b)->user_id ? -1 : 1;
}

static int compare_node_key(const void* a, const void* key_b)
{
	if (((struct user*)a)->user_id == *(int*)key_b) {
		return 0;
	}
	return ((struct user*)a)->user_id < *(int*)key_b ? -1 : 1;
}

static const void* get_key_ptr(const btree_node_t* node)
{
	return &((const struct user*)node)->user_id;
}
```

Set the function pointers inside the btree structure and initialize the tree:

```c
struct btree tree;

tree.compare_node     = compare_node;
tree.get_key_ptr      = get_key_ptr;
tree.compare_node_key = compare_node_key;

btree_init(&tree);
```

#### Using the tree

The library does not allocate or free anything, you will need to manage that on your side.

You can't insert a node with a key that already exists in the tree, the function will return a `-EEXIST` errno.

Add an item to the tree:

```c
struct user new_user = {
	.user_id = 123,
	.name = "John",
};

btree_insert(&tree, &new_user);
```

The function to remove an item from the tree will also return the pointer to the item,
if the item was dynamically allocated you can then free it.

The function will return `NULL` if no node was found.

The `btree_remove` function takes a pointer to a key to remove, but in our example only the value matters:
```c
int key_to_rm = 123;

struct user* removed = btree_remove(&tree, &key_to_rm);
```

Getting an item from the tree, this function will return `NULL` if not item was found,
it works similarly to `btree_remove`.

```c
int key = 123;

struct user* user = btree_get(&tree, &key);
```

The walk function will call the function given in a parameter for each node in the tree.
The nodes are called in order (dictated by the two compare functions).

```c
void print_item(btree_node_t* node)
{
	struct user* user = node;
	printf("%d: %s\n", user->user_id, user->name);
}


btree_walk(&tree, print_item);
```
