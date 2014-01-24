#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "mbd.h"
#include "ugtree.h"

//only could be used in compareNode func, because g_node_children_foreach no return value
static GNode* tempFindGNode = NULL;

extern GHashTable* getGroupAllTypeDefine();
extern char** getAllUsersFromMBD();

void combineHashMap(GHashTable* des, GHashTable* src);
void node_free(Node* node);


void computePriority(Node *pnode){
	if(pnode->isActive){
		if(pnode->numRunJob <= 0){
			pnode->priority=pnode->share*MAX_SHARE_VALUE;
		}else{
			pnode->priority=pnode->share/pnode->numRunJob;
		}
	}else{
		pnode->priority=-1;
	}
	pnode->updatedFlag=1;
}


int setTopLevelGroupShare(GNode* proot){
	GNode* groupTop = proot->children;
	GNode* p=groupTop;
	while(NULL!=p){
		Node* d = p->data;
		if(d->share<=0) {
			GNode* children = p->children;
			GNode* q = children;
			int sum=0;
			while(NULL!=q){
				Node* data = q->data;
				sum += data->ratio;
				q=q->next;
			}
			d->share = d->ratio = sum;
		}
		p=p->next;
	}
	return 0;
}

/*
  * compute every node share value and set the type 
  */ 
static gboolean computeNodeShare(GNode *node,gpointer date){
	Node* parent=node->data;

	//if it is leaf, no need get the children to compute share
	if(G_NODE_IS_LEAF(node)) return FALSE;
	//if it is root, no need get the children to compute share
	if(G_NODE_IS_ROOT(node)) return FALSE;
	//if node's share is -1, error

	//if no leaf, set type as group
	if(0==parent->type) parent->type=1;
	if(parent->share<=0){
		ls_syslog(LOG_ERR,"There is no share value for member %s\n",parent->name);
		return TRUE;
	}
	
	GNode* children = node->children;
	float ratioSum = 0;
	GNode* p=children;
	while(NULL!=p){
		Node* d = p->data;
		if(-1==d->ratio){
			ls_syslog(LOG_ERR,"There is no ratio defined for member %s\n",d->name);
		}else{
			ratioSum += d->ratio;
		}
		p=p->next;
	}

	p=children;
	while(NULL!=p){
		Node* d = p->data;
		d->share = (d->ratio/ratioSum)*parent->share;
		p=p->next;
	}
	return FALSE;
}

int computeTreeShare(GNode* proot){
	g_node_traverse(proot, G_PRE_ORDER, G_TRAVERSE_NON_LEAVES, -1, computeNodeShare, NULL);
	return 0;
}

static gboolean updateDescendantOfNode(GNode *pn, gpointer pd){
	if(G_NODE_IS_ROOT(pn)) return TRUE;
	if(G_NODE_IS_LEAF(pn)) return FALSE;

	Node* data=pn->data;
	GHashTable* pDesc=data->pDescendant;
	
	GNode* pChildren = pn->children;
	GNode* p=pChildren;
	while(NULL!=p){
		Node* pData=p->data;
		if(G_NODE_IS_LEAF(p)){
			g_hash_table_insert(pDesc, strdup(pData->name), p);
			//ls_syslog(LOG_ERR,"insert %s\n",pData->name);
		}else{
			combineHashMap(pDesc, pData->pDescendant);
			//ls_syslog(LOG_ERR,"merge %s\n",pData->name);
		}
		p=p->next;
	}

	return FALSE;
}


int updateDescendantOfTreeNode(GNode* proot){
	g_node_traverse(proot, G_POST_ORDER, G_TRAVERSE_NON_LEAVES, -1, updateDescendantOfNode, NULL);
	return 0;
}

