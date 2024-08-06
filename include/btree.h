/*!
 * \file btree.h
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
#ifndef BTREE_H_
#define BTREE_H_

#include <stdint.h>
#include <stdbool.h>

#define BTREE struct priv_btree_node __tree

typedef void btree_node_t;

struct btree {
	struct priv_btree_node* root;
	uint32_t                size;

	int (*compare_node)(const void* a, const void* b); // a < b => < 0, a == b => 0, a > b => > 0
	int (*compare_node_key)(const void* a, const void* key_b);
	const void* (*get_key_ptr)(const btree_node_t* node);
};

struct priv_btree_node {
	struct priv_btree_node* left;
	struct priv_btree_node* right;
	int                     height;
};

/*!
 * \brief Initialize the binary tree
 *
 * \param[in,out] tree structure context of the binary tree
 */
void btree_init(struct btree* tree);

/*!
 * \brief Insert a new node in the binary tree
 *
 * A key must be already set for the node to be inserted.
 *
 * \param[in,out] tree structure context of the binary tree
 * \param[in] node_ptr new node to insert in the tree
 *
 * \return int errno or 0 on success
 */
int btree_insert(struct btree* tree, btree_node_t* node);

/*!
 * \brief Remove the node with the fiven key from the tree and return it
 *
 * \param[in,out] tree structure context of the binary tree
 * \param[in] key_ptr pointer to the key \ref btree
 *
 * \return btree_node_t* the removed node from the binary tree
 */
btree_node_t* btree_remove(struct btree* tree, const void* key_ptr);

/*!
 * \brief Go through each node in order in the tree and call the given function
 *
 * \param[in,out] tree structure context of the binary tree
 * \param[in] fn function called for each node in order
 */
void btree_walk(struct btree* tree, void (*fn)(btree_node_t*));

/*!
 * \brief Get the node 
 *
 * \param[in,out] tree structure context of the binary tree
 * \param[in] key_ptr pointer to the key \ref btree
 *
 * \return btree_node_t* the node found in the tree or NULL
 */
btree_node_t* btree_get(struct btree* tree, const void* key_ptr);

#endif /* BTREE_H_ */
