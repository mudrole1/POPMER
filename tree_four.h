#ifndef TREE_FOUR_H
#define TREE_FOUR_H

#include <iostream>
#include "steptimed.h"

// Define a structure to be used as the tree node
struct TreeNode
{
    std::shared_ptr<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > > data;
    TreeNode *conti;
    TreeNode *repeat; // TODO I think I will need multiple repeat branches,
                      // based on what goal is not valid anymore
    char key; //to which branch it should go
    size_t id; //id of the preceding step
    std::string type; //start or end of the preceding step

};

class Tree_four
{
    private:


    public:
        TreeNode *root;
        Tree_four();
        ~Tree_four();
        bool isEmpty();
        TreeNode *SearchTree(size_t id, std::string type);
        int insert(TreeNode *newNode);
        int insert(TreeNode *newNode, TreeNode *oldNode);
        int insert(std::shared_ptr<std::pair<std::shared_ptr<StepTimed>, std::pair<std::string, size_t> > > d);
        //int Delete(int Key);
        void PrintOne(TreeNode *T);
        void PrintTree();
        TreeNode * getNode();
        TreeNode * getSubTree(char k);

    private:
        void ClearTree(TreeNode *T);
        TreeNode *DupNode(TreeNode * T);
        void PrintAll(TreeNode *T);
};


#endif // TREE_FOUR_H