int initTree(GNode** root, GArray* groups){
	Node* rootNode = node_new_group("",-1);
	GNode* proot = g_node_new(rootNode);
	int n=groups->len;
	int i=0;

	for(;i<n;i++){
		Group* g=g_array_index(groups,Group*,i);
		Node* pfNode = node_new_group(g->name,-1);
		GNode* pfGNode = g_node_new(pfNode);
		if(1==g->type) {
			pfNode->type=2;
			pfNode->share=pfNode->ratio=g->share;
		}
		//find is there is a leaf group is this node
		GNode* fres=findParentInLeaves(proot,pfNode);
		if(NULL==fres){
			//insert this node as tree 1st level
			//ls_syslog(LOG_ERR,"Not find node %s\n",pfNode->name);
			g_node_insert(proot, -1, pfGNode);
			fres=pfGNode;
		}else{
			node_free(pfNode);
			g_node_destroy(pfGNode);
		}
		//insert all of this group members under fres
		int n=g->members->len;
		int i=0;
		for(;i<n;i++){
			Node* member=g_array_index(g->members,Node*,i);
			//ls_syslog(LOG_ERR,"insert member:%s\n",member->name);
			GNode* fres2=findNodeInLevelOfTree(proot,2,member);
			if(NULL==fres2){
				//if the member is not  the tree node of 1st level, insert it 
				g_node_insert_data(fres, -1, member);
			}else{
				//if the member is not the tree node of 1st level, move the tree under fres
				g_node_unlink(fres2);
				g_node_insert(fres, -1, fres2);
				node_free(fres2->data);
				fres2->data=member;
			}
		}	
	}
	
	*root=proot;
	return 0;
}

void printMapOfNode(gpointer key, gpointer value, gpointer user_data){
	char* name=key;
	GNode* gnode=value;
	Node* node=gnode->data;
	ls_syslog(LOG_DEBUG,"%s--%s  ", name, node->name);
}

static gboolean printNode(GNode *n,gpointer d){
	Node* data_g=n->data;
	ls_syslog(LOG_ERR,"%s:%s:%d:%.2f:%d:%.2f  ",data_g->type?"group":"user",data_g->name,data_g->ratio,data_g->share,data_g->numRunJob,data_g->priority);
	GHashTable* pdes=data_g->pDescendant;
	ls_syslog(LOG_DEBUG,"descendant:");
	g_hash_table_foreach(pdes, printMapOfNode, NULL);
	return FALSE;
}

void printTree(GNode *root){
	ls_syslog(LOG_ERR,"Print tree:");
	g_node_traverse(root, G_PRE_ORDER, G_TRAVERSE_ALL, -1, printNode, NULL);
	ls_syslog(LOG_ERR,"end tree");
}

static gboolean compareNode(GNode *n,gpointer d){
	Node* data_in=d;
	Node* data_g=n->data;
	if(data_g==NULL) return FALSE;
	if(strcmp(data_g->name,data_in->name)==0){
		tempFindGNode=n;
		return TRUE;
	}
	tempFindGNode=NULL;
	return FALSE;
}


GNode* findParentInLeaves(GNode* proot, Node* pfindNode){
	g_node_traverse(proot, G_POST_ORDER, G_TRAVERSE_ALL, -1, compareNode, pfindNode);
	return tempFindGNode;
}

GNode* findNodeInLevelOfTree(GNode* proot, int depth, Node* pfindNode){
	g_node_traverse(proot, G_POST_ORDER, G_TRAVERSE_ALL, depth, compareNode, pfindNode);
	return tempFindGNode;
}

void printGroups(GArray* groups){
	int n=groups->len;
	int i=0;
	// print all group members
	for(;i<n;i++){
		Group* g=g_array_index(groups,Group*,i);
		ls_syslog(LOG_ERR,"Name:%s\n",g->name);
		int m=g->members->len, j;
		for(j=0;j<m;j++){
			Node* n=g_array_index(g->members,Node*,j);
			ls_syslog(LOG_ERR,"    member:%s=%d\n",n->name,n->ratio);
		}
	}
}

Node* node_new_user(char* nodeName, int ratio){
	Node* n=g_new0(Node, 1);
	if(NULL!=nodeName){
		strcpy(n->name, nodeName);	
	}else{
		n->name[0] = 0;
	}
	n->isActive=1;
	n->ratio=ratio;
	n->type=0;
	n->share=-1;
	n->pDescendant=g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL);
	return n;
}

Node* node_new_group(char* nodeName, int ratio){
	Node* n=node_new_user(nodeName, ratio);
	n->type=1;
	return n;
}

void node_free(Node* node){
	g_hash_table_destroy(node->pDescendant);
	g_free(node);
	node=NULL;
}

static gboolean treeNodeFree(GNode *n,gpointer d){
	Node* data=n->data;
	node_free(data);
	return FALSE;
}


void tree_free(GNode* proot){
	if(NULL==proot) return;
	g_node_traverse(proot, G_POST_ORDER, G_TRAVERSE_ALL, -1, treeNodeFree, NULL);
	g_node_destroy(proot);
}

