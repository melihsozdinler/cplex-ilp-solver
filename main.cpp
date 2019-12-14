#include <iostream>
#include "defs.h"
#include <ilcplex/ilocplex.h>
#include <time.h>
#include <math.h>
#define CPLEX_DEBUG 0

enum EN_STATUS{
	EN_CPLEX_SUCCESS = 0,
	EN_CPLEX_NO_RESULT = 1
};

struct bipartiteNode{ // To store node
    int nodeId;
};
typedef struct bipartiteNode NODE;

struct bipartiteEdge{ // To store edge
    NODE sourceNodeId;
    NODE targetNodeId;
};
typedef struct bipartiteEdge EDGE;

double *solutionRows;

int solveByCplex( int K, int L, EDGE *edges, int V1, int V2, int E);


int checkStatus( std::vector<std::pair<int,bool> > in_vecCheck )
{
	if ( !in_vecCheck.empty() )
	{
		for ( int tmp_iCount = 0; tmp_iCount < in_vecCheck.size(); tmp_iCount++ )
		{
			if ( in_vecCheck[tmp_iCount].second == true )
			{
				cout << "[checkStatus] is " << in_vecCheck[tmp_iCount].first << endl;
				return in_vecCheck[tmp_iCount].first;
			}
		}
	}
	cout << "[checkStatus] is " << -1 << endl;
	return -1;
}

bool isAppendedAlrady( std::vector<std::pair<int,bool> > in_vecCheck, int in_iToCheck )
{
	cout << "[isAppendedAlrady] fuunction call" << endl;
	if ( !in_vecCheck.empty() )
	{
		for ( int tmp_iCount = 0; tmp_iCount < in_vecCheck.size(); tmp_iCount++ )
		{
			if ( in_vecCheck[in_iToCheck].first == in_iToCheck )
			{
				cout << "[isAppendedAlrady] is true" << endl;
				return true;
			}
		}
	}
	cout << "[isAppendedAlrady] is false" << endl;
	return false;
}


double diffclock(clock_t clock1,clock_t clock2)
{
	double diffticks=clock1-clock2;
	double diffms=(diffticks*10)/CLOCKS_PER_SEC;
	return diffms;
}

