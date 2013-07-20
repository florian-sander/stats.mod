/*
 * Copyright (C) 2000,2001  Florian Sander
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#define GENERIC_BINARY_TREE 1

#ifdef DYNAMIC_MEM_DEBUG
#define bt_malloc(x)	malloc(x)
#define bt_free(x)		free(x)
#else
#define bt_malloc	nmalloc
#define	bt_free		nfree
#endif

struct generic_binary_tree {
  void *root;
  int (*comparedata) (void *data1, void *data2);
  int (*expmemdata) (void *data);
  void (*freedata) (void *data);
};

struct generic_binary_tree_node {
  void *data;
  void *left;
  void *right;
};

static void btree_add(struct generic_binary_tree *, void *);
//static int btree_expmem(struct generic_binary_tree *);
static int btree_recursive_expmem(struct generic_binary_tree *, struct generic_binary_tree_node *);
static void *btree_get(struct generic_binary_tree *, void *t);
static void btree_freetree(struct generic_binary_tree *);
static void btree_recursive_free(struct generic_binary_tree *,
                                 struct generic_binary_tree_node *);
static void btree_getall(struct generic_binary_tree *, void (*) (void *));
static void btree_recursive_getall(struct generic_binary_tree_node *,
                                   void (*) (void *));
//static void btree_getall_expanded(struct generic_binary_tree *tree, void (*) (void *));
static void btree_recursive_getall_expanded(struct generic_binary_tree_node *,
                                            void (*) (void *));
static void btree_remove(struct generic_binary_tree *, void *);

static void btree_add(struct generic_binary_tree *tree, void *data)
{
  struct generic_binary_tree_node *node, *lastnode;
  int cmp, lastcmp;

  Assert(tree);
  Assert(data);
  cmp = lastcmp = 0;
  node = tree->root;
  lastnode = NULL;
  while (node) {
    cmp = tree->comparedata(node->data, data);
    if (!cmp) {
      // item is identical -> free old data and insert new
      tree->freedata(node->data);
      node->data = data;
      return;
    }
    lastnode = node;
    lastcmp = cmp;
    if (cmp < 0)
      node = node->left;
    else
      node = node->right;
  }
  node = bt_malloc(sizeof(struct generic_binary_tree_node));
  node->left = NULL;
  node->right = NULL;
  node->data = data;
  if (!lastnode)
    tree->root = node;
  else {
    Assert(lastcmp);
    if (lastcmp < 0) {
      Assert(!lastnode->left);
      lastnode->left = node;
    } else {
      Assert(!lastnode->right);
      lastnode->right = node;
    }
  }
}

/*
static int btree_expmem(struct generic_binary_tree *tree)
{
  int size = 0;

  Assert(tree);
  size += btree_recursive_expmem(tree, tree->root);
  return size;
}
*/

static int btree_recursive_expmem(struct generic_binary_tree *tree, struct generic_binary_tree_node *node)
{
  int size = 0;

  if (!node)
    return 0;
  size += sizeof(struct generic_binary_tree_node);
  size += tree->expmemdata(node->data);
  size += btree_recursive_expmem(tree, node->left);
  size += btree_recursive_expmem(tree, node->right);
  return size;
}

static void *btree_get(struct generic_binary_tree *tree, void *what)
{
  struct generic_binary_tree_node *node;
  int cmp;

  node = tree->root;
  while (node) {
    cmp = tree->comparedata(node->data, what);
    if (!cmp)
      return node->data;
    if (cmp < 0)
      node = node->left;
    else
      node = node->right;
  }
  return NULL;
}

static void btree_freetree(struct generic_binary_tree *tree)
{
  btree_recursive_free(tree, tree->root);
}

static void btree_recursive_free(struct generic_binary_tree *tree,
                                 struct generic_binary_tree_node *node)
{
  if (!node)
    return;
  btree_recursive_free(tree, node->left);
  btree_recursive_free(tree, node->right);
  tree->freedata(node->data);
  bt_free(node);
}

/* btree_getall():
 * calls the specified function for each item in the tree.
 * NOTE: getall() calls the proc _before_ it proceeds into recursion. This way,
 *       one can savely store the tree into a file without mixing up its form.
 *       But if you delete an item from the called prcedure, this function
 *       WILL crash. Use btree_getall()_expanded instead.
 */
