/*
 * Copyright (c) 2000-2005 The Regents of The University of Michigan
 * Copyright (c) 2013 Advanced Micro Devices, Inc.
 * Copyright (c) 2013 Mark D. Hill and David A. Wood
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Steve Reinhardt
 *          Nathan Binkert
 */

/* @file
 * EventQueue interfaces
 */

//#ifndef __SIM_MYPRIORITY_HH__
//#define __SIM_MYPRIORITY_HH__

#include <algorithm>
#include <cassert>
#include <climits>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <string>
#include <map>


#include "sim/core.hh"
#include "sim/eventq_impl.hh"
#include "sim/mypriority.hh"

#include "base/flags.hh"
#include "base/types.hh"
#include "debug/MYSTATS.hh"
#include "debug/SimStatus.hh"
#include "sim/serialize.hh"

std::vector<int> perCoreReq;
std::vector<int> perCoreOutSReq;
//std::vector<int> perCoreOutstandingReq;
static int flag = 0;

//int perCoreReq[8];
//int * perCoreReq = new int(8);
static float T1 = 0;	//total requests serviced from group 1
static float T2 = 0;	//total requests serviced from group 2
static float O1 = 0;	//total outstanding requests present in the queue from group 1
static float O2 = 0;	//total outstanding requests present in the queue from group 2
float priRatio=0.111;
int numCores=8;
// For T1:T2. eg. if ratio is 8:2 => 8/2 = 4

uint64_t getPriority() {

//    DPRINTF(MYRENAME,"Status: 1: IQ full (ROB:IQ) %d\t%d \n",);
//	std::cout << curTick() << "\tgetPriority \t T1= " << T1 << " T2= " << T2 << "\n";
//	std::cout << "priority ratio is " << (T1/T2) << " (if < " << priRatio << " then return 0 else 1)\n";

    DPRINTF(SimStatus," getpriority: %d:%d:%f \n", T1, T2, float(T1/T2));

    int priGroup = -1;
	if(T1 <= 0) {
//		std::cout << "returning 1 as T1 = 0\n";
		priGroup = 1;
//		return 1;
	}
	else if(T2 <= 0) {
//		std::cout << "returning 0 as T2 = 0\n";
		priGroup = 0;
//		return 0;
	}
	else if(T1/T2 <= priRatio) {
//		std::cout << "returning 0\n";
		priGroup = 0;
//		return 0;
	}
	else {
		priGroup = 1;
	}

	return priGroup;
}

void setPerCoreData(uint64_t masterId, int opr) {

	if(flag == 0) {
		for(int i = 0; i < 8; i++) {
			perCoreReq.push_back(0);
			perCoreOutSReq.push_back(0);
		}
		flag = 1;
	}
	int a=5, b=6;
	int index = -1;
	if(masterId % 2 != 0) {
		index = (masterId - a)/4;
	}
	else {
		index = (masterId - b)/4;
	}
	if(opr == -1) {
		perCoreReq[index]++;
	}
	perCoreOutSReq[index]+=opr;
//	for(int i = 0; i < 8; i++) {
//		std::cout << "OutSCoreCount " << perCoreOutSReq[i] << "\n";
//	}
}

void setPriority(uint64_t masterId, int pri) {

//	if(T1 == 4144 || T2 == 33151) {
//		std::cout << curTick() << "\t print1 \t" << T1 << "\t" << T2 << "\t" << masterId << "\t" << pri << "\n";
//	}
	if (pri == 2 || pri == -2) {
		if(numCores == 8){
			if(masterId < 4 ){
				if(pri == -2) {
					T1++;
					O1 = O1 - 1;
					perCoreOutSReq[masterId] = perCoreOutSReq[masterId] - 1;
					perCoreReq[masterId]++;
				}
				else {
					O1 = O1 + 1;
					perCoreOutSReq[masterId] = perCoreOutSReq[masterId] + 1;
				}
			}
			else {
				if(pri == -2) {
					T2++;
					O2 = O2 - 1;
					perCoreOutSReq[masterId] = perCoreOutSReq[masterId] - 1;
					perCoreReq[masterId]++;
				}
				else {
					O2 = O2 + 1;
					perCoreOutSReq[masterId] = perCoreOutSReq[masterId] + 1;
				}
			}
		}
	}
	else {
	if(numCores == 8){
//		if(T1 == 4144 || T2 == 33151) {
//			std::cout << curTick() << "\t print2 \t" << T1 << "\t" << T2 << "\t" << masterId << "\t" << pri << "\n";
//		}
		if(masterId == 5 || masterId == 9 || masterId == 13 || masterId == 17 || masterId == 6 || masterId == 10 || masterId == 14 || masterId == 18){
			if(pri == -1) {
				T1++;
			}
			O1 = O1 + pri;
		}
		else {
			if(pri == -1) {
				T2++;
			}
			O2 = O2 + pri;
		}
//		if(T1 == 4144 || T2 == 33151) {
//			std::cout << curTick() << "\t print3 \t" << T1 << "\t" << T2 << "\t" << masterId << "\t" << pri << "\n";
//		}
	}

	else if(numCores == 4){
		if(masterId == 5 || masterId == 9 || masterId == 6 || masterId == 10){
			if(pri == -1) {
				T1++;
			}
			O1 = O1 + pri;
		}
		else {
			if(pri == -1) {
				T2++;
			}
			O2 = O2 + pri;
		}
	}
	else if(numCores == 2){
		if(masterId == 5 || masterId == 6){
			if(pri == -1) {
				T1++;
			}
			O1 = O1 + pri;
		}
		else {
			if(pri == -1) {
				T2++;
			}
			O2 = O2 + pri;
		}
	}

//	std::cout << curTick() << "\tMasterid " << masterId << "\t priority " << pri << "\t" << T1 << ":" << T2 << "\n";
	setPerCoreData(masterId, pri);
	}
	if(pri == 1)
	    DPRINTF(SimStatus," setpriority: %d:%d \t  %d:%d \n", T1, T2, O1, O2);	//temp printing
//		recordL3Status(curTick(), "globalL3Q", "Added", T1, T2, perCoreReq, O1, O2, perCoreOutSReq, -1);
	else
		recordL3Status(curTick(), "globalL3Q", "Deleted", T1, T2, perCoreReq, O1, O2, perCoreOutSReq, -1);

    DPRINTF(SimStatus," setpriority: %d:%d \t  %d:%d \n", T1, T2, O1, O2);
//	std::cout << curTick() << "\tsetPriority \tT1= " << T1 << "\tT2= " << T2 << "\n";
}

void retryingCore(uint64_t portId) {
//	std::cout << curTick() << "\t print4 \t" << T1 << "\t" << T2 << "\t" << portId << "\n";
	recordL3Status(curTick(), "globalL3Q", "Select", T1, T2, perCoreReq, O1, O2, perCoreOutSReq, portId);

}

//#endif // __SIM_MYPRIORITY_HH__
