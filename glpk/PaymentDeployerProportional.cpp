
#include "PaymentDeployerProportional.h"

#include <assert.h>

#include <iostream>
#include <fstream>
#include <tuple>

#include "../utils.h"

using namespace std;

int  PaymentDeployerProportional::RunSolver(std::vector<vector<long> > & flows, long & totalFee) {
	return 1;
}

int  PaymentDeployerProportional::RunSolver(std::vector<Tpath> & paths, long & totalFee) {

	int i, j;
	char commandString[1024], solStatus[1024];
	int nodeNum=numNodes;

	/* create temporary files */
	char * dataFile = tempnam(0, "DATA");
	char * outputFile = tempnam(0, "OUT");
	//char * clpModel = tempnam(0, "CLP");

	// open data file
	FILE *fpDat = fopen(dataFile, "w");
	if (fpDat == NULL) {
		cout << "failed to open file: " << dataFile << endl;
		return COULD_NOT_OPEN_DATA_FILE;
	}

	// create data file
	fprintf(fpDat, "\ndata;\n\n");

	// gateway set
	fprintf(fpDat, "set N:=");
	for (i = 0; i < nodeNum; i++) {
		fprintf(fpDat, " %d", i);
	}
	fprintf(fpDat, ";\n\n");

	// num paths to be used
	fprintf(fpDat, "param numPaths :=%d;\n", numPaths);


	// residual funds constraints
		fprintf(fpDat, "param r:=\n");
		for (i = 0; i < nodeNum; i++) {
			for (int j=0; j<nodeNum; j++){
				if (resFunds(i,j)>0)
					fprintf(fpDat, "[%d,%d] %lu ", i,j,resFunds(i,j));
			}
		}
		fprintf(fpDat, ";\n\n");

	// residual funds constraints
	/*fprintf(fpDat, "param r:\n");
	for (i = 0; i < nodeNum; i++) {
		fprintf(fpDat, "%d ", i);
	}
	fprintf(fpDat, ":=\n");

	// write down the r matrix
	for (i = 0; i < nodeNum; i++) {
		fprintf(fpDat, "%d ", i);
		for (j = 0; j < nodeNum; j++) {
			fprintf(fpDat, "%lu ", resFunds(i,j));
		}
		fprintf(fpDat, "\n");
	}
	fprintf(fpDat, ";\n\n");*/


		fprintf(fpDat, "param baseSendingFee:=\n");
		for (i = 0; i < nodeNum; i++) {
			for (int j=0; j<nodeNum; j++){
				if (resFunds(i,j)>0)
					fprintf(fpDat, "[%d,%d] %lu ", i,j,getBaseFee(i,j));
			}
		}
		fprintf(fpDat, ";\n\n");


	// base fees
		fprintf(fpDat, "param feerate_perkw:=\n");
		for (i = 0; i < nodeNum; i++) {
			for (int j=0; j<nodeNum; j++){
				if (resFunds(i,j)>0)
					fprintf(fpDat, "[%d,%d] %lu ", i,j,feerate_perkw(i,j));
			}
		}
		fprintf(fpDat, ";\n\n");


	fprintf(fpDat, "param lowbound :=\n");
	for (i = 0; i < nodeNum; i++) {
		for (j = 0; j < nodeNum; j++) {
			if (resFunds(i,j)==0 ) continue;

			fprintf(fpDat, "[%d,%d,*] ", i,j);
			for (int p=1; p <= numPaths; p++){
				fprintf(fpDat, " %lu %lu ", p, getLowerBound(i,j,p));
			}
		}
	}
	fprintf(fpDat, ";\n\n");

		fprintf(fpDat, "param upbound :=\n");
		for (i = 0; i < nodeNum; i++) {
			for (j = 0; j < nodeNum; j++) {
				if (resFunds(i,j)==0 ) continue;

				fprintf(fpDat, "[%d,%d,*] ", i,j);
				for (int p=1; p <= numPaths; p++){
					fprintf(fpDat, " %lu %lu ", p, getUpperBound(i,j,p));
				}
			}
		}
		fprintf(fpDat, ";\n\n");


		fprintf(fpDat, "param linkused :=\n");
		for (i = 0; i < nodeNum; i++) {
			for (j = 0; j < nodeNum; j++) {
				if (resFunds(i,j)==0 ) continue;

				fprintf(fpDat, "[%d,%d,*] ", i,j);
				for (int p=1; p <= numPaths; p++){
					fprintf(fpDat, " %lu %lu ", p, getLowerBound(i,j,p) > 0 ? 1 : 0);
				}
			}
		}
		fprintf(fpDat, ";\n\n");




	fprintf(fpDat, "\nparam source := %d;\n\n", source);
	fprintf(fpDat, "param destination := %d;\n\n", destination);
	fprintf(fpDat, "param P := %d;\n\n", payment);

	fprintf(fpDat, "end;\n\n");

	fclose(fpDat);

	sprintf(commandString, "glpsol --model %s --data %s -w %s",
					(modelsDirectory+string("/ModelProportional")).c_str(), dataFile, outputFile);
	cout << commandString << endl;
//	::system(commandString);

//	sprintf(commandString, "glpsol --model %s --data %s -w %s",
	//						"/home/giovanni/windows/workspace/PaymentChannelsSimulator/glpk/Model", dataFile, outputFile);
	//::system(commandString);


	string res=exec(commandString);
	return parseOutputFile(res, outputFile, paths, totalFee);
}