static void btree_getall(struct generic_binary_tree *tree, void (*func) (void *))
{
  Assert(tree);
  btree_recursive_getall(tree->root, func);
}

static void btree_recursive_getall(struct generic_binary_tree_node *node,
                                   void (*func) (void *))
{
  if (!node)
    return;
  // first call the function, then proceed into recursion
  // this way, the tree keeps in form if its saved to a file, for example
  Assert(func);
  func(node->data);

  btree_recursive_getall(node->left, func);
  btree_recursive_getall(node->right, func);
}

/* btree_getall_expanded():
 * the same as btree_getall(), but calls the function after the greatest level of recursion
 * has been reached. The node-pointers won't be accessed anymore when the first function
 * gets called. You can savely use this to free items.
 */
/*
static void btree_getall_expanded(struct generic_binary_tree *tree, void (*func) (void *))
{
  Assert(tree);
  btree_recursive_getall_expanded(tree->root, func);
}
*/

static void btree_recursive_getall_expanded(struct generic_binary_tree_node *node,
                                            void (*func) (void *))
{
  if (!node)
    return;
  btree_recursive_getall_expanded(node->left, func);
  btree_recursive_getall_expanded(node->right, func);

  Assert(func);
  func(node->data);
}

static void btree_remove(struct generic_binary_tree *tree, void *data)
{
  struct generic_binary_tree_node *node, *last, *largenode, *lastlarge;
  int ret, lastret;

  Assert(tree);
  Assert(data);
  last = NULL;
  lastret = 0;
  node = tree->root;
  while (node) {
    ret = tree->comparedata(node->data, data);
    if (ret == 0)
      break;
    last = node;
    lastret = ret;
    if (ret < 0)
      node = node->left;
    else
      node = node->right;
  }
  if (!node)  // oops, item not found
    return;
  if (!node->left && !node->right) {
    // *freu* no sub-branches! We can easily delete this item.
    if (last) {
      if (lastret < 0)
        last->left = NULL;
      else
        last->right = NULL;
    } else
      tree->root = NULL;
  } else if (!node->left) {
    // also pretty easy. Just connect the child to the parent.
    if (last) {
      if (lastret < 0)
        last->left = node->right;
      else
        last->right = node->right;
    } else
      tree->root = node->right;
  } else if (!node->right) {
    // same as above, but mirrored
    if (last) {
      if (lastret < 0)
        last->left = node->left;
      else
        last->right = node->left;
    } else
      tree->root = node->left;
  } else {
    // aaargh... two sub-trees! The world is not fair... *sigh*
//    debug0("argl... worst case, two subtrees. :( Let's pray...");
    // now we take the largest item from the left subtree and replace the
    // doomed node with it.
    // since it is the largest val, the tree remains valid and doesn't
    // get deformed too much.

    // at first, we have to find this node and cut it from the tree
    largenode = node->left;
    lastlarge = NULL;
    while (largenode && largenode->right) {
      lastlarge = largenode;
      largenode = largenode->right;
    }

    // only set largenode->left to node->left if largenode exists.
    // otherwise node->left points to largenode, which would result
    // in a nice short-circuit
    // If it does not exist, just leave largenode->left as it is because we just
    // move largenode one level up, so it can keep its left subtree.
    if (lastlarge) {
      lastlarge->right = largenode->left;
      largenode->left = node->left;
    }

    // now connect node's subtrees to it
    largenode->right = node->right;

    // and finally replace node with largenode
    if (last) {
      if (lastret < 0)
        last->left = largenode;
      else
        last->right = largenode;
    } else
      tree->root = largenode;
  }
  // finally kill the node... we shouldn't need it anymore
  tree->freedata(node->data);
  bt_free(node);
  node = NULL;
}

#ifdef BTREE_WITHOPTIMIZE
static void btree_optimize(struct generic_binary_tree *tree,
                           struct generic_binary_tree_node *node,
                           struct generic_binary_tree_node *last,
                           int limit)
{
/*  int leftdepth, rightdepth;

  if (!node)
    return;
  btree_optimize(tree, node->left, node, last, limit);
  btree_optimize(tree, node->right, node, last, limit);
  leftdepth = btree_depth(node->left);
  rightdepth = btree_depth(node->right);
  if ((leftdepth - rightdepth) > limit) {

  }
*/
}
#endif
