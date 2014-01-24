#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "mbd.h"
#include "ugtree.h"

extern int readConf(char* filename, GArray** groups);
extern int isShareDefined();
extern void destroyGroupAllMap();


GHashTable* getUserJobInfo(struct qData *qPtr);
static int initSharetree(GNode** pproot);
int updateAllQueueTree();

static GHashTable* ptreeMap=NULL;

int isTreeNone(){
	if(NULL==ptreeMap) return 1;
	guint n=g_hash_table_size(ptreeMap);
	if(0==n) return 1;
	return 0;
}

static gboolean changeNodeJobNum(GNode *pn, gpointer pd){
	if(G_NODE_IS_ROOT(pn)) return FALSE;
	Capsule* pcap = pd;
	char* user=pcap->data1;
	int num=*(int*)(pcap->data2);
	Node* pdata=pn->data;
	if(G_NODE_IS_LEAF(pn)){
		if(strcmp(user,pdata->name)==0){
			pdata->numRunJob+=num;
			pdata->numPendJob-=num;
			if(pdata->numPendJob<=0){
				pdata->isActive=0;
			}
			computePriority(pdata);
		}
	}else{
		GHashTable* pdesc=pdata->pDescendant;
		GNode* pnode=g_hash_table_lookup(pdesc, user);
		if(NULL!=pnode){
			//group node's inactive flag always is 1, so no need to set it.
			pdata->numRunJob+=num;
			pdata->numPendJob-=num;
			computePriority(pdata);
		}
	}
	
	return FALSE;
}


int changeJobNumOfUser(struct qData *qPtr, char* user, int num){
	GNode* proot = g_hash_table_lookup(ptreeMap, qPtr);
	Capsule* pcap = g_new0(Capsule,1);
	pcap->data1=user;
	pcap->data2=&num;
	g_node_traverse(proot, G_POST_ORDER, G_TRAVERSE_ALL, -1, changeNodeJobNum, pcap);
	g_free(pcap);
	return 0;	
}

int initAllQueueTree(){
	ptreeMap = g_hash_table_new(NULL, NULL);
	struct qData *qPtr;
	for (qPtr = qDataList->forw; qPtr != qDataList; qPtr = qPtr->forw) {
		if(qPtr->qAttrib & Q_ATTRIB_ROUND_ROBIN){
			ls_syslog(LOG_DEBUG, "init tree for queue:%s",qPtr->queue);
			GNode* proot = NULL;
			if(initSharetree(&proot)!=0) return -1;
			g_hash_table_insert(ptreeMap, qPtr, proot);
			//printTree(proot);
		}
	}
	return 0;
}

static int initSharetree(GNode** pproot){
	GNode* proot=NULL;
	GArray* pgroups=NULL;
	char lsbUserPath[1024]={0};
	sprintf(lsbUserPath, "%s/lsb.users", getenv("LSF_ENVDIR"));
	if(readConf(lsbUserPath,&pgroups)){
		ls_syslog(LOG_ERR, "Failed read group info from lsb.users.");
		exit(-1);
	}
	
	if(!isShareDefined()) return -1; 
	if(initTree(&proot, pgroups)){
		ls_syslog(LOG_ERR, "Failed to init tree.");
		exit(-1);
	}
	updateDescendantOfTreeNode(proot);
	setTopLevelGroupShare(proot);
	computeTreeShare(proot);
	*pproot=proot;
	return 0;
}

static void updateShareTree(gpointer key, gpointer value, gpointer user_data){
	struct qData *qPtrKey = key;
	GNode* proot = value;

	GHashTable* pjob=getUserJobInfo(qPtrKey);
	if(updateTreeJobInfo(proot, pjob)){
		ls_syslog(LOG_ERR, "Failed to update share tree with job info.");
		if(pjob != NULL){
			g_hash_table_destroy(pjob);
			pjob = NULL;
		}
		return;
	}
	if(pjob != NULL){
		g_hash_table_destroy(pjob);
		pjob = NULL;
	}
	//printTree(proot);
	return;
}

int updateAllQueueTree(){
	g_hash_table_foreach(ptreeMap, updateShareTree, NULL);
	return 0;
}