long PaymentDeployerProportional::getUpperBound(int i, int j, int pathN) {
	if (upperbound.find(pair<int,int>(i,j))!=upperbound.end()){
		return upperbound[pair<int,int>(i,j)][pathN-1];
	} else {
		return 0;
	}
}

long PaymentDeployerProportional::getLowerBound(int i, int j, int pathN) {
	if (lowerbound.find(pair<int,int>(i,j))!=lowerbound.end()){
		return lowerbound[pair<int,int>(i,j)][pathN-1];
	} else {
		return 0;
	}
}


long PaymentDeployerProportional::feerate_perkw(int i, int j) {
	if (fees.find(pair<int,int>(i,j))==fees.end())
			return 0;

	return fees[pair<int,int>(i,j)].coefficients[0];
}




void PaymentDeployerProportional::setLowerBound(int i, int j, int pathN, long l) {
	//lowerbound[pair<int,int>(i,j)].resize(pathN+1);

	if (lowerbound.find(pair<int,int>(i,j))==lowerbound.end())
		lowerbound[pair<int,int>(i,j)].resize(numPaths);

	lowerbound[pair<int,int>(i,j)][pathN-1] = l;
}


void PaymentDeployerProportional::setUpperBound(int i, int j, int pathN, long l) {
	//upperbound[pair<int,int>(i,j)].resize(pathN+1);
	if (upperbound.find(pair<int,int>(i,j))==upperbound.end())
		upperbound[pair<int,int>(i,j)].resize(numPaths);

	upperbound[pair<int,int>(i,j)][pathN-1] = l;


}


int  PaymentDeployerProportional::parseOutputFile(string glpk_output, string outputFile, std::vector<Tpath> & paths, long & totalFee) {

#ifdef DEBUG
		std::cout << glpk_output;
#endif

		std::size_t found=glpk_output.find("SOLUTION FOUND");
		if (found==std::string::npos){
				return PAYMENT_FAILED;
		}

#ifdef DEBUG
		cout << "Opening output file...\n";
		// open output file
#endif

		ifstream fpOut (outputFile);

		if (fpOut.is_open() == false) {
			cout << "failed to open file: " << outputFile << endl;
			return COULD_NOT_OPEN_OUTPUT_FILE;
		}

		string line;

		int rows;
		int columns;


	    getline (fpOut,line);
	    getline (fpOut,line);

		rows=convertTo(token(line,2));

	    getline (fpOut,line);

		columns=convertTo(token(line,2));

	    //cout << "COLUMNS " << columns << "\n";
		//cout << "RW " << rows << "\n";

		//assert(columns==numNodes*numNodes);

		//fscanf(fpOut, "%s", solStatus);
		//fscanf(fpOut, "%s", solStatus);
		//fscanf(fpOut, "%s", solStatus);

		getline (fpOut,line);
		getline (fpOut,line);
		getline (fpOut,line);

		double paying = convertToDouble(token(line,4));

		totalFee = round(paying) - payment;

		std::cout<<"Total fees " << totalFee << "\n";

		//go to the solutions
		//found=line.find("Column name");
		//while (found==std::string::npos){
		//	getline (fpOut,line);
		//	found=line.find("Column name");
		//}

		getline (fpOut,line);
		getline (fpOut,line);

		for (int i=0; i<rows; i++)
			getline (fpOut,line);


		double fVal;

		long flowsT[numPaths+1][numNodes][numNodes];
		memset(&flowsT,0,sizeof(flowsT));

		string line2;

		for (int i=0; i<numNodes; i++)
			for (int j=0; j<numNodes; j++)
				for (int p=1; p<=numPaths; p++){

				getline(fpOut, line);


				//if (line.size()<50){
				//				getline(fpOut, line2);
				//}

				if (resFunds(i,j)==0 || this->getUpperBound(i,j,p)==0) continue;

				//line = line + " " + line2;

				//std::cout << "LINE WITH SOLUTIONS " << line << "\n";
//				std::cout << "LINE WITH SOLUTIONS " << tokenize(line)[4] << "\n";

				fVal=convertToDouble(token(line,3));

				//if (fabs(fVal)>EPSILON){
				if (fVal>0)
					flowsT[p][i][j]=round(fVal);

#ifdef DEBUG
					if (flowsT[p][i][j]>0)
						fprintf(stdout, "flow[%d,%d,%d]=%lu\n",i,j, p, flowsT[p][i][j]);
#endif
					//std::cout << "flow["<<i<<","<<j<<"]="<<flow[i][j]<<  "\n";

				//} else {
				////	flow[i][j]=0;
				//}
				//fscanf(fpOut, "%s", solStatus);

		}

		for (int p=1; p<=numPaths;  p++){
			Tpath path;
			int curr=source;

			do {
				for (int i=0; i<numNodes; i++){
					if (flowsT[p][curr][i]!=0){
						//std::cerr << "adding " << curr << " " << i << "\n";
						path.push_back( pair<pair<int,int>,long>( pair<int,int>(curr,i), flowsT[p][curr][i]));
						curr=i;
						break;
					}
				}
			} while (curr!=destination);

			paths.push_back(path);
		}

		fpOut.close();
#ifdef DEBUG
		cout << "Finished parsing.\n";
#endif

		return 0;



}

