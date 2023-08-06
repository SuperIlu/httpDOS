/*!\file btree.c
 *
 *\note <b>Not used</b>
 *
 * Binary Tree with variably sized data and node deletion.
 *
 * C User Journal May 1992
 */

#include <stdio.h>
#include <string.h>

#include "wattcp.h"
#include "misc.h"
#include "misc_str.h"
#include "btree.h"

/*
 *  Non-recursive tree insertion.
 *  If duplicate key then new data is substituted.
 *
 *  returns 1 on success
 *          0 on failure
 */
int tree_insert (TreeNode **root, const void *info, size_t info_size, CmpFunc cmp)
{
  TreeNode *node = *root;

  /* does not enter while loop when instantiating root
   */
  while (node)
  {
    int diff = (*cmp)(info, node->info);

    if (diff == 0)
    {
      free (node->info);
      goto tree_load;
    }
    else if (diff < 0)
    {
      if (node->left)
          node = node->left;
      else
      {
        node->left = calloc (1, sizeof(*node));
        if (node->left)
            node->left->parent = node;
        node = node->left;
        goto tree_load;
      }
    }
    else
    {
      if (node->right)
          node = node->right;
      else
      {
        node->right = calloc (1, sizeof(*node));
        if (node->right)
            node->right->parent = node;
        node = node->right;
        goto tree_load;
      }
    }
  }

  /* only arrives here when instantiating root
   */
  node  = calloc (1, sizeof(*node));
  *root = node;

tree_load:
  if (!node)
     return (0);

  node->info = calloc (1, info_size);
  if (!node->info)
  {
    free (node);
    return (0);
  }

  memcpy (node->info, info, info_size);
  node->info_size = info_size;
  return (1);
}

/*
 *  Non-recursive find, returns first matching entry or NULL
 */
TreeNode *tree_find (TreeNode *root, const void *info, CmpFunc cmp)
{
  TreeNode *node = root;

  while (node)
  {
    int cmp_val = (*cmp)(info, node->info);

    if (cmp_val == 0)
       break;

    if (cmp_val < 0)
         node = node->left;
    else node = node->right;
  }
  return (node);
}

TreeNode *tree_delete (TreeNode *root, TreeNode *node)
{
  TreeChild child;
  TreeNode *temp;

  if (!node || !root)
     return (root);

  if (root == node)
       child = NOT_CHILD;
  else if (node->parent->left == node)
       child = LEFT_CHILD;
  else if (node->parent->right == node)
       child = RIGHT_CHILD;
  else return (root);

  if (!node->right)
  {
    temp = node;
    node = node->left;
  }
  else if (!node->left)
  {
    temp = node;
    node = node->right;
  }
  else
  {
    temp = node->right;
    while (temp->left)
       temp = temp->left;

    temp->left = node->left;
    temp->left->parent = temp;
    temp = node;
    node = node->right;
  }

  switch (child)
  {
    case NOT_CHILD:
         free (temp->info);
         free (temp);
         if (node)
             node->parent = NULL;
         return (node);

    case LEFT_CHILD:
         temp->parent->left = node;
         if (node)
             node->parent = temp->parent;
         free (temp->info);
         free (temp);
         break;

    case RIGHT_CHILD:
         temp->parent->right = node;
         if (node)
             node->parent = temp->parent;
         free (temp->info);
         free (temp);
         /* fall-through */
  }
  return (root);
}

/*
 *  Recursive post-order traversal to deallocate tree memory
 */
void tree_free (TreeNode *root)
{
  if (!root)
     return;

  tree_free (root->left);
  tree_free (root->right);
  if (root->info)
     free (root->info);
  free (root);
}

#if defined(TEST_PROG)
/*
 *  Interactive tree test driver
 */
#ifdef __DJGPP__
#include <conio.h>  /* getch() */
#endif

#define KEYSIZE 80

typedef enum tree_order {
        NO_ORDER, PRE_ORDER,
        IN_ORDER, POST_ORDER
      } tree_order;

tree_order t_order = IN_ORDER;

typedef struct {
        char key[KEYSIZE];
        int  id;
      } record;

static record rec;

void prompt (const char *verb)
{
  printf ("\nEnter String to %s\t( <Enter> for none )\n>", verb);
}

int MS_CDECL rec_cmp (const void *a, const void *b)
{
  const record *rec1 = (const record*) a;
  const record *rec2 = (const record*) b;
  return strcmp (rec1->key, rec2->key);
}

void tree_print (const record *record)
{
  if (record)
     printf ("Key:           %s\n"
             "Record Number: %d\n", record->key, record->id);
}

/*
 * Recursive tree traversal function with 3 traversing modes
 */
void tree_trace (TreeNode *root)
{
  if (!root)
     return;

  switch (t_order)
  {
    case PRE_ORDER:
         tree_print (root->info);
         tree_trace (root->left);
         tree_trace (root->right);
         return;

    case IN_ORDER:
         tree_trace (root->left);
         tree_print (root->info);
         tree_trace (root->right);
         return;

    case POST_ORDER:
         tree_trace (root->left);
         tree_trace (root->right);
         tree_print (root->info);
         return;

    default:
         return;
  }
}

int main (void)
{
  TreeNode *tree_root = NULL;
  TreeNode *found     = NULL;

  char *orders[4]    = { "no-order", "pre-order", "in-order", "post-order" };
  char  buf[KEYSIZE] = "";
  int   record_num   = 0;
  int   ch, count    = 0;

  prompt ("Insert");

  while (fgets(buf, sizeof(buf), stdin))
  {
    _strlcpy (rec.key, buf, KEYSIZE);
    rec.id = ++record_num;
    if (!tree_insert (&tree_root, &rec, sizeof(rec), (CmpFunc)rec_cmp))
    {
      printf ("\n\tTree Insertion Failure!\n");
      return (1);
    }
    prompt ("Insert");
  }

  prompt ("Delete");
  fgets (buf, sizeof(buf), stdin);
  rec.key[0] = '\0';
  _strlcpy (rec.key, buf, KEYSIZE);

  while ((found = tree_find (tree_root, &rec, (CmpFunc)rec_cmp)) != NULL)
  {
    tree_print (found->info);
    tree_root = tree_delete (tree_root, found);
    count++;
  }

  printf ("\n\t%d String(s) Deleted\n"
          "\n\tSelect Tree Traversal Type\n"
          "\n\t\t1) pre-order\n"
            "\t\t2) in-order\n"
            "\t\t3) post-order\n\n"
            "\t>", count);

  ch = getch();
  ch -= '0';
  if (ch < PRE_ORDER || ch > POST_ORDER)
     ch = NO_ORDER;

  printf ("\n\t... Walking Tree %s ...\n\n", orders[ch]);
  t_order = ch;
  tree_trace (tree_root);
  tree_free (tree_root);
  return (0);
}
#endif  /* TEST_PROG */
