#include <stdio.h>

#ifndef UGTREE_H
#define UGTREE_H

//just use to pass multiple params into callback func
typedef struct Capsule{
	gpointer data1;
	gpointer data2;
	gpointer data3;
} Capsule;


typedef struct Node{
	char name[128];
	int type;      /* 0---user, 1---group, 2---group with all user*/
	float share;   /* the share computed from parent group*/
	int ratio;  /* the ratio number defined in lsb.users*/
	int isActive;  /* 0---inactive, 1---active*/
	int numPendJob; /* Number of pending job submit by this user */
	int numRunJob; /*Reserved. Number of running job submit by this user */
	float priority; /* share/numPendJob */
	int updatedFlag; /* 0---no update, 1---updated */
	GHashTable* pDescendant;  /* used to record the users computed when update tree*/
} Node;

typedef struct UserJobInfo{
	char name[128];
	int numPendJob; /* Number of pending job submit by this user */
	int numRunJob; /*Reserved. Number of running job submit by this user */
} UserJobInfo;


typedef struct Group{
	char name[128];
	GArray* members; 
	int type; /* 0---group, 1---group with all user*/
	int share; /* default -1, just for group with all user */
} Group;

Node* node_new_user(char* nodeName, int oriShare);
Node* node_new_group(char* nodeName, int oriShare);

void printGroups(GArray* groups);
int initTree(GNode** root, GArray* groups);
GNode* findParentInLeaves(GNode* proot, Node* pfindNode);
GNode* findNodeInLevelOfTree(GNode* proot, int depth, Node* pfindNode);
void printTree(GNode *root);
int computeTreeShare(GNode* proot);
int setTopLevelGroupShare(GNode* proot);
int updateTreeJobInfo(GNode* proot, GHashTable* phash);
int updateDescendantOfTreeNode(GNode* proot);
void computePriority(Node *pnode);
void tree_free(GNode* proot);


#endif

