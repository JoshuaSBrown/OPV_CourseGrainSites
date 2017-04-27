#ifndef CLUSTER_H_
#define CLUSTER_H_

struct site{
	int siteId;
	double * neighRates;
	int * neighIds;
	int visitFreq;

	site(){};//default constructor

	//constructor
	site(int sId, double* nRates, int* nId, int sizenId, int vFreq){
		//error handling?
		siteId=sId;
		visitFreq=vFreq;
		for(int i =0;i<sizenId;i++){
			neighRates[i]=nRates[i];
			neighIds[i]=nId[i];
			i++;
		}
	}
};

class cluster {
	public:
		cluster(int siteId1, int * neighIds1,double * neighRates1, int sizeNeighId1,int visitFreq1, int siteId2, int * neighIds2,double * neighRates2, int sizenId2, int visitFreq2); //constructor  passed certain values? site ids the rates
		~cluster(); //deconstructor
		// test function and simulation fuction

//		int testCluster(void);
//runs all test cases of every function, prints out correct output and what the function prints like such f: (correct) real
//any errors will be reported in the error stream with the funciton call

	private:
		int clusterId;
		int visitFreqCluster;
		//all other funtions
		site * siteInCluster[100];
		int potentialCluster(int visitFreq1, int visitFreq2);
//INPUT: the visitation frequency of site 1 and of site 2 and the threshold of what determines a cluster
//OUTPUT: -1 if mal-input or error, 0 if not a cluster, 1 if a cluster

//		int clusterOrSite(int clusterId1, int clusterId2);
//INPUT: the cluster Id of one site, the cluster Id of another site
//OUTPUT: -1 if mal-input or error, 1 if site to site interaction, 2 if site-cluster, 3 if cluster-cluster

//		int neighSiteCluster(int * neighIds1, int * neighIds2, int * neighCluster);
//INPUT: neighbors ids of sites 1 and 2, an array to write all the neighbors to
//OUTPUT: -1 if mal-input or error, 1 if successful

//calculate site ratio given hop rates off site, need list of neighbors for that site, hop rates to the neigh form site, list of sites Ids in cluster
//pass maybe cluster struct and return array of site ratios of the sites hop off/hop on see matlab file
//
//fn 2 determine sites in cluster and return list of id sites in cluster,
//
//struct cluster of 2 lsits ids in cluster and neighbors, int id of cluster, int # of sites in cluster, int # of neighbors
//
//create a repository and error handeling
//
//dwell time 1/sum of rates; list of neighbor rates; return double
//
//calculate pvals for site rate(#)/sum of rates site #; list of neighbor hop rates, out put an array
//
//calculate p hop off given lisst of neigh rates for a site, output an array
//
//
//convergence
//	1. prob on site is 1/(number of sites in cluster)
//	2. site 1 eg (prob hope to site 1)*(prob on site 2)+(prob hppe to site 1)(prob on site 5)
//	3.Total (new site prob)     total=site1prob+site2prob...
//	4.probsite1=site1prob/total + probsite1 ....
//	5.normalise probonsite1=probonsite1/sum(probs on site #)
//
//calculate total prob off cluster to a given neighbor n
//	1. sum hop rates off the cluster
//	2. calc prob hop to neigh off cluster ex for neigh1 prob neigh=dwellofsite(1)/sum(dwell + probonsite1*(rate from site1 to neigh1/tot    tot =sum of hop rates off
//
//note: if charge is within cluster, most likeyl on which cluster, where most likely to jump off cluster to which site?, dwell time on cluster as single site
//
//
//prob to hop off from a given site in cluster n
//hopoff1 = ProbOnSite1 * Dwell1/sum(dwell) * rate from 1to neighbor n + probonsite1*dwell/sum(dwell)*rate from site1toneigh m
//then normalize hopoff[i]/sum(hopoff)
//
//escape time off cluster
//	1. prob hop off site i time =1/(rate to neigh n + rate to neigh m)
//	escape time = site1hop off time * hop off site 1+...
//
//Tprob_off cluster or escape cluster = tescapsesite1*probhopoffsite1+...
//
//write sample simluation, see test case in matlab
//look at time for maximum efficiency time.h, finde standard deviation and overlap run for multiple interations and find average function time
//
//site pval on and off cluster see matlab code
//
//resolution = arbitray (20)
//
//simluation set up, return time(see matlab), what site it jumped to
//
//
//write test function in library
//
//See matlab code for details
//look to simpligy the equations
//
//pvals internal,
//
//Must id sites, neighbor rates, neighbor ids
//
//also passed visitation freqs and thresh
//
//class data struct sites in cluster (arrays) uses static
// with arrays or linked lists  first elemetn cluster id
//
//time off(tprob off), id which it jumps to
};

int setThresh(int n);
// sets thresh as a static for all functions, not in class cluster

// test function and simulation fuction

//int testCluster(void);
//runs all test cases of every function, prints out correct output and what the function prints like such f: (correct) real
//any errors will be reported in the error stream with the funciton call


#endif