static gboolean resetNode(GNode *n,gpointer d){
	Node* data_g=n->data;
	data_g->numPendJob = 0;
	data_g->numRunJob = 0;
	data_g->priority = 0;
	data_g->updatedFlag = 0;
	data_g->isActive=1;
	return FALSE;
}

void visitHashMap(gpointer key, gpointer value, gpointer des){
	GHashTable* hm = des;
	g_hash_table_insert(hm, strdup(key), value);
}

void combineHashMap(GHashTable* des, GHashTable* src){
	g_hash_table_foreach(src, visitHashMap, des);
}

void resetTreeJobInfo(GNode *proot){
	g_node_traverse(proot, G_PRE_ORDER, G_TRAVERSE_ALL, -1, resetNode, NULL);
}

static void computeMapPriority(gpointer key, gpointer value, gpointer user_data){
	Capsule* pcap = user_data;
	Node* pUNode=(Node*)pcap->data1;
	GHashTable* phash=(GHashTable*)pcap->data2;
	
	char* name=(char*)key;
	UserJobInfo* pjob=g_hash_table_lookup(phash, name);
	if(NULL==pjob){
		ls_syslog(LOG_DEBUG,"Can not get job info of user %s.\n",name);
		return;
	}

	pUNode->numRunJob += pjob->numRunJob;
	pUNode->numPendJob += pjob->numPendJob;
	//ls_syslog(LOG_ERR,"pUNode->name:%s   pjob->numPendJob:%d\n",pUNode->name,pUNode->numPendJob);
}


static gboolean updateNode(GNode *pn, gpointer pd){
	Node* data=pn->data;
	GHashTable* phash=pd;
	
	if(!G_NODE_IS_LEAF(pn)){
		Capsule* pcap = g_new0(Capsule,1);
		pcap->data1=data;
		pcap->data2=phash;
		GHashTable* pDesc=data->pDescendant;
		g_hash_table_foreach(pDesc, computeMapPriority, pcap);
		data->isActive = data->numPendJob>0?1:0;
		g_free(pcap);
	}else{
		UserJobInfo* pjob=g_hash_table_lookup(phash, data->name);
		if(NULL==pjob){
			ls_syslog(LOG_DEBUG,"Can not get job info of user %s.\n",data->name);
			data->numRunJob=0;
			data->numPendJob=0;
			data->isActive=0;
		}else{
			data->numRunJob=pjob->numRunJob;
			data->numPendJob=pjob->numPendJob;
			data->isActive = data->numPendJob>0?1:0;
		}
	}
	computePriority(data);

	return FALSE;
}

/* update member of group with all defined in tree */
int updateMGA(GNode* proot){
	GHashTable* ga = getGroupAllTypeDefine();
	GNode* pGroups = proot->children;
	GNode* p=pGroups;
	while(NULL!=p){
		Node* d = p->data;
		if(2==d->type){
			//remove all members of all group
			GNode* pchildren=g_node_last_child(p);
			while(pchildren!=NULL){
				node_free(pchildren->data);
				GNode* temp = pchildren;
				pchildren=pchildren->prev;
				g_node_destroy(temp);
			}
			g_hash_table_remove_all(d->pDescendant);
			char** users=getAllUsersFromMBD();
			char** q=users;
			int sum=0;
			while(NULL!=*q){
				char* val=g_hash_table_lookup(ga, *q);
				if(NULL==val) val=g_hash_table_lookup(ga, "default");
				if(NULL==val) {
					ls_syslog(LOG_ERR, "Can not find the share of %s in lsb.users.\n",*q);
					exit(1);
				}
				int shareInt=atoi(val);
				Node* n=node_new_user(*q, shareInt);
				n->share=n->ratio;
				sum += n->share;
				GNode* pNewGNode=g_node_insert_data(p,-1,n);
				g_hash_table_insert(d->pDescendant, strdup(*q), pNewGNode);
				q++;
			}
			g_strfreev(users);
			d->share=sum;
			ls_syslog(LOG_DEBUG, "updateMGA:384, %s share is %d",d->name,d->share);
			//for all group, its ratio == its share, so no need compute again
			//computeNodeShare(p,NULL);
		}
		p=p->next;
	}
	return 0;
}

int updateTreeJobInfo(GNode* proot, GHashTable* phash){
	resetTreeJobInfo(proot);
	updateMGA(proot);
	g_node_traverse(proot, G_PRE_ORDER, G_TRAVERSE_ALL, -1, updateNode, phash);
	return 0;
}



