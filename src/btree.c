/*!
 * \file btree.c
 * \author Guillaume Valentis (guillaumevalentis@gmail.com)
 * \brief Generic binary tree optimized for embedded systems
 * \version 0.1
 * \date 2023-03-24
 *
 * \copyright MIT License Copyright (c) 2023 Guillaume Valentis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <errno.h>
#include "btree.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))

__attribute__((always_inline)) inline static int height(const struct priv_btree_node* node)
{
	if (node == NULL) {
		return 0;
	}
	return node->height;
}

__attribute__((always_inline)) inline static int update_height(const struct priv_btree_node* root)
{
	return 1 + max(height(root->left), height(root->right));
}

static struct priv_btree_node* r_rotate(struct priv_btree_node* y)
{
	struct priv_btree_node* x = y->left;
	struct priv_btree_node* z = x->right;

	// rotate
	x->right = y;
	y->left  = z;

	y->height = update_height(y);
	x->height = update_height(x);

	// new root
	return x;
}

static struct priv_btree_node* l_rotate(struct priv_btree_node* x)
{
	struct priv_btree_node* y = x->right;
	struct priv_btree_node* z = y->left;

	// rotate
	y->left  = x;
	x->right = z;

	x->height = update_height(x);
	y->height = update_height(y);

	// new root
	return y;
}

static int get_balance(const struct priv_btree_node* node)
{
	if (node == NULL) {
		return 0;
	}
	return height(node->left) - height(node->right);
}

static struct priv_btree_node* min_value_node(struct priv_btree_node* node)
{
	struct priv_btree_node* tmp = node;

	while (tmp->left != NULL) {
		tmp = tmp->left;
	}

	return tmp;
}

static struct priv_btree_node* insert(struct btree* tree, struct priv_btree_node* root, struct priv_btree_node* node)
{
	struct priv_btree_node* ret  = NULL;
	int                     comp = 0;

	if (root == NULL) {
		node->height = 1;
		return node;
	}

	comp = tree->compare_node(node, root);
	if (comp < 0) {
		ret = insert(tree, root->left, node);
		if (ret == NULL) {
			return NULL;
		}
		root->left = ret;
	}
	else if (comp > 0) {
		ret = insert(tree, root->right, node);
		if (ret == NULL) {
			return NULL;
		}
		root->right = ret;
	}
	else {
		return NULL;
	}

	root->height = update_height(root);

	int b = get_balance(root);
	if (b > 1) {
		comp = tree->compare_node(node, root->left);
		if (comp < 0) {
			return r_rotate(root);
		}
		if (comp > 0) {
			root->left = l_rotate(root->left);
			return r_rotate(root);
		}
	}

	if (b < -1) {
		comp = tree->compare_node(node, root->right);
		if (comp > 0) {
			return l_rotate(root);
		}
		if (comp < 0) {
			root->right = r_rotate(root->right);
			return l_rotate(root);
		}
	}

	return root;
}

static struct priv_btree_node* balance(struct priv_btree_node* root)
{
	int balance = get_balance(root);

	// left-left
	if (balance > 1 && get_balance(root->left) >= 0) {
		return r_rotate(root);
	}

	// left-right
	if (balance > 1 && get_balance(root->left) < 0) {
		root->left = l_rotate(root->left);
		return r_rotate(root);
	}

	// right-right
	if (balance < -1 && get_balance(root->right) <= 0) {
		return l_rotate(root);
	}

	// rigth-left
	if (balance < -1 && get_balance(root->right) > 0) {
		root->right = r_rotate(root->right);
		return l_rotate(root);
	}

	return root;
}

static struct priv_btree_node* delete_node(struct btree* tree, struct priv_btree_node* root, const void* key_ptr,
                                           struct priv_btree_node** rm_node)
{
	int comp = 0;

	if (root == NULL || key_ptr == NULL) {
		return NULL;
	}

	comp = tree->compare_node_key(root, key_ptr);
	if (comp > 0) {
		root->left = delete_node(tree, root->left, key_ptr, rm_node);
	}
	else if (comp < 0) {
		root->right = delete_node(tree, root->right, key_ptr, rm_node);
	}
	else {
		*rm_node = root;
		if ((root->left == NULL) || (root->right == NULL)) {
			struct priv_btree_node* tmp = root->left ? root->left : root->right;
			if (tmp == NULL) {
				root = NULL;
			}
			else {
				// NOTE: zero copy to test and validate
				root = tmp;
			}
		}
		else {
			struct priv_btree_node* tmp     = min_value_node(root->right);
			struct priv_btree_node* removed = NULL;

			root->right = delete_node(tree, root->right, tree->get_key_ptr(tmp), &removed);

			// Replace the removed node in place of the current root
			// We know the removed node should exist (fetched min_value_node can't be NULL)
			if (removed != NULL) {
				removed->left  = root->left;
				removed->right = root->right;
			}
			root = removed;
		}
	}

	if (root == NULL) {
		return NULL;
	}

	root->height = update_height(root);

	return balance(root);
}

static void walk(struct priv_btree_node* node, void (*fn)(btree_node_t*))
{
	if (node != NULL) {
		walk(node->left, fn);
		fn(node);
		walk(node->right, fn);
	}
}

static btree_node_t* get_node(struct btree* tree, struct priv_btree_node* node, const void* key_ptr)
{
	if (node == NULL) {
		return NULL;
	}

	int comp = tree->compare_node_key(node, key_ptr);
	if (comp > 0) {
		return get_node(tree, node->left, key_ptr);
	}
	if (comp < 0) {
		return get_node(tree, node->right, key_ptr);
	}
	return node;
}

void btree_init(struct btree* tree)
{
	tree->root = NULL;
	tree->size = 0;
}

int btree_insert(struct btree* tree, btree_node_t* node_ptr)
{
	struct priv_btree_node* ret  = NULL;
	struct priv_btree_node* node = node_ptr;

	if (node == NULL || tree->get_key_ptr(node) == NULL) {
		return -EINVAL;
	}

	node->right  = NULL;
	node->left   = NULL;
	node->height = 1;

	if (tree->root == NULL) {
		tree->root = node;
		tree->size++;
		return 0;
	}

	ret = insert(tree, tree->root, node);

	if (ret == NULL) {
		return -EEXIST;
	}

	tree->root = ret;
	tree->size++;

	return 0;
}

btree_node_t* btree_remove(struct btree* tree, const void* key_ptr)
{
	struct priv_btree_node* removed = NULL;
	tree->root                      = delete_node(tree, tree->root, key_ptr, &removed);
	tree->size--;
	return removed;
}

void btree_walk(struct btree* tree, void (*fn)(btree_node_t*))
{
	if (tree == NULL || fn == NULL) {
		return;
	}
	walk(tree->root, fn);
}

btree_node_t* btree_get(struct btree* tree, const void* key_ptr)
{
	if (tree == NULL || key_ptr == NULL) {
		return NULL;
	}
	return get_node(tree, tree->root, key_ptr);
}