/**
	This main function reads from our file format and forms the cplex program
	based on sparse matrix representation. 
**/
int main(int argc, char *argv[])
{
	cout << "/**************************************************/" << endl;
	cout << "	       CPLEX'S RUN " 			 << endl;
	cout << "/**************************************************/" << endl;
	printf( "Program Begins\n" );
	FILE *fptr;
	int edgeNumber;
	int nodeNumberL,nodeNumberR;
	int read1,read2,count = 0;
	double solnTime = 0;

	int K_i = 10;
	int L_i = 10;
	long max = 0;
	int max_L = 0;
	int max_R = 0;
	//fptr = fopen( "experiment_60x60_100_0.100000_48_cplex.txt", "r" );
	//fptr = fopen( "input2.txt", "r" );
	//fptr = fopen("experiment_20x20_100_0.060000_30_cplex.txt", "r" );
	//fptr = fopen("experiment_20x20_100_0.060000_2_cplex.txt", "r" );
	//fptr = fopen( "my_cplex.txt", "r" );
	//fptr = fopen( "drug-target-Yildirim2007-s2-cplex.txt", "r" );
	//fptr = fopen( "drug-therapy-Nacher2008-s4-cplex.txt", "r" );

	clock_t begin=clock();
	/** 
		We read edges from a file format that we have. See samples inside inputs/ folder.
	**/
	fptr = fopen( argv[1], "r" );

	fscanf( fptr, "%d%d%d%d%d", &edgeNumber, &nodeNumberL, &nodeNumberR, &K_i, &L_i );
    cout << edgeNumber << " - " << nodeNumberL << " - " << nodeNumberR << " - " << K_i << " - " << L_i << endl;
	EDGE *edges = (EDGE *)malloc( sizeof( bipartiteEdge ) * (edgeNumber + 1) );
	
	NODE *sources = (NODE *)malloc( sizeof( bipartiteNode ) * (nodeNumberL + 1) );
	NODE *targets = (NODE *)malloc( sizeof( bipartiteNode ) * (nodeNumberR + 1) );

	// Init Sources
	for ( int i = 0; i < nodeNumberL; i++ ){
		(sources+i)->nodeId = 0;
	}

	// Init Targets
	for ( int i = 0; i < nodeNumberR; i++ ){
		(targets+i)->nodeId = 0;
	}
	
	// Reading the bipartite graph from file
	for( int i = 0; i < edgeNumber; i++ ){
		fscanf( fptr, "%d%d", &read1, &read2 );
		(edges+i)->sourceNodeId.nodeId = read1;
		(edges+i)->targetNodeId.nodeId = read2;
		(sources+read1)->nodeId++;
		(targets+read2)->nodeId++;

		if ( max_L < (sources+read1)->nodeId )
			max_L = (sources+read1)->nodeId;

		if ( max_R < (targets+read2)->nodeId )
			max_R = (targets+read2)->nodeId;
 		//if (i % 100 == 0)
			//cout << i << " - " << read1 << " - " << read2 << "-\n";
			//cout << i << " - " << (edges+i)->sourceNodeId.nodeId << " - " << (edges+i)->targetNodeId.nodeId << "-\n";
	}
	fclose( fptr );
	printf( "File Parsed\n" );
 	K_i = max_L;
 	L_i = max_R;

	//std::cout << max_L << " - " << max_R;
	std::cout << "Using binary search options" << std::endl;
	/*
	while( true ){
		std::cout << "**************************************" << std::endl;
		std::cout << "K:" << K_i << " - " << "L:" << L_i << std::endl;
		std::cout << "**************************************" << std::endl;
		int status = solveByCplex( K_i, L_i, edges, nodeNumberL, nodeNumberR, edgeNumber );
		if ( status != 1 ) {
			//env.error() << "Failed to optimize LP" << endl;
			//throw(-1);
			L_i = L_i * 2;
			if( L_i >= nodeNumberL )
				L_i = nodeNumberL - 1;
		}
		else{
			L_i = L_i / 2;
		}
		if( nodeNumberL <= L_i || L_i < nodeNumberR / 2){
			L_i = nodeNumberR / 2;
			K_i = K_i + 1;
		}
		if( K_i == nodeNumberR )
			break;
	}
	*/

	/* Algorithm 1 */
	/*
	while( L_i > 1 ){
		while( K_i > 1 ){
			std::cout << "**************************************" << std::endl;
			std::cout << "K:" << K_i << " - " << "L:" << L_i  << " - " << "Max:" << max <<std::endl;
			std::cout << "**************************************" << std::endl;
			int status = 0;
			vector<int> edgeSoln;
			vector<int> nodeLSoln;
			vector<int> nodeRSoln;
			
			if ( max < L_i * K_i )
			{
				status = solveByCplex( K_i, L_i, edges, nodeNumberL, nodeNumberR, edgeNumber);
				
				// Loop through the solution;
				// If there is more than one solution at the same size, CPLEX returns all
				if ( EN_CPLEX_SUCCESS == status && NULL != solutionRows )
				{
					for ( int variables = 0; variables < edgeNumber + nodeNumberL + nodeNumberR; variables++ )
					{
						cout << "Variable[" << variables << "] = " << solutionRows[variables] << endl;
						if ( solutionRows[variables] )
						{
							if ( variables < edgeNumber )
							{
									edgeSoln.push_back(variables);
							}
							else
							{
								if ( variables >= edgeNumber && variables < edgeNumber + nodeNumberL )
								{
									nodeLSoln.push_back(variables);
								}
								else
								{
									if ( variables >= edgeNumber + nodeNumberL && variables < edgeNumber + nodeNumberL + nodeNumberR )
									{
										nodeRSoln.push_back(variables);
									}
									else
									{
										cout << "Not possible case" << endl;
									}
								}	
							}
						}
					}
					
					cout << "Node R Solution" << endl;
					for ( int variables = 0; variables < nodeRSoln.size(); variables++ )
					{
						cout << nodeRSoln[variables] << "-";
					}

					cout << endl << "Node L Solution" << endl;
					for ( int variables = 0; variables < nodeLSoln.size(); variables++ )
					{
						cout << nodeLSoln[variables] << "-";
					}

					cout << endl << "Edge Solution" << endl;
					for ( int variables = 0; variables < edgeSoln.size(); variables++ )
					{
						cout << edgeSoln[variables] << "-";
					}
					
				}
				else
				{
					cout << "NULL variable or status is " << status << endl;
				}

				free( solutionRows );
				solutionRows = NULL;
			}
			std::cout << status << std::endl;
			if ( status != 1 ) {
				if ( max < L_i * K_i )
					max = L_i * K_i;
			}
			K_i--;
		}
		K_i = max_L;
		L_i--;
	}
	*/



	/* Algorithm 2 */


 	K_i = 10;
 	L_i = 10;

	int K_iBound = max_L;
	int L_iBound = max_R;

	bool t_bKMinReached = true;
	bool t_bLMinReached = false;

	int t_iMaxLSoln = 0;
	int t_iMaxKSoln = 0;
	
	long t_iMaxCurrent = 0;

	std::vector< std::vector< bool > > t_vecIsTried;
	std::vector< std::vector< bool > > t_vecIsSolution;
	std::vector< int > t_vecTriedMaxForK;
	std::vector< std::pair< int,int > > t_vecSolutionListKL;
	std::pair< int,int > t_pairMaxSolnKL;
	std::pair< int,int > t_pairLastSolnKL;
	std::pair< int,int > t_pairSoln1;
	std::pair< int,int > t_pairSoln2;

	int t_iTrialCount = 0;
	int t_iLastTrialResult = 0, t_iLastTrialResultInner = 0;
	int t_iKiCurrent = -1;
	int t_iLiCurrent = -1;

	std::string t_strLogs = "";
	char *tmp_strBuffer;

	std::vector< bool > tmp_vecDummy;
	int tmp_iDummy = -1;

	for(int tmp_iCount = 0; tmp_iCount <= K_iBound; tmp_iCount++ )
	{
		t_vecIsTried.push_back(tmp_vecDummy);
		t_vecIsSolution.push_back(tmp_vecDummy);
		t_vecTriedMaxForK.push_back(tmp_iDummy);
		for(int tmp_iCount2 = 0; tmp_iCount2 <= L_iBound; tmp_iCount2++ )
		{
			bool tmp_bDummy = false;
			t_vecIsTried[tmp_iCount].push_back(tmp_bDummy);
			t_vecIsSolution[tmp_iCount].push_back(tmp_bDummy);
		}
	}

	cout << "KBound " << K_iBound << " - " << "LBound" << L_iBound << endl;

	while( true ){
		std::cout << "**************************************" << std::endl;
		std::cout << "K:" << K_i << " - " << "L:" << L_i  << " - " << "Max:" << t_iMaxCurrent <<std::endl;
		std::stringstream t_stream;
		std::string tmp_strLogs;
		t_stream << "K:" << K_i;
		t_stream << "-";
		t_stream << "L:" << L_i;
		t_stream << "-" << "Max:" << t_iMaxCurrent;
		t_stream << "-" << "K_iBound:" << K_iBound;
		t_stream << "-" << "L_iBound:" << L_iBound;
		t_stream >> tmp_strLogs;
		t_strLogs += tmp_strLogs + "\n";
		std::cout << "**************************************" << std::endl;
		int status = 0;
		vector<int> edgeSoln;
		vector<int> nodeLSoln;
		vector<int> nodeRSoln;

		t_vecIsTried[K_i][L_i] = true;

		if ( t_iMaxCurrent < L_i * K_i )
		{
			status = solveByCplex( K_i, L_i, edges, nodeNumberL, nodeNumberR, edgeNumber);

			// Loop through the solution;
			// If there is more than one solution at the same size, CPLEX returns all
			if ( EN_CPLEX_SUCCESS == status && NULL != solutionRows )
			{
				for ( int variables = 0; variables < edgeNumber + nodeNumberL + nodeNumberR; variables++ )
				{
					//cout << "Variable[" << variables << "] = " << solutionRows[variables] << endl;
					if ( solutionRows[variables] )
					{
						if ( variables < edgeNumber )
						{
							edgeSoln.push_back(variables);
						}
						else
						{
							if ( variables >= edgeNumber && variables < edgeNumber + nodeNumberL )
							{
								nodeLSoln.push_back(variables);
							}
							else
							{
								if ( variables >= edgeNumber + nodeNumberL && variables < edgeNumber + nodeNumberL + nodeNumberR )
								{
									nodeRSoln.push_back(variables);
								}
								else
								{
									cout << "Not possible case" << endl;
								}
							}	
						}
					}
				}
				
				//cout << "Node R Solution" << endl;
				//for ( int variables = 0; variables < nodeRSoln.size(); variables++ )
				//{
				//	cout << nodeRSoln[variables] << "-";
				//}

				//cout << endl << "Node L Solution" << endl;
				//for ( int variables = 0; variables < nodeLSoln.size(); variables++ )
				//{
				//	cout << nodeLSoln[variables] << "-";
				//}

				//cout << endl << "Edge Solution" << endl;
				//for ( int variables = 0; variables < edgeSoln.size(); variables++ )
				//{
				//	cout << edgeSoln[variables] << "-";
				//}
			}
			else
			{
				cout << "NULL variable or status is " << status << endl;
			}

			free( solutionRows );
			solutionRows = NULL;

		}
		else
		{
			t_iLastTrialResultInner = 2;
		}

		t_iKiCurrent = K_i;
		t_iLiCurrent = L_i;

		t_vecIsTried[K_i][L_i] = true;
		t_iTrialCount++;

		std::cout << status << std::endl;

		std::pair<int,int> t_pairStat;

		if ( status != 1 )
		{
			if ( L_i > t_vecTriedMaxForK[K_i] )
			{
				t_vecTriedMaxForK[K_i] = L_i;
			}

			t_pairStat.first = K_i;
			t_pairStat.second = L_i;
			// Add this solution to bag for checks
			t_vecSolutionListKL.push_back(t_pairStat);
			t_pairLastSolnKL.first = K_i;
			t_pairLastSolnKL.second = L_i;
			t_vecIsSolution[K_i][L_i] = true;

			if ( t_iMaxCurrent < K_i * L_i )
			{
				t_iMaxCurrent = K_i * L_i;
				t_pairMaxSolnKL.first = K_i;
				t_pairMaxSolnKL.second = L_i;
			}
		}
		else
		{
			t_vecIsSolution[K_i][L_i] = false;
			// Add this non-solution to bag for checks
		}

		if ( t_vecIsSolution[K_i][L_i] )
		{
			// There is a solution guess the higher move
			cout << K_i << "," << L_i << " is in Solution" << endl;
			if ( L_i == L_iBound || t_vecIsTried[K_i][L_i - 1] == true  )
			//if ( L_i == L_iBound || t_vecIsTried[K_i][L_i - 1] == true || t_vecIsTried[K_i][L_i + 1] == true )
			{
				// That means no more solution for L_i
				if ( !t_vecSolutionListKL.empty() && K_i != K_iBound )
				{
					cout << "L_i is bound: " << L_i << ". Vector Size is " << t_vecSolutionListKL.size() << endl;
					std::vector< std::pair< int,int > >::iterator iterate = t_vecSolutionListKL.begin();
					int tmp_iMaxCurrentLiForKi = -1;
					// Find Max L_i value for current K_i
					long tmp_iMaxCurrent = -1;
					for(; iterate != t_vecSolutionListKL.end(); iterate++ )
					{
						t_pairSoln1 = (*iterate);
						if ( t_pairSoln1.first == K_i && tmp_iMaxCurrentLiForKi <  t_pairSoln1.second )
						{
							tmp_iMaxCurrentLiForKi = t_pairSoln1.second;
							t_pairSoln2 = t_pairSoln1;

							if ( tmp_iMaxCurrent < t_pairSoln1.second * t_pairSoln1.first )
							{
								tmp_iMaxCurrent = t_pairSoln1.second * t_pairSoln1.first;
							}
						}
					}

					if ( tmp_iMaxCurrentLiForKi == -1 )
					{
						cout << "No solution for K_i:" << K_i << " , " << "then decrease K_i" << endl;
						cout << "Last solution is [" << t_pairLastSolnKL.first << "," << t_pairLastSolnKL.second << "]" << endl;
						L_i = t_pairLastSolnKL.second;
						K_i = t_pairLastSolnKL.first + (K_iBound - t_pairLastSolnKL.first) / 2;
						cout << "Next solution to use [" << K_i << "," << L_i << "]" << endl;
					}
					else
					{
						cout << "There was solution for K_i:" << K_i << " , as L_i: " << t_pairSoln2.second << "then increase K_i" << endl;
						if ( tmp_iMaxCurrent < t_iMaxCurrent )
						{
							L_i = t_pairSoln2.second;
							int tmp_iMaxCurrentKi = -1;
							iterate = t_vecSolutionListKL.begin();
							for(; iterate != t_vecSolutionListKL.end(); iterate++ )
							{
								t_pairSoln1 = (*iterate);
								if ( t_pairSoln1.first < K_i && tmp_iMaxCurrentKi <  t_pairSoln1.first )
								{
									tmp_iMaxCurrentKi = t_pairSoln1.first;
									t_pairSoln2 = t_pairSoln1;
								}
								cout << " --- " << t_pairSoln1.first << "," << t_pairSoln1.second << " ---" << endl;
							}
							cout << "!!!!!!!!!!!!!  " << tmp_iMaxCurrentKi << " , " << t_iMaxCurrent << "!!!!!!!!!!!!!" << endl;
							if ( tmp_iMaxCurrentKi == - 1)
							{
								K_i = K_i - (K_i - 2) / 2;
							}
							else
							{
								K_i = K_i - (K_i - t_pairSoln2.first) / 2;
							}

						}
						else
						{
							L_i = t_pairSoln2.second;
							K_i = t_pairSoln2.first + (K_iBound - t_pairSoln2.first) / 2 + 1;
						}
						cout << "Next solution to use [" << K_i << "," << L_i << "]" << endl;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				cout << "L_i is not in bound: " << L_i << ". Vector Size is " << t_vecSolutionListKL.size() << endl;
				int tmp_iUpperBound = L_iBound;
				int tmp_iLastHighestTry = -1;
				// Check highest L_i value that is tried and make a range using that limit
				for (int tmp_iCount = L_i; tmp_iCount < t_vecIsTried[K_i].size(); tmp_iCount++ )
				{
					if ( t_vecIsTried[K_i][tmp_iCount] && tmp_iCount > L_i)
					{
						if ( tmp_iLastHighestTry < tmp_iCount )
						{
							tmp_iLastHighestTry = tmp_iCount;
							break;
						}
					}
				}
				if ( tmp_iLastHighestTry == -1)
				{
					//cout << ceilf((tmp_iUpperBound - L_i) / 2) << " - " << tmp_iUpperBound - L_i << " - " << (tmp_iUpperBound - L_i) / 2 << endl;
					L_i = L_i + (int)ceilf((float)((tmp_iUpperBound - L_i) / 2.0));
				}
				else
				{
					L_i = L_i + (int)ceilf((float)((tmp_iLastHighestTry - L_i) / 2));
				}

				cout << "Next solution to use [" << K_i << "," << L_i << "]" << endl;
			}
		}
		else
		{
			// There is no solution guess the lower move
			cout << K_i << "," << L_i << " is not in Solution" << endl;
			
			// First check limits
			if ( L_i == L_iBound || t_vecIsTried[K_i][L_i - 1] == true )
			//if ( L_i == L_iBound || t_vecIsTried[K_i][L_i - 1] == true || t_vecIsTried[K_i][L_i + 1] == true/ )
			{
				// That means no more solution for L_i
				if ( !t_vecSolutionListKL.empty() && K_i != K_iBound)
				{
					cout << "L_i is bound: " << L_i << ". Vector Size is " << t_vecSolutionListKL.size() << endl;
					std::vector< std::pair< int,int > >::iterator iterate = t_vecSolutionListKL.begin();
					int tmp_iMaxCurrentLiForKi = -1;
					// Find Max L_i value for current K_i
					long tmp_iMaxCurrent = -1;
					for(; iterate != t_vecSolutionListKL.end(); iterate++ )
					{
						t_pairSoln1 = (*iterate);
						if ( t_pairSoln1.first == K_i && tmp_iMaxCurrentLiForKi <  t_pairSoln1.second )
						{
							tmp_iMaxCurrentLiForKi = t_pairSoln1.second;
							t_pairSoln2 = t_pairSoln1;
							
							if ( tmp_iMaxCurrent < t_pairSoln1.second * t_pairSoln1.first )
							{
								tmp_iMaxCurrent = t_pairSoln1.second * t_pairSoln1.first;
							}
						}
					}

					if ( tmp_iMaxCurrentLiForKi == -1 )
					{
						cout << "No solution for K_i:" << K_i << " , " << "then decrease K_i" << endl;
						cout << "Last solution is [" << t_pairLastSolnKL.first << "," << t_pairLastSolnKL.second << "]" << endl;
						L_i = t_pairLastSolnKL.second;
						K_i = t_pairLastSolnKL.first - (t_pairLastSolnKL.first - 2) / 2;
						cout << "Next solution to use [" << K_i << "," << L_i << "]" << endl;
					}
					else
					{
						cout << "There was solution for K_i:" << K_i << " , as L_i: " << t_pairSoln2.second << "then increase K_i" << endl;
						if ( tmp_iMaxCurrent < t_iMaxCurrent )
						{
							L_i = t_pairSoln2.second;
							int tmp_iMaxCurrentKi = -1;
							iterate = t_vecSolutionListKL.begin();
							for(; iterate != t_vecSolutionListKL.end(); iterate++ )
							{
								t_pairSoln1 = (*iterate);
								if ( t_pairSoln1.first < K_i && tmp_iMaxCurrentKi <  t_pairSoln1.first )
								{
									tmp_iMaxCurrentKi = t_pairSoln1.first;
									t_pairSoln2 = t_pairSoln1;
								}
								cout << " --- " << t_pairSoln1.first << "," << t_pairSoln1.second << " ---" << endl;
							}

							cout << "!!!!!!!!!!!!!  " << tmp_iMaxCurrentKi << " , " << t_iMaxCurrent << "!!!!!!!!!!!!!" << endl;
							if ( tmp_iMaxCurrentKi == - 1)
							{
								K_i = K_i - (K_i - 2) / 2;
							}
							else
							{
								K_i = K_i - (K_i - t_pairSoln2.first) / 2;
							}

						}
						else
						{
							L_i = t_pairSoln2.second;
							K_i = t_pairSoln2.first + (K_iBound - t_pairSoln2.first) / 2 + 1;
						}
						cout << "Next solution to use [" << K_i << "," << L_i << "]" << endl;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				cout << "L_i is not in bound: " << L_i << ". Vector Size is " << t_vecSolutionListKL.size() << endl;
				int tmp_iLowerBound = 2;
				// Check highest L_i value that is tried and make a range using that limit
				for (int tmp_iCount = L_i; tmp_iCount < t_vecIsTried[K_i].size(); tmp_iCount-- )
				{
					if ( t_vecIsTried[K_i][tmp_iCount] && tmp_iCount < L_i)
					{
						if ( tmp_iLowerBound < tmp_iCount )
						{
							tmp_iLowerBound = tmp_iCount;
							break;
						}
					}
				}
				L_i = L_i - (L_i - tmp_iLowerBound ) / 2;

				// Added for 3x3 case of input2.txt
				if ( t_vecIsTried[K_i][L_i] )
				{
					cout << "Tried Before" << endl;
					K_i = K_i - 1;
				}
				else
				{
					cout << "Not Tried Before" << endl;
				}

				cout << "Next solution to use [" << K_i << "," << L_i << "]" << endl;
			}
		}

		if ( L_i < 2 || K_i < 2 || t_vecIsTried[K_i][L_i] )
			break;
		//system("pause");
	}
	
	cout << "MAX Solution is " << t_pairMaxSolnKL.first << " - " << t_pairMaxSolnKL.second << endl;

	clock_t end=clock();
	solnTime = double(diffclock(end,begin));

	fptr = fopen( "status.txt", "w" );
	fprintf( fptr, "%d\t%lf", 1, solnTime/*double(diffclock(end,begin))*/ );
	fclose( fptr );
	
	free(edges);
	free(sources);
	free(targets);

	return 0;
}

/**
	* solveByCplex function forms the cplex program based on sparse matrix representation,
	* using the edges structure obtained as bipartite graph.
	* @param K (int) : is a variable for the limitation of extracted bipartite graph Layer V1
	* @param L (int) : is a variable for the limitation of extracted bipartite graph Layer V2
	* @param edges (int) : is a structure to store bipartite graph
	* @param V1 (int) : is the number of vertices V1 in G
	* @param V2 (int) : is the number of vertices V2 in G
	* @param E (int) : is the number edges in G
**/
int solveByCplex( int K, int L, EDGE *edges, int V1, int V2, int E ){

#ifndef CPLEX_DEBUG
	printf( "CPLEX call Begins\n" );
#endif
	int      i, j, k, c, v ;
	int      ni, nh  ; 
	int      nrows, ncols, ncoeff ;
	int      flag    ;
	int 	 constraint, variables, allValues = 0;

	double   *obj = NULL;
	double   *rhs = NULL;
	char     *sense = NULL;
	int      *matbeg = NULL;
	int      *matcnt = NULL;
	int      *matind = NULL;
	double   *matval = NULL;
	char     *ctype = NULL;
	int       status = 0;
	int       solstat;
	double    objval;
	double   *x ;
	CPXENVptr  env = NULL;
	CPXLPptr   lp = NULL;
	
	obj = (double *) malloc((E + V1 + V2)*sizeof(double))  ;
	if (obj == NULL) { 
		printf("Memory alloc error\n") ; 
		exit(1) ; 
	}
	
	k = 0 ; 
#ifndef CPLEX_DEBUG
	printf( "Objective handling Begins\n" );
#endif
	//***************************
	// Objective handling begins
	//***************************
		variables = E + V1 + V2;
		for( i = 0 ; i < variables ; i++ ){
			if( k < E ){
				obj[ k ] = 1 ; // Edges must be in objective
			} 
			else{
				obj[ k ] = 0 ; // Vertices should not be in objective
			}
			k++;
		}
	//***************************
	// Objective handling ends
	//***************************
#ifndef CPLEX_DEBUG
	printf( "Objective handling Ends\n" );
#endif
	ncols = k ;
	nrows = V1 + V2 + E;

	std::vector<char> v_sense( nrows );
	std::vector<char> v_ctype( nrows );
	std::vector<double> v_rhs( nrows );
	std::vector<int> v_matbeg( nrows+1 );

	sense   = &v_sense[0];
	ctype   = &v_ctype[0];
	rhs     = &v_rhs[0];
	matbeg  = &v_matbeg[0];
	/*
	sense   = (char *)   malloc((nrows)*sizeof(char))    ;
	ctype	= (char *)   malloc((nrows)*sizeof(char))    ;
	rhs     = (double *) malloc((nrows)*sizeof(double))  ;
	matbeg  = (int *)    malloc((nrows+1)*sizeof(int))   ;
	*/

	ncoeff =  2 * E + E; // A^T v = 2 e
	ncoeff += 2 * E + E; // A e = s for V_1 and V_2

	std::vector<int> v_matind( ncoeff );
	std::vector<double> v_matval( ncoeff );
	matind  = &v_matind[0];
	matval  = &v_matval[0];
	/*
	matind  = (int *)    malloc((ncoeff)*sizeof(int))    ;
	matval  = (double *) malloc((ncoeff)*sizeof(double)) ;
	*/
	vector<list<int> > indexs( variables ); // size as number of constraints
	vector<int> lastIndex( variables ); // index will be filled using its last occurrance	
	vector<list<int> > elementsIndex( variables ); // size as number of constraints
	list<int>::iterator it;
#ifndef CPLEX_DEBUG
	printf( "Constraint handling Begins\n" );
#endif
	//***************************
	// Constraint handling begins
	//***************************
		// We just trying to form sparse matrix representation and its matind indexes.
		// 1. A^T v = 2 e
		constraint = 0;
		for( j = 0; j < E; j++ ){
// 			cout << j << " - " << (edges+j)->sourceNodeId.nodeId << " - " << (edges+j)->targetNodeId.nodeId  << endl;
			indexs[ constraint ].push_back( j );
			indexs[ constraint ].push_back( E + (edges+j)->sourceNodeId.nodeId );
			indexs[ constraint ].push_back( E + V1 + (edges+j)->targetNodeId.nodeId ); 
			constraint++;
		}
		// 1. done
		// 2. A e = s
		// begins for VL
		for( i = 0; i < V1; i++ ){
// 			int ifexist = 0;
// 			for( j = 0; j < E; j++ ){
// 				if( ( edges + j )->sourceNodeId.nodeId == i )
// 					ifexist = 1;
// 			}
// 			if( ifexist == 1 ){
				//c.add( expr - x[ i ] * K == 0); c[constraint].setName(labelConst);
				for( j = 0; j < E; j++ ){
					if( ( edges + j )->sourceNodeId.nodeId == i )
						indexs[ constraint ].push_back( j );
				}
				indexs[ constraint ].push_back( E + i );
// 			}
			constraint++;
		}
		// begins for VR
		for( i = 0; i < V2; i++ ){
// 			int ifexist = 0;
// 			for( j = 0; j < E; j++ ){
// 				if( ( edges + j )->targetNodeId.nodeId == i )
// 					ifexist = 1;
// 			}
// 			if( ifexist == 1 ){
				//c.add( expr - x[ i + V1 ] * L == 0); c[constraint].setName(labelConst);
				for( int j = 0; j < E; j++ ){
					if( ( edges + j )->targetNodeId.nodeId == i )
						indexs[ constraint ].push_back( j );
				}
				indexs[ constraint ].push_back( E + V1 + i );
// 			}
			constraint++;
		}
		// end
	//***************************
	// Constraint handling ends
	//***************************
#ifndef CPLEX_DEBUG
	printf( "Constraint handling Ends\n" );

	printf( "Constraint filling Begins\n" );
#endif
	//***************************
	// Constraint filling begins
	//***************************

		//***************************
		// Filling matind & matval 
		//***************************
		for( i = 0 ; i < variables ; i++ ){
			lastIndex[ i ] = 0;
		}

// 		for( i = 0; i < indexs.size(); i++ ){
// 			j = 0;
// 			cout << " S : " << indexs[ i ].size() << endl; 
// 			for ( it= indexs[ i ].begin() ; it !=  indexs[ i ].end(); it++ ){
// 				matind[ allValues ] = lastIndex[ *it ] ;
// 				if( i < E ){
// 					if( j == 0 ){
// 						matval[ allValues ] =  -2;
// 					}
// 					else{
// 						matval[ allValues ] =  1;
// 					}
// 				}
// 				else{
// 					if( i >= E && i < E + V1 ){
// 						if( j == 0 ){
// 							matval[ allValues ] =  -K;
// 						}
// 						else{
// 							matval[ allValues ] =  1;
// 						}
// 					}
// 					else{
// 						if( i >= E + V1 && i < E + V2 ){
// 							if( j == 0 ){
// 								matval[ allValues ] =  -L;
// 							}
// 							else{
// 								matval[ allValues ] =  1;
// 							}
// 						}
// 						else{
// 							;
// 						}
// 					}
// 				}
// 				allValues++;
// 				lastIndex[ *it ]++ ;
// 				j++;
// 			}
// 		}
		cout << indexs.size() << endl;
		for( v = 0; v < variables; v++ ){
			for( i = 0; i < indexs.size(); i++ ){
// 				cout << " S : " << indexs[ i ].size() << endl; 
				j = 0;
				for ( it= indexs[ i ].begin() ; it !=  indexs[ i ].end(); it++ ){
					if( v == *it ){
						matind[ allValues ] = lastIndex[ v ] ;
						if( i < E ){
							if( j == 0 ){
								matval[ allValues ] =  -2;
							}
							else{
								matval[ allValues ] =  1;
							}
						}
						else{
							if( i >= E && i < E + V1 ){
								if( j == indexs[ i ].size() - 1 ){
									matval[ allValues ] =  -1 * K;
// cout <<  matval[ allValues ] << " : " << allValues << " TRY\n";
								}
								else{
									matval[ allValues ] =  1;
								}
							}
							else{
								if( i >= E + V1 && i < E + V1 + V2 ){
									if( j == indexs[ i ].size() - 1 ){
										matval[ allValues ] =  -1 * L;
// cout <<  matval[ allValues ] << " : " << allValues << " TRY\n";
									}
									else{
										matval[ allValues ] =  1;
									}
								}
								else{
									;
								}
							}
						}
						allValues++;
					}
					j++;
				}
				lastIndex[ v ]++ ;
			}
		}
		//***************************
		// Filling matbag 
		//***************************
		matbeg[ 0 ] = 0;
		for( i = 1; i < indexs.size(); i++ ){
			matbeg[ i ] = indexs[ i - 1 ].size() + matbeg[ i - 1 ];
		}
		matbeg[ i ] = indexs[ i - 1 ].size() + matbeg[ i - 1 ];
		//***************************
		// Filling sense 
		//***************************
		for( i = 0; i < indexs.size(); i++ ){
			if( i < E )
				sense[ i ] = 'G';
			else
				sense[ i ] = 'E';
		}
		//***************************
		// Filling ctype 
		//***************************"
		for( i = 0; i < indexs.size(); i++ ){
			ctype[ i ] = 'B';
		}
		//***************************
		// Filling rhs 
		//***************************
		for( i = 0; i < indexs.size(); i++ ){
			rhs[ i ] = 0;
		}
	//***************************
	// Constraint filling ends
	//***************************
#ifndef CPLEX_DEBUG
	printf( "Constraint filling Ends\n" );

	printf( "Solving CPLEX\n" );
#endif
	env = CPXopenCPLEX (&status);
	
// 	cout << "Number of Constraints :" << indexs.size() << endl;
// 	for( i = 0; i < indexs.size(); i++ ){
// 		cout << matbeg[ i ] << " : " << sense[ i ] << " : " << ctype[ i ] << " : " << rhs[ i ] << endl;
// 	}

// 	cout << " Matval and Matindex" << endl;
// 	for( i = 0; i < ncoeff; i++ ){
// 		if( i % ( 2 * E + E ) == 0 )
// 			cout << "_______________________\n";
// 		cout << matval[ i ] << " : " << matind[ i ] << endl;
// 	}

	if ( env == NULL ) {
		char  errmsg[ 1024 ];
		fprintf (stderr, "Could not open CPLEX environment.\n");
		CPXgeterrorstring ( env, status, errmsg);
		fprintf (stderr, "%s", errmsg);
		exit( 1 ) ;
	}
	
	status = CPXsetintparam ( env, CPX_PARAM_SCRIND, CPX_ON);
	if ( status ) {
		fprintf ( stderr, "Failure to turn on screen indicator, error %d.\n", status );
		exit( 2 ) ;
	}
	
	lp = CPXcreateprob ( env, &status, "cplex" );
	if ( lp == NULL ) {
		fprintf ( stderr, "Failed to create LP.\n" );
		exit( 3 ) ; 
	}
	
	CPXchgobjsen ( env, lp, CPX_MAX ) ; 

	status = CPXnewcols ( env, lp, ncols, obj, NULL, NULL, ctype, NULL ) ; 
	if ( status ) {
		fprintf ( stderr, "Error in CPXnewcols.\n" );
		exit( 4 ) ; 
	}
	
	status = CPXaddrows( env, lp, 0, nrows, matbeg[nrows], rhs, sense, matbeg, matind, matval, NULL, NULL );
	if ( status ) {
		fprintf ( stderr, "Error in CPXcopyctype\n" );
		exit( 5 ) ; 
	}

	CPXsetdblparam(env, CPX_PARAM_CUTSFACTOR, 0);   // Row multiplier factor for cuts
	CPXsetintparam(env, CPX_PARAM_CLIQUES, 2);     // Cliques
	CPXsetintparam(env, CPX_PARAM_NODELIM , 25000); 
	CPXsetintparam(env, CPX_PARAM_INTSOLLIM, 10 );
	CPXsetintparam(env, CPX_PARAM_MIPDISPLAY, 0 ); // for logs
	
	/* Optimize the problem and obtain solution. */
	status = CPXmipopt ( env, lp );
	if ( status ) {
		fprintf ( stderr, "Failed to optimize MIP.\n" );
		exit(8) ; 
	}
	
	solstat = CPXgetstat ( env, lp );

	/* Write the output to the screen. */
#ifndef CPLEX_DEBUG
	printf ( "\nSolution status = %d\n", solstat );
#endif
	status = CPXgetmipobjval ( env, lp, &objval );
	if ( status ) {
		fprintf ( stderr,"No MIP objective value available.  Exiting...\n" );
		exit( 11 ); 
	}
#ifndef CPLEX_DEBUG
	printf ("Solution value  = %f\n\n", objval);

	printf( "CPLEX call Ends\n" );
#endif
	if( objval > 0.0 ){
		
		/*
			for( i = 0 ; i < variables ; i++ ){
				cout <<	matval[ i ] << "-";
				k++;
			}

			cout <<	endl;

			for( i = 0 ; i < variables ; i++ ){
				cout <<	matind[ i ] << "-";
				k++;
			}
		*/

		/* The size of the problem should be obtained by asking CPLEX what
		the actual size is, rather than using what was passed to CPXcopylp.
		cur_numrows and cur_numcols store the current number of rows and
		columns, respectively.  */

		int cur_numrows, cur_numcols;

		cur_numrows = CPXgetnumrows (env, lp);
		cur_numcols = CPXgetnumcols (env, lp);

		solutionRows = (double *)malloc(sizeof(double) * cur_numrows);
		double *slack = (double *)malloc(sizeof(double) * cur_numcols);

		status = CPXgetmipx (env, lp, solutionRows, 0, cur_numcols-1);
		if ( status ) {
			fprintf (stderr, "Failed to get optimal integer x.\n");
		}

		
		status = CPXgetmipslack (env, lp, slack, 0, cur_numrows-1);
		if ( status ) {
			fprintf (stderr, "Failed to get optimal slack values.\n");
		}

		/*
		for (i = 0; i < cur_numrows; i++) {
			printf ("Row %d:  Value = %10f\n", i, rows[i]);
		}
		*/

		/*
		for (i = 0; i < cur_numrows; i++) {
			printf ("Row %d:  Value = %10f\n", i, solutionRows[i]);
		}
		*/

		//system("pause");

		/*
		for (j = 0; j < cur_numcols; j++) {
			printf ("Column %d:  Slack = %10f\n", j, slack[j]);
		}
		*/

		//system("pause");
	}
	else
	{
		solutionRows = NULL;
	}

	
	status = CPXwriteprob (env, lp, "myprob.lp", NULL);
	if ( status ) {
		printf ("Failed to write LP to disk.\n");
	}

	if( objval > 0.0 ){
		cout << K << "\t-\t" << L << "\t exists in the bipartite graph\n";
		return 0;
	}
	else{
		return 1;
	}
}