//select a leaf node with bigest priority, return it
GNode* compete(GNode* proot){
	GNode* p=proot->children;
	GNode* winner = NULL;
	while(NULL!=p){
		winner=p;
		GNode* c=p->next;
		while(NULL!=c){
			Node* cd=c->data;
			Node* wd=winner->data;
			if(cd->priority > wd->priority){
				winner=c;
			}
			c=c->next;
		}
		p=winner->children;
	}
	return winner;
}

static void competeMap(gpointer key, gpointer value, gpointer user_data){
	GHashTable* winMap = user_data;
	GNode* proot=value;
	GNode* winner=compete(proot);
	g_hash_table_insert(winMap, key, winner);
	if(winner!=NULL)
	{
		struct qData *qPtr = key;
		Node* data=winner->data;
		ls_syslog(LOG_DEBUG, "winnner--queue:%s,user:%s,priority:%f,run:%d,pend:%d",qPtr->queue,data->name,data->priority,data->numRunJob,data->numPendJob);
	}
}

GHashTable* competeAllQueues(){
	GHashTable* winMap = g_hash_table_new(NULL, NULL);
	g_hash_table_foreach(ptreeMap, competeMap, winMap);
	return winMap;
}


static void printMap(gpointer key, gpointer value, gpointer user_data){
	char* name=key;
	UserJobInfo* user=value;
	ls_syslog(LOG_DEBUG,"%s--%d", name, user->numRunJob);
}


void printJobInfo(GHashTable* ht){
	g_hash_table_foreach(ht, printMap, NULL);
}


GHashTable* getUserJobInfo(struct qData *qPtr){
	GHashTable* jobMap=g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_free);
	sTab hashSearchPtr;
    hEnt *hashEntryPtr;
    struct uData *uData;
	ls_syslog(LOG_DEBUG,"start getUserJobInfo for queue:%s", qPtr->queue);
	hashEntryPtr = h_firstEnt_(&uDataList, &hashSearchPtr);
    while (hashEntryPtr) {
        uData = (struct uData *) hashEntryPtr->hData;
		
        if (uData->flags & USER_OTHERS || uData->flags & USER_GROUP) {
            hashEntryPtr = h_nextEnt_(&hashSearchPtr);
            continue;
        }
		if(strcmp(uData->user,"default")==0){
			hashEntryPtr = h_nextEnt_(&hashSearchPtr);
			continue;
		}

		struct userAcct *uAcct = getUAcct(qPtr->uAcct, uData);
        UserJobInfo* user=g_new0(UserJobInfo,1);
		strcpy(user->name,uData->user);
		user->numRunJob = (NULL==uAcct?0:uAcct->numRUN) + (NULL==uAcct?0:uAcct->numUSUSP); 
		user->numPendJob = NULL==uAcct?0:uAcct->numPEND;
		g_hash_table_insert(jobMap,strdup(user->name), user);
		hashEntryPtr = h_nextEnt_(&hashSearchPtr);
		ls_syslog(LOG_DEBUG,"queue:%s,user:%s,running:%d,pending:%d", qPtr->queue, user->name, user->numRunJob, user->numPendJob);
    }
	return jobMap;
}


char** getAllUsersFromMBD(){
	sTab hashSearchPtr;
    hEnt *hashEntryPtr;
    struct uData *uData;
	GList* header = NULL;
	hashEntryPtr = h_firstEnt_(&uDataList, &hashSearchPtr);
	int i=0;
    while (hashEntryPtr) {
        uData = (struct uData *) hashEntryPtr->hData;

        if (uData->flags & USER_OTHERS || uData->flags & USER_GROUP) {
            hashEntryPtr = h_nextEnt_(&hashSearchPtr);
            continue;
        }
		if(strcmp(uData->user,"default")==0){
			hashEntryPtr = h_nextEnt_(&hashSearchPtr);
			continue;
		}
        header=g_list_prepend(header,strdup(uData->user));
        hashEntryPtr = h_nextEnt_(&hashSearchPtr);
		i++;
    }
	char** users=g_new0(char*, i+1);
	GList* p=header;
	i=0;
	while(NULL!=p){
		users[i++]=p->data;
		p=p->next;
	}
	g_list_free(header);
	return users;
}

static void destoryTree(gpointer key, gpointer value, gpointer user_data){
	GNode* proot=value;
	tree_free(proot);
}

void destroyQueueTree(){
	g_hash_table_foreach(ptreeMap, destoryTree, NULL);
	destroyGroupAllMap();
}



