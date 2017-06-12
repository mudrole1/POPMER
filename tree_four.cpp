#include <iostream>
#include <string.h>
#include "tree_four.h"

using namespace std;

//--------------------------------------------
// Function: Tree_four()
// Purpose: Class constructor.
//--------------------------------------------
Tree_four::Tree_four()
{
    root = NULL;
    return;
}

//--------------------------------------------
// Function: Tree_four()
// Purpose: Class destructor.
//--------------------------------------------
Tree_four::~Tree_four()
{
    ClearTree(root);
    return;
}

//--------------------------------------------
// Function: ClearTree()
// Purpose: Perform a recursive traversal of
//        a tree destroying all nodes.
//--------------------------------------------
void Tree_four::ClearTree(TreeNode *T)
{
    if(T==NULL) return;  // Nothing to clear
    if(T->conti != NULL) ClearTree(T->conti); // Clear left sub-tree
    if(T->repeat != NULL) ClearTree(T->repeat); // Clear right sub-tree
    delete T;    // Destroy this node
    return;
}

//--------------------------------------------
// Function: isEmpty()
// Purpose: Return TRUE if tree is empty.
//--------------------------------------------
bool Tree_four::isEmpty()
{
    return(root==NULL);
}

//--------------------------------------------
// Function: DupNode()
// Purpose: Duplicate a node in the tree.  This
//        is used to allow returning a complete
//        structure from the tree without giving
//        access into the tree through the pointers.
// Preconditions: None
// Returns: Pointer to a duplicate of the node arg
//--------------------------------------------
TreeNode *Tree_four::DupNode(TreeNode * T)
{
    TreeNode *dupNode;

    dupNode = new TreeNode();
    *dupNode = *T;    // Copy the data structure
    dupNode->conti = NULL;    // Set the pointers to NULL
    dupNode->repeat = NULL;
    return dupNode;
}

//--------------------------------------------
// Function: SearchTree()
// Purpose: Perform an iterative search of the tree and
//        return a pointer to a treenode containing the
//        search key or NULL if not found.
// Preconditions: None
// Returns: Pointer to a duplicate of the node found
//--------------------------------------------
/*TreeNode *Tree_four::SearchTree(size_t id, std::string type)
{
    TreeNode *temp;

    temp = root;
    while((temp != NULL) && (temp->data->second.second != id)&&(temp->data->second.first != type))
    {
        temp = temp->conti;
    }
    return temp;
}*/

//--------------------------------------------
// Function: Insert()
// Insert a new node into the tree.
// Preconditions: None
// Returns: int (TRUE if successful, FALSE otherwise)
//--------------------------------------------
int Tree_four::insert(TreeNode *newNode, TreeNode *oldNode)
{
   if(newNode->key=='r')
   {
       if(oldNode->repeat != NULL)
       {
           std::cout << "we want to insert to repeat branch, but it is not empty\n";
           throw;
       }
       else
       {
           oldNode->repeat = newNode;
       }
   }

    //TODO other branches later
}

int Tree_four::insert(TreeNode *newNode)
{
    TreeNode *temp;
    TreeNode *back;

    temp = root;
    back = NULL;

    if(newNode->key == 'c') //continue - add to the end
    {
        while(temp != NULL) // Loop till temp falls out of the tree
        {
            back = temp;
            temp = temp->conti;
        }

        // Now attach the new node to the node that back points to
        if(back == NULL) // Attach as root node in a new tree
            root = newNode;
        else
        {
            back->conti = newNode;
        }
    }
    else if(newNode->key == 'r')
    {
        while((temp != NULL) && (temp->data->second.second != newNode->id)&&(temp->data->second.first != newNode->type))
        { // Loop till temp falls out of the tree
            back = temp;
            temp = temp->conti;
        }

        // Now attach the new node to the node that back points to
        if(back == NULL) // Attach as root node in a new tree
            root = newNode;
        else
        {
            back->repeat = newNode;
        }
    }
    return(true);
}

