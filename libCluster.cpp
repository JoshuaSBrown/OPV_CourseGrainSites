#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "cluster.h"

using namespace std;

#ifdef _T_
#define Var 1
#else
#define Var 0
#endif

static int clusterIdCounter = 0;
static int thresh = 0;

int setThresh(int n){
    if(n>0){
        thresh=n;
        if(Var==1) cout<<"Thresh hold is: "<<thresh<<endl;
        return 1;
    }else{
        if(Var==1) cerr<<"ERROR in setThresh"<<endl;
        return 0;
    }
}

cluster::cluster(int siteId1,int * neighId1, double * neighRates1, int sizen1, int visitFreq1, int siteId2, int * neighId2, double * neighRates2, int sizen2, int visitFreq2){
	clusterId=clusterIdCounter;
	clusterIdCounter++;
	if(potentialCluster(visitFreq1,visitFreq2)){
			siteInCluster[0]= new site(siteId1,neighRates1,neighId1,sizen1,visitFreq1);
			siteInCluster[1]= new site(siteId2,neighRates2,neighId2,sizen2,visitFreq2);
                        }
}

cluster::~cluster(){
}
/*
int testCluster(){
	printf("potentialCluster(10,-10,10): (-1) %d\n",potentialCluster(10,-10));
	printf("potentialCluster(10,6,8): (0) %d\n",potentialCluster(10,6,8));
	printf("potentailCluster(10,10,5): (1) %d\n",potentialCluster(10,10,5));
	printf("clusterOrSite(-1,-1): (1) %d\n",clusterOrSite(-1,-1));
	printf("clusterOrSite(-10,2): (-1) %d\n",clusterOrSite(-10,2));
	printf("clusterOrSite(1,-1): (2) %d\n",clusterOrSite(1,-1));
	printf("clusterOrSite(1,1): (3) %d\n",clusterOrSite(1,1));
	return 0;
}

*/

int cluster::potentialCluster(int visitFreq1, int visitFreq2){
	if(visitFreq1<0||visitFreq2<0||thresh<0){
		fprintf(stderr,"ERROR in potentialCluster\n");//find a way to turn off in preprocessor
		return -1;
	}
	if((visitFreq1 > thresh) && (visitFreq2 > thresh))
		return 1;
	return 0;
}

/*
int cluster::clusterOrSite(int clusterId1, int clusterId2){
	if(clusterId1<-1||clusterId2<-1)
		return -1;
	if(clusterId1 == -1 && clusterId2 == -1)
		return 1;
	if(clusterId1 == -1 || clusterId2 == -1)
		return 2;
	return 3;
}

int cluster::neighSiteCluster(int neighIds1[], int * neighIds2, int * neighCluster){
}
*/
