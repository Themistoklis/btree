#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btree.h"

struct data {
	BTREE;
	int key;
};

static int compare_node(const void* a, const void* b)
{
	if (((const struct data*)a)->key == ((const struct data*)b)->key) {
		return 0;
	}
	return ((const struct data*)a)->key < ((const struct data*)b)->key ? -1 : 1;
}

static int compare_node_key(const void* a, const void* key_b)
{
	if (((struct data*)a)->key == *(int*)key_b) {
		return 0;
	}
	return ((struct data*)a)->key < *(int*)key_b ? -1 : 1;
}

static const void* get_key_ptr(const btree_node_t* node)
{
	return &((const struct data*)node)->key;
}

void mermaid(btree_node_t* node_ptr)
{
	struct data* node = node_ptr;
	printf("\tN%d((Id: %d\\n Height: %d))\n", node->key, node->key, node->__tree.height);
	if (node->__tree.left) {
		printf("\t\tN%d-->N%d\n", node->key, ((struct data*)(node->__tree.left))->key);
	}
	else {
		printf("\t\tN%d-->EL%d((null))\n", node->key, node->key);
	}
	if (node->__tree.right) {
		printf("\t\tN%d-->N%d\n", node->key, ((struct data*)node->__tree.right)->key);
	}
	else {
		printf("\t\tN%d-->ER%d((null))\n", node->key, node->key);
	}
}

static void print_mermaid(struct btree* tree)
{
	printf("Mermaid diagram:\n");
	printf("```mermaid\n");
	printf("graph TB;\n");
	printf("\tEP%d[ROOT]-->N%d\n", ((struct data*)tree->root)->key, ((struct data*)tree->root)->key);
	btree_walk(tree, mermaid);
	printf("```\n");
}

int main(void)
{
	struct btree tree;

	tree.compare_node     = compare_node;
	tree.get_key_ptr      = get_key_ptr;
	tree.compare_node_key = compare_node_key;

	btree_init(&tree);

	struct data n1  = { 0 };
	n1.key          = 50;
	struct data n2  = { 0 };
	n2.key          = 10;
	struct data n3  = { 0 };
	n3.key          = 47;
	struct data n4  = { 0 };
	n4.key          = 108;
	struct data n5  = { 0 };
	n5.key          = 13;
	struct data n6  = { 0 };
	n6.key          = 5;
	struct data n7  = { 0 };
	n7.key          = 1;
	struct data n8  = { 0 };
	n8.key          = 127;
	struct data n9  = { 0 };
	n9.key          = 354562934;
	struct data n10 = { 0 };
	n10.key         = 2345;
	struct data n11 = { 0 };
	n11.key         = 123;
	struct data n12 = { 0 };
	n12.key         = 111;

	btree_insert(&tree, &n1);
	btree_insert(&tree, &n2);
	btree_insert(&tree, &n3);
	btree_insert(&tree, &n4);
	btree_insert(&tree, &n5);
	btree_insert(&tree, &n6);
	btree_insert(&tree, &n7);
	btree_insert(&tree, &n8);
	btree_insert(&tree, &n9);
	btree_insert(&tree, &n10);
	btree_insert(&tree, &n11);
	btree_insert(&tree, &n12);

	print_mermaid(&tree);

	int i = 354562934;
	btree_remove(&tree, &i);
	print_mermaid(&tree);

	i = 127;
	btree_remove(&tree, &i);
	print_mermaid(&tree);

	return 0;
}