//--------------------------------------------
// Function: Insert()
// Insert a new node into the tree.
// Preconditions: None
// Returns: int (TRUE if successful, FALSE otherwise)
//--------------------------------------------
int Tree_four::insert(std::shared_ptr<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > > d)
{
    TreeNode *newNode;

    // Create the new node and copy data into it
    newNode = new TreeNode();
    newNode->data = d;
    newNode->key = 'c'; //continue
    newNode->id = -1; //add to the end
    newNode->type = ""; //add to the end
    newNode->conti = newNode->repeat = NULL;

    // Call other Insert() to do the actual insertion
    return(insert(newNode));
}

//--------------------------------------------
// Function: Delete()
// Purpose: Delete a node from the tree.
// Preconditions: Tree contains the node to delete
// Returns: int (TRUE if successful, FALSE otherwise)
//--------------------------------------------
/*int Tree_four::Delete(int Key)
{
    TreeNode *back;
    TreeNode *temp;
    TreeNode *delParent;    // Parent of node to delete
    TreeNode *delNode;      // Node to delete

    temp = root;
    back = NULL;

    // Find the node to delete
    while((temp != NULL) && (Key != temp->Key))
    {
        back = temp;
        if(Key < temp->Key)
            temp = temp->left;
        else
            temp = temp->right;
    }

    if(temp == NULL) // Didn't find the one to delete
    {
        cout << "Key not found. Nothing deleted.\n";
        return false;
    }
    else
    {
        if(temp == root) // Deleting the root
        {
            delNode = root;
            delParent = NULL;
        }
        else
        {
            delNode = temp;
            delParent = back;
        }
    }

    // Case 1: Deleting node with no children or one child
    if(delNode->right == NULL)
    {
        if(delParent == NULL)    // If deleting the root
        {
            root = delNode->left;
            delete delNode;
            return true;
        }
        else
        {
            if(delParent->left == delNode)
                delParent->left = delNode->left;
            else
                delParent->right = delNode->left;
                delete delNode;
            return true;
        }
    }
    else // There is at least one child
    {
        if(delNode->left == NULL)    // Only 1 child and it is on the right
        {
            if(delParent == NULL)    // If deleting the root
            {
                root = delNode->right;
                delete delNode;
                return true;
            }
            else
            {
                if(delParent->left == delNode)
                    delParent->left = delNode->right;
                else
                    delParent->right = delNode->right;
                delete delNode;
                return true;
            }
        }
        else // Case 2: Deleting node with two children
        {
            // Find the replacement value.  Locate the node
            // containing the largest value smaller than the
            // key of the node being deleted.
            temp = delNode->left;
            back = delNode;
            while(temp->right != NULL)
            {
                back = temp;
                temp = temp->right;
            }
            // Copy the replacement values into the node to be deleted
            delNode->Key = temp->Key;
            delNode->fValue = temp->fValue;
            delNode->iValue = temp->iValue;
            strcpy(delNode->cArray, temp->cArray);

            // Remove the replacement node from the tree
            if(back == delNode)
                back->left = temp->left;
            else
                back->right = temp->left;
            delete temp;
            return true;
        }
    }
}*/

//--------------------------------------------
// Function: PrintOne()
// Purpose: Print data in one node of a tree.
// Preconditions: None
// Returns: void
//--------------------------------------------
void Tree_four::PrintOne(TreeNode *T)
{
    std::shared_ptr<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > > s = T->data;
    std::cout << "id: " << s->second.second << s->second.first << ":" << *s->first.get()<< "\n";
}

//--------------------------------------------
// Function: PrintAll()
// Purpose: Print the tree using a recursive
//        traversal
// Preconditions: None
// Returns: void
//--------------------------------------------
void Tree_four::PrintAll(TreeNode *T)
{
    if(T != NULL)
    {
        PrintAll(T->conti);
        PrintOne(T);
        PrintAll(T->repeat);
    }
}

//--------------------------------------------
// Function: PrintTree()
// Purpose: Print the tree using a recursive
//        traversal.  This gives the user access
//        to PrintAll() without giving access to
//        the root of the tree.
// Preconditions: None
// Returns: void
//--------------------------------------------
void Tree_four::PrintTree()
{
    PrintAll(root);
}

TreeNode * Tree_four::getNode()
{
   return this->root;
}


TreeNode * Tree_four::getSubTree(char k)
{
    if(k=='c')
        return this->root->conti;
    else if(k == 'r')
        return this->root->repeat;

}
