/*
 * FeeCalculator.cpp
 *
 *  Created on: 12 lug 2016
 *      Author: giovanni
 */

#include "FeeCalculator.h"
#include "PaymentChannel.h"
#include <iostream>

/*

FeeCalculatorOld::~FeeCalculatorOld() {
	// TODO Auto-generated destructor stub
}


double FeeCalculatorOld::getSendingFee(double payment, PaymentChannel & p, int A, int B){
	if (feeType==FIXED){
	}
	return baseFee;
}
*/

void FeeCalculatorOptimized::dump(){
	std::cout << "Basefee: " << baseFee << " \n";
	std::cout << "Slow:" << slow << " Shigh " << shigh << "\n";
}


std::vector<long> FeeCalculatorOptimized::getPoints(ln_units resFundsA, ln_units resFundsB){

	std::vector<long> points;

	points.push_back(0);

	if (resFundsA > resFundsB){
		points.push_back((resFundsA-resFundsB)/2);
	}

	points.push_back(resFundsA);

	return points;
}


std::vector<long> FeeCalculatorOptimized::getSlopes(ln_units resFundsA, ln_units resFundsB){

	std::vector<long> slopes;

	if (resFundsA > resFundsB){
		slopes.push_back(slow);
		slopes.push_back(shigh);
	} else
		slopes.push_back(shigh);

	return slopes;

}


millisatoshis FeeCalculatorOptimized::calcFee(millisatoshis P, PaymentChannel * pc,  bool reverse=false){

	ln_units fundsA,fundsB;

	fundsA = !reverse ? pc->getResidualFundsA() : pc->getResidualFundsB();
	fundsB = reverse ? pc->getResidualFundsA() : pc->getResidualFundsB();

	ln_units imbh = (fundsA - fundsB)/2;

	millisatoshis fee=0;

	if (fundsA > fundsB) {
		if ( P <= imbh )
			fee = baseFee + slow * P;
		else
			fee = baseFee + slow * imbh + (P-imbh) * shigh;
	} else
		fee = baseFee + P * shigh;


	return fee;
}


