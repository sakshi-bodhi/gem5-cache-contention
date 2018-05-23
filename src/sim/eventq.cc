/*
 * Copyright (c) 2000-2005 The Regents of The University of Michigan
 * Copyright (c) 2008 The Hewlett-Packard Development Company
 * Copyright (c) 2013 Advanced Micro Devices, Inc.
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
 *          Steve Raasch
 */

#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>

#include "base/misc.hh"
#include "base/trace.hh"
#include "cpu/smt.hh"
#include "debug/Checkpoint.hh"
#include "debug/PktTrace.hh"
#include "sim/core.hh"
#include "sim/eventq_impl.hh"

using namespace std;
Tick simQuantum = 0;

//
// Main Event Queues
//
// Events on these queues are processed at the *beginning* of each
// cycle, before the pipeline simulation is performed.
//
uint32_t numMainEventQueues = 0;
vector<EventQueue *> mainEventQueue;
__thread EventQueue *_curEventQueue = NULL;
bool inParallelMode = false;

//---------CHANGED-----------

std::ofstream outfile;

uint64_t array_cycle[30000];
int32_t array_qLen[30000];
uint64_t array_length = 30000;
std::string strBuf;

uint64_t reqIdCounter = 0;
std::map<uint64_t, CycleVector> cycleMap;
std::map<uint64_t, PacketInfo> map1;
std::map<uint64_t, PacketDelayInfo> map2;
std::map<uint64_t, uint64_t> l3busDelay;
std::map<uint64_t, uint64_t> l2busDelay;
std::map<uint64_t, uint64_t> membusDelay;
//std::map<uint64_t, uint64_t> membusDelayR;
//std::map<uint64_t, uint64_t> l3busDelayR;
//std::map<uint64_t, uint64_t> l2busDelayR;

std::map<uint64_t, uint64_t> icacheDelay;
std::map<uint64_t, uint64_t> dcacheDelay;
std::map<uint64_t, uint64_t> l3Delay;
std::map<uint64_t, uint64_t> l2Delay;
std::map<uint64_t, uint64_t> memRWQDelay;
std::map<uint64_t, uint64_t> memRespQDelay;
std::map<uint64_t, uint64_t> memAccessTime;
std::map<uint64_t, uint64_t> memRespTime;
std::map<std::string, qLength> qLenStat;
std::map<std::string, qLength> qLenStatL1i;
std::map<std::string, qLength> qLenStatL1d;
std::map<std::string, qLength> qLenStatL2;
std::map<std::string, qLength> qLenStatL3;
//std::map<uint64_t, uint64_t> l3DelayR;
//std::map<uint64_t, uint64_t> l2DelayR;
//std::map<uint64_t, uint64_t> icacheDelayR;
//std::map<uint64_t, uint64_t> dcacheDelayR;

//std::vector<PacketDelayPathInfo> DPInfo;
int counterL3 = 500;
int counterL2 = 500;
int counterMembus = 500;
//int counterL3R = 500;
//int counterL2R = 500;
//int counterMembusR = 500;

uint64_t totalIcacheRequestsL3 = 0;
TotalStats totalL3;
TotalStats totalL3Hit;
TotalStats totalL3Bus;

uint64_t totalIcacheRequestsL2 = 0;
TotalStats totalL2;
TotalStats totalL2Hit;
TotalStats totalL2Bus;

TotalStats totalL1;
TotalStats totalL1Hit;

TotalStats totalMem;
TotalStats totalMemRWQ;
TotalStats totalMemRespQ;
TotalStats totalMemAccess;
TotalStats totalRespTime;

uint64_t totalDelay = 0;
uint64_t totalRequests = 0;
uint64_t totalRequestsFinished = 0;

TotalStats avgQTime;
TotalStats avgQTimeL1i;
TotalStats avgQTimeL1d;
TotalStats avgQTimeL2;
TotalStats avgQTimeL3;
TotalStats avgGlobalQSize;
TotalStats avgThroughput;
std::map<string, TotalStats> avgQSize;
bool firstThroughput = false;	//to initialize the avgThroughput.numEntry
int i = 0;
int j = 0;
int k = 0;


//uint64_t totalIcacheRequestsL3 = 0;
//uint64_t totalRequestsL3 = 0;
//uint64_t totalDelayL3 = 0;
//uint64_t totalL3HitDelay = 0;
//uint64_t totalL3Hit = 0;
//uint64_t totalL3BusDelay = 0;
//uint64_t totalL3BusEntry = 0;
//
//uint64_t totalIcacheRequestsL2 = 0;
//uint64_t totalRequestsL2 = 0;
//uint64_t totalDelayL2 = 0;
//uint64_t totalL2HitDelay = 0;
//uint64_t totalL2Hit = 0;
//uint64_t totalL2BusDelay = 0;
//uint64_t totalL2BusEntry = 0;
//
//uint64_t totalRequestsL1 = 0;
//uint64_t totalDelayL1 = 0;
//uint64_t totalL1HitDelay = 0;
//uint64_t totalL1Hit = 0;
//
//uint64_t totalRequestsMem = 0;
//uint64_t totalDelayMem = 0;
//uint64_t totalMemRWQDelay = 0;
//uint64_t totalRWQEntry = 0;
//uint64_t totalMemRespQDelay = 0;
//uint64_t totalMemRespQEntry = 0;
//uint64_t totalMemAccessDelay = 0;
//uint64_t totalMemAccessEntry = 0;
//uint64_t totalRespTDelay = 0;
//uint64_t totalRespTEntry = 0;

uint64_t totalCyclesBusyL3 = 0;
uint64_t lastActiveCycleL3 = 0;
uint64_t totalWaitingForL3 = 0;

//data structures
std::map<int, coreStatusVector> completeStatus;
std::map<int, L3QStatusVector> L3QCompleteStatus;

std::vector<coreStatus> last_entry_vector;

//variables
bool recordStatusFlag = false;
bool recordL3StatusFlag = true;

uint64_t writeAfter = 1000;
uint64_t writeAfter1 = 1000;


void printStatusMap(std::map<int, coreStatusVector> &statusMap) {

	std::cout << "statusMap size- " << statusMap.size() << "\n";
	typedef std::map<int, coreStatusVector>::const_iterator MapIterator;
	for (MapIterator iter = statusMap.begin(); iter != statusMap.end(); iter++)
	{
	    std::cout << "Core: " << iter->first << " has " << iter->second.vec.size() << " entries\n";
	    for(std::vector<coreStatus>::const_iterator it = iter->second.vec.begin(); it != iter->second.vec.end(); ++it) {
	    	std::cout << it->tick << "\t";
	    	std::cout << it->fetch_status << ":" << it->fetch_overall_status << "\t";
	    	std::cout << it->decode_status << ":" << it->decode_overall_status << "\t";
	    	std::cout << it->rename_status << ":" << it->rename_overall_status << ":" << it->rename_reason_to_block << "\t";
	    	std::cout << it->dispatch_iew_status << ":" << it->exe_iew_status << ":" << it->wb_iew_status << ":" << it->iew_overall_status << "\t";
	    	std::cout << it->commit_status << ":" << it->commit_overall_status << ":" << it->commit_head_instruction << "\t";
	    	std::cout << it->l1_mshr_total << ":" << it->l1_mshr_ready << "\t";
	    	std::cout << it->l2_mshr_total << ":" << it->l2_mshr_ready << "\t";
	    	std::cout << it->l3_mshr_total << ":" << it->l3_mshr_ready << "\n";
	    }
	}
}

std::ofstream outfileptr;
std::string filename = "./results/core";
//bool firstAccessToFile=true;

void writeFile(std::map<int, coreStatusVector> &statusMap) {
//stringStream.open(filename+".txt", std::ios_base::app);
typedef std::map<int, coreStatusVector>::const_iterator MapIterator;
	for (MapIterator iter = statusMap.begin(); iter != statusMap.end(); iter++)
	{
		std::ofstream stringStream;
//		std::string str1;
//		std::ostringstream stringStream;
		int core = iter->first;
		std::string file = filename + std::to_string(core) + ".txt";
		stringStream.open(file, std::ios_base::app);

		for(std::vector<coreStatus>::const_iterator it = iter->second.vec.begin(); it != iter->second.vec.end()-1; ++it) {
			stringStream << it->tick;
			stringStream << "\t";
			int cycle = it->tick/500;
			stringStream << cycle;
			stringStream << "\t";
			stringStream << it->fetch_status;
			stringStream << ":";
			stringStream << it->fetch_overall_status;
			stringStream << "\t";
			stringStream << it->decode_status;
			stringStream << ":";
			stringStream << it->decode_overall_status;
			stringStream << "\t";
			stringStream << it->rename_status;
			stringStream << ":";
			stringStream << it->rename_overall_status;
			stringStream << ":";
			stringStream << it->rename_reason_to_block;
			stringStream << "\t";
			stringStream << it->dispatch_iew_status;
			stringStream << ":";
			stringStream << it->exe_iew_status;
			stringStream << ":";
			stringStream << it->wb_iew_status;
			stringStream << ":";
			stringStream << it->iew_overall_status;
			stringStream << "\t";
			stringStream << it->commit_status;
			stringStream << ":";
			stringStream << it->commit_overall_status;
			stringStream << ":";
			stringStream << it->commit_head_instruction;
			stringStream << "\t";
			stringStream << it->l1_mshr_total;
			stringStream << ":";
			stringStream << it->l1_mshr_ready;
			stringStream << "\t";
			stringStream << it->l2_mshr_total;
			stringStream << ":";
			stringStream << it->l2_mshr_ready;
			stringStream << "\t";
			stringStream << it->l3_mshr_total;
			stringStream << ":";
			stringStream << it->l3_mshr_ready;
			stringStream << "\n";
		}
			coreStatus last_entry;
			last_entry.tick = statusMap[core].vec.back().tick;
			last_entry.fetch_status = statusMap[core].vec.back().fetch_status ;
			last_entry.fetch_overall_status = statusMap[core].vec.back().fetch_overall_status;
			last_entry.decode_status = statusMap[core].vec.back().decode_status ;
			last_entry.decode_overall_status = statusMap[core].vec.back().decode_overall_status;
			last_entry.rename_status = statusMap[core].vec.back().rename_status;
			last_entry.rename_overall_status = statusMap[core].vec.back().rename_overall_status;
			last_entry.rename_reason_to_block = statusMap[core].vec.back().rename_reason_to_block;
			last_entry.dispatch_iew_status = statusMap[core].vec.back().dispatch_iew_status;
			last_entry.exe_iew_status = statusMap[core].vec.back().exe_iew_status;
			last_entry.wb_iew_status = statusMap[core].vec.back().wb_iew_status;
			last_entry.iew_overall_status = statusMap[core].vec.back().iew_overall_status;
			last_entry.commit_status= statusMap[core].vec.back().commit_status;
			last_entry.commit_overall_status = statusMap[core].vec.back().commit_overall_status;
			last_entry.commit_head_instruction = statusMap[core].vec.back().commit_head_instruction;
			last_entry.l1_mshr_total = statusMap[core].vec.back().l1_mshr_total ;
			last_entry.l1_mshr_ready = statusMap[core].vec.back().l1_mshr_ready ;
			last_entry.l2_mshr_total = statusMap[core].vec.back().l2_mshr_total;
			last_entry.l2_mshr_ready = statusMap[core].vec.back().l2_mshr_ready;
			last_entry.l3_mshr_total = statusMap[core].vec.back().l3_mshr_total;
			last_entry.l3_mshr_ready = statusMap[core].vec.back().l3_mshr_ready;

			last_entry_vector.push_back(last_entry);
	}

	for (int i = 0; i < statusMap.size(); i++)
	{
		statusMap.erase(i);
		statusMap[i].vec.push_back(last_entry_vector.at(i));
	}
	last_entry_vector.clear();
//	std::cout << "printing coreStatus\n";
//	printStatusMap(statusMap);
//	for (int iter = 0; iter < statusMap.size(); iter++)
//	{
//		for(int it = 0; it < statusMap[iter].vec.size()-1; ++it) {
////			iter->second.vec.erase(iter->second.vec.begin() + it);
//			statusMap[iter].vec.erase(statusMap[iter].vec.begin()+it);
//		}
//	}
}
void writeToFile(std::map<int, coreStatusVector> &statusMap) {

	typedef std::map<int, coreStatusVector>::const_iterator MapIterator;
	for (MapIterator iter = statusMap.begin(); iter != statusMap.end(); iter++)
	{
		std::string stringBuf;
		std::string str1;
		std::ostringstream stringStream;
		int core = iter->first;
		std::string tempfile = filename + std::to_string(core) + ".txt";
//		filename = tempfile;
//		std::string tempstringBuf = stringBuf + std::to_string(core);

//		stringBuf = tempstringBuf;
//		outfileptr = outfileptr + core.str();
		for(std::vector<coreStatus>::const_iterator it = iter->second.vec.begin(); it != iter->second.vec.end(); ++it) {
			stringStream << it->tick;
			stringStream << "\t";
			stringStream << it->fetch_status;
			stringStream << ":";
			stringStream << it->fetch_overall_status;
			stringStream << "\t";
			stringStream << it->decode_status;
			stringStream << ":";
			stringStream << it->decode_overall_status;
			stringStream << "\t";
			stringStream << it->rename_status;
			stringStream << ":";
			stringStream << it->rename_overall_status;
			stringStream << ":";
			stringStream << it->rename_reason_to_block;
			stringStream << "\t";
			stringStream << it->dispatch_iew_status;
			stringStream << ":";
			stringStream << it->exe_iew_status;
			stringStream << ":";
			stringStream << it->wb_iew_status;
			stringStream << ":";
			stringStream << it->iew_overall_status;
			stringStream << "\t";
			stringStream << it->commit_status;
			stringStream << ":";
			stringStream << it->commit_overall_status;
			stringStream << ":";
			stringStream << it->commit_head_instruction;
			stringStream << "\t";
			stringStream << it->l1_mshr_total;
			stringStream << ":";
			stringStream << it->l1_mshr_ready;
			stringStream << "\t";
			stringStream << it->l2_mshr_total;
			stringStream << ":";
			stringStream << it->l2_mshr_ready;
			stringStream << "\t";
			stringStream << it->l3_mshr_total;
			stringStream << ":";
			stringStream << it->l3_mshr_ready;
			stringStream << "\n";
		}
		str1 = stringStream.str();
		stringBuf.append(str1);
		outfileptr.open(tempfile);
		outfileptr << stringBuf;
		outfileptr.flush();
		outfileptr.close();
	}
}


void initEntry(std::map<int, coreStatusVector> &statusMap, int core, Tick tick) {

		coreStatus cs;
		cs.tick = tick;
		cs.fetch_status = statusMap[core].vec.back().fetch_status ;
		cs.fetch_overall_status = statusMap[core].vec.back().fetch_overall_status;
		cs.decode_status = statusMap[core].vec.back().decode_status ;
		cs.decode_overall_status = statusMap[core].vec.back().decode_overall_status;
		cs.rename_status = statusMap[core].vec.back().rename_status;
		cs.rename_overall_status = statusMap[core].vec.back().rename_overall_status;
		cs.rename_reason_to_block = statusMap[core].vec.back().rename_reason_to_block;
		cs.dispatch_iew_status = statusMap[core].vec.back().dispatch_iew_status;
		cs.exe_iew_status = statusMap[core].vec.back().exe_iew_status;
		cs.wb_iew_status = statusMap[core].vec.back().wb_iew_status;
		cs.iew_overall_status = statusMap[core].vec.back().iew_overall_status;
		cs.commit_status= statusMap[core].vec.back().commit_status;
		cs.commit_overall_status = statusMap[core].vec.back().commit_overall_status;
		cs.commit_head_instruction = statusMap[core].vec.back().commit_head_instruction;
		cs.l1_mshr_total = statusMap[core].vec.back().l1_mshr_total ;
		cs.l1_mshr_ready = statusMap[core].vec.back().l1_mshr_ready ;
		cs.l2_mshr_total = statusMap[core].vec.back().l2_mshr_total;
		cs.l2_mshr_ready = statusMap[core].vec.back().l2_mshr_ready;
		cs.l3_mshr_total = statusMap[core].vec.back().l3_mshr_total;
		cs.l3_mshr_ready = statusMap[core].vec.back().l3_mshr_ready;
		statusMap[core].vec.push_back(cs);
}

void createEntry(std::map<int, coreStatusVector> &statusMap, int core, Tick tick) {

		coreStatus cs;
		cs.tick = tick;
		cs.fetch_status = -1;
		cs.fetch_overall_status = -1;
		cs.decode_status = -1;
		cs.decode_overall_status = -1;
		cs.rename_status = -1;
		cs.rename_overall_status = -1;
		cs.rename_reason_to_block = "none";
		cs.dispatch_iew_status = -1;
		cs.exe_iew_status = -1;
		cs.wb_iew_status = -1;
		cs.iew_overall_status = -1;
		cs.commit_status= -1;
		cs.commit_overall_status = -1;
		cs.commit_head_instruction = "none";
		cs.l1_mshr_total = -1;
		cs.l1_mshr_ready = -1;
		cs.l2_mshr_total = -1;
		cs.l2_mshr_ready = -1;
		cs.l3_mshr_total = -1;
		cs.l3_mshr_ready = -1;
		statusMap[core].vec.push_back(cs);
}

void addEntryToMap(std::map<int, coreStatusVector> &statusMap, int core, Tick tick, std::string stageString, std::string stageName, int status, int overall_status, std::string info){
//
//if(!statusMap[core].vec.empty())
//	if(tick > statusMap[core].vec.back().tick) {
//		writeFile(statusMap);
//	}
//	std::cout << tick <<" PRINT addEntryToMap core id: " << core  << " " << stageString << " status: " << status << " overall " << overall_status << "\n";
	if(stageName.find("fetch") != std::string::npos) {
		if (statusMap.find(core) != statusMap.end()) {
			if (statusMap[core].vec.back().tick < tick && statusMap[core].vec.back().fetch_status != status) {
				initEntry(statusMap, core, tick);
				statusMap[core].vec.back().fetch_status = status;
				statusMap[core].vec.back().fetch_overall_status = overall_status;
			} else if (statusMap[core].vec.back().tick == tick && statusMap[core].vec.back().fetch_status != status) {
				statusMap[core].vec.back().fetch_status = status;
				statusMap[core].vec.back().fetch_overall_status = overall_status;
			} else if (statusMap[core].vec.back().tick > tick) {
				std::cout << "Error current tick is smaller than previous entry!!\n";
			}
		}
		else {
			createEntry(statusMap, core, tick);
			statusMap[core].vec.back().fetch_status = status;
			statusMap[core].vec.back().fetch_overall_status = overall_status;
		}
	}

	if(stageName.find("decode") != std::string::npos) {
		if (statusMap.find(core) != statusMap.end()) {
			if (statusMap[core].vec.back().tick < tick && statusMap[core].vec.back().decode_status != status) {
				initEntry(statusMap, core, tick);
				statusMap[core].vec.back().decode_status = status;
				statusMap[core].vec.back().decode_overall_status = overall_status;
			} else if (statusMap[core].vec.back().tick == tick && statusMap[core].vec.back().decode_status != status) {
				statusMap[core].vec.back().decode_status = status;
				statusMap[core].vec.back().decode_overall_status = overall_status;
			} else if (statusMap[core].vec.back().tick > tick) {
				std::cout << "Error current tick is smaller than previous entry!!\n";
			}
		}
		else {
			createEntry(statusMap, core, tick);
			statusMap[core].vec.back().decode_status = status;
			statusMap[core].vec.back().decode_overall_status = overall_status;
		}
	}


	if(stageName.find("rename") != std::string::npos) {
		if (statusMap.find(core) != statusMap.end()) {
			if (statusMap[core].vec.back().tick < tick && statusMap[core].vec.back().rename_status != status) {
				initEntry(statusMap, core, tick);
				statusMap[core].vec.back().rename_status = status;
				statusMap[core].vec.back().rename_overall_status = overall_status;
				statusMap[core].vec.back().rename_reason_to_block = info;
			} else if (statusMap[core].vec.back().tick == tick && statusMap[core].vec.back().rename_status != status) {
				statusMap[core].vec.back().rename_status = status;
				statusMap[core].vec.back().rename_overall_status = overall_status;
				statusMap[core].vec.back().rename_reason_to_block = info;
			} else if (statusMap[core].vec.back().tick > tick) {
				std::cout << "Error current tick is smaller than previous entry!!\n";
			}
		}
		else {
			createEntry(statusMap, core, tick);
			statusMap[core].vec.back().rename_status = status;
			statusMap[core].vec.back().rename_overall_status = overall_status;
			statusMap[core].vec.back().rename_reason_to_block = info;
		}
	}


	if(stageName.find("dispatch") != std::string::npos) {
		if (statusMap.find(core) != statusMap.end()) {
			if (statusMap[core].vec.back().tick < tick && statusMap[core].vec.back().dispatch_iew_status != status) {
				initEntry(statusMap, core, tick);
				statusMap[core].vec.back().dispatch_iew_status = status;
				statusMap[core].vec.back().iew_overall_status = overall_status;
			} else if (statusMap[core].vec.back().tick == tick && statusMap[core].vec.back().dispatch_iew_status != status) {
				statusMap[core].vec.back().dispatch_iew_status = status;
				statusMap[core].vec.back().iew_overall_status = overall_status;
			} else if (statusMap[core].vec.back().tick > tick) {
				std::cout << "Error current tick is smaller than previous entry!!\n";
			}
		}
		else {
			createEntry(statusMap, core, tick);
			statusMap[core].vec.back().dispatch_iew_status = status;
			statusMap[core].vec.back().iew_overall_status = overall_status;
		}
	}


	if(stageName.find("iew") != std::string::npos) {
		if (statusMap.find(core) != statusMap.end()) {
			if (statusMap[core].vec.back().tick < tick && statusMap[core].vec.back().exe_iew_status != status) {
				initEntry(statusMap, core, tick);
				statusMap[core].vec.back().exe_iew_status = status;
				statusMap[core].vec.back().iew_overall_status = overall_status;
			} else if (statusMap[core].vec.back().tick == tick && statusMap[core].vec.back().exe_iew_status != status) {
				statusMap[core].vec.back().exe_iew_status = status;
				statusMap[core].vec.back().iew_overall_status = overall_status;
			} else if (statusMap[core].vec.back().tick > tick) {
				std::cout << "Error current tick is smaller than previous entry!!\n";
			}
		}
		else {
			createEntry(statusMap, core, tick);
			statusMap[core].vec.back().exe_iew_status = status;
			statusMap[core].vec.back().iew_overall_status = overall_status;
		}
	}


	if(stageName.find("wb") != std::string::npos) {
		if (statusMap.find(core) != statusMap.end()) {
			if (statusMap[core].vec.back().tick < tick && statusMap[core].vec.back().wb_iew_status != status) {
				initEntry(statusMap, core, tick);
				statusMap[core].vec.back().wb_iew_status = status;
				statusMap[core].vec.back().iew_overall_status = overall_status;
			} else if (statusMap[core].vec.back().tick == tick && statusMap[core].vec.back().wb_iew_status != status) {
				statusMap[core].vec.back().wb_iew_status = status;
				statusMap[core].vec.back().iew_overall_status = overall_status;
			} else if (statusMap[core].vec.back().tick > tick) {
				std::cout << "Error current tick is smaller than previous entry!!\n";
			}
		}
		else {
			createEntry(statusMap, core, tick);
			statusMap[core].vec.back().wb_iew_status = status;
			statusMap[core].vec.back().iew_overall_status = overall_status;
		}
	}


	if(stageName.find("commit") != std::string::npos) {
		if (statusMap.find(core) != statusMap.end()) {
//			std::cout << "head " << statusMap[core].vec.back().commit_head_instruction << "\tinfo " << info << "\n";
			if (statusMap[core].vec.back().tick <= tick && info.compare("none")) {
				initEntry(statusMap, core, tick);
				statusMap[core].vec.back().commit_status = status;
				statusMap[core].vec.back().commit_overall_status = overall_status;
				statusMap[core].vec.back().commit_head_instruction = info;
			}
			if (statusMap[core].vec.back().tick < tick && statusMap[core].vec.back().commit_status != status) {
				initEntry(statusMap, core, tick);
				statusMap[core].vec.back().commit_status = status;
				statusMap[core].vec.back().commit_overall_status = overall_status;
//				statusMap[core].vec.back().commit_head_instruction = info;
			} else if (statusMap[core].vec.back().tick == tick && statusMap[core].vec.back().commit_status != status) {
				statusMap[core].vec.back().commit_status = status;
				statusMap[core].vec.back().commit_overall_status = overall_status;
//				statusMap[core].vec.back().commit_head_instruction = info;
			} else if (statusMap[core].vec.back().tick > tick) {
				std::cout << "Error current tick is smaller than previous entry!!\n";
			}
		}
		else {
			createEntry(statusMap, core, tick);
			statusMap[core].vec.back().commit_status = status;
			statusMap[core].vec.back().commit_overall_status = overall_status;
			statusMap[core].vec.back().commit_head_instruction = info;
		}
	}

	if(stageString.find("icache") != std::string::npos || stageString.find("dcache") != std::string::npos) {
		if (statusMap.find(core) != statusMap.end()) {
			if (statusMap[core].vec.back().tick < tick) {
				initEntry(statusMap, core, tick);
				statusMap[core].vec.back().l1_mshr_total = status;
				statusMap[core].vec.back().l1_mshr_ready = overall_status;
//				std::cout << "PRINT 1 i/dcache: " << statusMap[core].vec.back().l1_mshr_total << "\t" << statusMap[core].vec.back().l1_mshr_ready << "\n";
			} else if (statusMap[core].vec.back().tick == tick) {
				statusMap[core].vec.back().l1_mshr_total = status;
				statusMap[core].vec.back().l1_mshr_ready = overall_status;
//				std::cout << "PRINT 2 i/dcache: " << statusMap[core].vec.back().l1_mshr_total << "\t" << statusMap[core].vec.back().l1_mshr_ready << "\n";
			} else if (statusMap[core].vec.back().tick > tick) {
				std::cout << "Error current tick is smaller than previous entry!!\n";
			}
		}
		else {
			createEntry(statusMap, core, tick);
			statusMap[core].vec.back().l1_mshr_total = status;
			statusMap[core].vec.back().l1_mshr_ready = overall_status;
//			std::cout << "PRINT 3 i/dcache: " << statusMap[core].vec.back().l1_mshr_total << "\t" << statusMap[core].vec.back().l1_mshr_ready << "\n";
		}
	}

	if(stageString.find("l2") != std::string::npos) {
		if (statusMap.find(core) != statusMap.end()) {
			if (statusMap[core].vec.back().tick < tick) {
				initEntry(statusMap, core, tick);
				statusMap[core].vec.back().l2_mshr_total = status;
				statusMap[core].vec.back().l2_mshr_ready = overall_status;
			} else if (statusMap[core].vec.back().tick == tick) {
				statusMap[core].vec.back().l2_mshr_total = status;
				statusMap[core].vec.back().l2_mshr_ready = overall_status;
			} else if (statusMap[core].vec.back().tick > tick) {
				std::cout << "Error current tick is smaller than previous entry!!\n";
			}
		}
		else {
			createEntry(statusMap, core, tick);
			statusMap[core].vec.back().l2_mshr_total = status;
			statusMap[core].vec.back().l2_mshr_ready = overall_status;
		}
	}

	if(stageString.find("l3") != std::string::npos) {
		if (statusMap.find(core) != statusMap.end()) {
			if (statusMap[core].vec.back().tick < tick) {
				initEntry(statusMap, core, tick);
				statusMap[core].vec.back().l3_mshr_total = status;
				statusMap[core].vec.back().l3_mshr_ready = overall_status;
			} else if (statusMap[core].vec.back().tick == tick) {
				statusMap[core].vec.back().l3_mshr_total = status;
				statusMap[core].vec.back().l3_mshr_ready = overall_status;
			} else if (statusMap[core].vec.back().tick > tick) {
				std::cout << "Error current tick is smaller than previous entry!!\n";
			}
		}
		else {
			createEntry(statusMap, core, tick);
			statusMap[core].vec.back().l3_mshr_total = status;
			statusMap[core].vec.back().l3_mshr_ready = overall_status;
		}
	}

		 writeAfter--;
	 if(writeAfter <= 0){
		 writeAfter = 1000;
		 writeFile(statusMap);
	 }
////		 writeToFile(statusMap);
//	 }

//	 printStatusMap(statusMap);
}


int readCoreId(std::string name) {		//reading first integer from a string as a core id

	std::vector<char> v(name.length() + 1);
	std::strcpy(&v[0], name.c_str());
	char* p = &v[0];
	long int core = -1;
	while (*p) { // While there are more characters to process...
	    if (isdigit(*p)) { // Upon finding a digit, ...
	        core = strtol(p, &p, 10); // Read a number, ...
//	        printf("CPU id %d\n", int(core)); // and print it.
	        break;
	    }
	    else { // Otherwise, move on to the next character.
	        p++;
	    }
	}
	return int(core);
}


void recordingStatus(Tick tick, std::string stageString, std::string stageName, int status, int overall_status, std::string info){

	int core;
	if(stageString.find("cpu.l2") != std::string::npos)
	{
			addEntryToMap(completeStatus, 0, tick, stageString, stageName, status, overall_status, info);
	}
	else if(stageString.find("cpu") != std::string::npos && stageString.find("cpu.l2") == std::string::npos)
	{
		core = readCoreId(stageString);
		if(core == -1) {
			addEntryToMap(completeStatus, 0, tick, stageString, stageName, status, overall_status, info);
		}
		else {
			addEntryToMap(completeStatus, core, tick, stageString, stageName, status, overall_status, info);
		}
	}

	else if(stageString.find("l3") != std::string::npos) {
		for (int i = 0; i < completeStatus.size(); i++)
		{
			addEntryToMap(completeStatus, i, tick, stageString, stageName, status, overall_status, info);
		}
	}
}

void recordStatus(Tick tick, std::string stageString, std::string stageName, int status, int overall_status, std::string info) {

//	std::cout << tick << "\t" << stageString << "\t" << stageName << "\t" << status << "\t" << overall_status << "\t" << info << "\n";
	if(recordStatusFlag) {
		recordingStatus(tick, stageString, stageName, status, overall_status, info);
	}
}

void printL3Status(std::map<int, L3QStatusVector> &statusMap) {

	std::cout << "statusMap size- " << statusMap.size() << "\n";
	typedef std::map<int, L3QStatusVector>::const_iterator MapIterator;
	for (MapIterator iter = statusMap.begin(); iter != statusMap.end(); iter++)
	{
//	    std::cout << "Core: " << iter->first << " has " << iter->second.vec.size() << " entries\n";
	    for(std::vector<L3QStatus>::const_iterator it = iter->second.vec.begin(); it != iter->second.vec.end(); ++it) {
	    	std::cout << it->tick << "\t";
	    	std::cout << it->operationType << "\t";
	    	std::cout << it->groupT1 << ":" << it->groupT2 << "\t";
	    	for(int i = 0; i < 8; i++) {
	    		std::cout << it->requestCount[i];
	    		std::cout << ":";
	    	}
	    	std::cout << it->prioritizedCore << "\n";
	    }
	}
}

void recordingL3Status(Tick tick, std::string stageString, std::string stageName, int groupT1count, int groupT2count, std::vector<int> coreCount, int groupO1count, int groupO2count, std::vector<int> coreOutSCount, int prioritizedCore) {

//	std::ofstream L3Qfilept;

	L3QStatus ls;
	ls.tick = tick;
	ls.operationType = stageName;
	ls.groupT1 = groupT1count;
	ls.groupT2 = groupT2count;
	ls.requestCount = coreCount;
	ls.groupO1 = groupO1count;
	ls.groupO2 = groupO2count;
	ls.requestOutSCount = coreOutSCount;
	ls.prioritizedCore = prioritizedCore;
	L3QCompleteStatus[tick].vec.push_back(ls);
//	std::cout << ls.tick << "\t" << ls.operationType << "\t" << ls.groupT1 << "\t" << ls.groupT2  << "\t" << ls.prioritizedCore << "\n";

//	printL3Status(L3QCompleteStatus);

	writeAfter1--;
	if(writeAfter1 <= 0){
		writeAfter1 = 1000;
		std::string file1 = "./results/L3QData.txt";
		std::ofstream stringStream1;
		stringStream1.open(file1, std::ios_base::app);
		typedef std::map<int, L3QStatusVector>::const_iterator MapIterator;
			for (MapIterator iter = L3QCompleteStatus.begin(); iter != L3QCompleteStatus.end(); iter++)	{
				for(std::vector<L3QStatus>::const_iterator it = iter->second.vec.begin(); it != iter->second.vec.end(); ++it) {
					stringStream1 << it->tick;
					stringStream1 << "\t";
					int cycle = it->tick/500;
					stringStream1 << cycle;
					stringStream1 << "\t";
					stringStream1 << it->operationType;
					stringStream1 << "\t";
					stringStream1 << it->groupT1;
					stringStream1 << ":";
					stringStream1 << it->groupT2;
					stringStream1 << "\t";

					float ratio = float(float(it->groupT1)/float(it->groupT2));
					stringStream1 << ratio;
					stringStream1 << "\t";

					for(int i = 0; i < 8; i++) {
						stringStream1 << it->requestCount[i];
						stringStream1 << ":";
					}
					stringStream1 << "\t";
					stringStream1 << it->groupO1;
					stringStream1 << ":";
					stringStream1 << it->groupO2;
					stringStream1 << "\t";

					for(int i = 0; i < 8; i++) {
						stringStream1 << it->requestOutSCount[i];
						stringStream1 << ":";
					}
					stringStream1 << "\t";
					stringStream1 << it->prioritizedCore;
					stringStream1 << "\n";
				}
			}
//			for (int i = 0; i < L3QCompleteStatus.size(); i++)
//			{
				L3QCompleteStatus.clear();
//			}
//			std::cout << "printing L3QStatus\n";
//			printL3Status(L3QCompleteStatus);
	}
}


void recordL3Status(Tick tick, std::string stageString, std::string stageName, int groupT1count, int groupT2count, std::vector<int> coreCount, int groupO1count, int groupO2count, std::vector<int> coreOutSCount, int prioritizedCore) {

	if(recordL3StatusFlag) {
		recordingL3Status(tick, stageString, stageName, groupT1count, groupT2count, coreCount, groupO1count, groupO2count, coreOutSCount, prioritizedCore);
	}
}


uint64_t getRequestId()
{
	return ++reqIdCounter;
}


uint64_t calcAverageStat(uint64_t numerator, uint64_t denominator) {
	uint64_t avg = numerator/denominator;
	return avg;
}


void modifyStats(uint64_t &totalDelayStat, uint64_t delayVal, uint64_t &totalEntryStat, uint64_t &avg) {

	delayVal = delayVal/500;
	totalDelayStat += delayVal;
	totalEntryStat += 1;
	avg = calcAverageStat(totalDelayStat, totalEntryStat);
}

void addDelayToMap(std::map<uint64_t, uint64_t> &delayMap, uint64_t delay) {

	delay = delay/500;
	if (delayMap.find(delay) != delayMap.end()) {
		delayMap[delay]++;
	 }
	 else {
		 delayMap[delay] = 1;
	 }
}

void addStageDelay(uint64_t rid, uint64_t fromTick, uint64_t toTick, std::string stageName, std::map<uint64_t, uint64_t> &delayMap, TotalStats &totalStats) {
	PacketDelayPathInfo pdpi;
	pdpi.stageName = stageName;
	pdpi.delay = toTick - fromTick;
	map2[rid].vec.push_back(pdpi);
	addDelayToMap(delayMap, pdpi.delay);
	modifyStats(totalStats.delay, pdpi.delay, totalStats.numEntry, totalStats.avg);
}

void printStats(TotalStats &totalStats){
	std::cout << totalStats.delay << "\t" << totalStats.numEntry << "=\t" << totalStats.avg << "\n";
}

void printMap(std::map<uint64_t, uint64_t> &delayMap) {
	 std::cout << "Size- " << delayMap.size() << "\n";
	typedef map<uint64_t, uint64_t>::const_iterator MapIterator;
 	for (MapIterator iter = delayMap.begin(); iter != delayMap.end(); iter++)
 	{
 	    std::cout << iter->first << "\t" << iter->second << "\n";
 	}
}

bool checkCounter(uint64_t &counter, int threshold) {

	if(--counter == 0) {
		counter = threshold;
		return true;
	}
	else {
		return false;
	}
}

void printLittlesLawStats(std::string statName, uint64_t total, uint64_t entries) {
	std::cout << statName << "\t" << total << "\t" << entries << "\n";
}

void calcQTime(uint64_t fromTick, uint64_t toTick) {
//	if(i < 10) {
//	avgQTime.delay = avgQTime.delay + toTick - fromTick;	//time taken by a request to wait in a queue
//	avgQTime.numEntry = avgQTime.numEntry + 1;	//total number of requests
//	std::cout << "waiting time for this packet " << (toTick - fromTick) << "\n";
//	std::cout << toTick << "\t" << fromTick << "\n";
//	printLittlesLawStats("AvgQTime", avgQTime.delay, avgQTime.numEntry);
//	i++;
//	}
}

void calcThroughput() {
//	if(j < 10) {
	if(firstThroughput) {
		avgThroughput.numEntry += 1;
	}
	else {
		avgThroughput.numEntry = 1;
//		avgGlobalQSize.delay = 0;	// initializing default values GlobalQ size
//		avgGlobalQSize.numEntry = 0;	// initializing default value of last cycle at which GlobalQ size changed
		firstThroughput = true;
	}
//	printLittlesLawStats("AvgThroughput", avgThroughput.numEntry, curTick());
//	j++;
//		}
}

void calcQSize(std::string cacheName, uint64_t atTick, uint64_t qSize) {
	uint64_t atCycle = atTick/500;
	 if (avgQSize.find(cacheName) != avgQSize.end()) {
		 avgQSize[cacheName].delay = avgQSize[cacheName].delay + qSize*(atCycle-avgQSize[cacheName].numEntry);
		 avgQSize[cacheName].numEntry = atCycle;
	 }
	 else {
		 TotalStats ts;
		 ts.delay = qSize;	//stores the updated qsize of readyList (which are a part of mshrQueues)
		 ts.numEntry = atCycle;	//last cycle when the queue was updated
		 avgQSize[cacheName] = ts;
	 }
//	 std::cout << "readyList size " << qSize << "\t";
//	 printLittlesLawStats(cacheName+"-AvgQSize", avgQSize[cacheName].delay, avgQSize[cacheName].numEntry);
}

//void initQLenStats(std::map<std::string, qLength> &qLenStat, TotalStats &avgQTime, std::string cacheName, std::string mshr, uint64_t curTick, uint64_t atTick, uint64_t qSize, std::string mode) {
//	if(mode == "I") {
//			if (qLenStat.find(mshr) != qLenStat.end()) {
//				std::cout << "ERROR1\n";
//			}
//			else {
//				qLength q;
//				q.cycle = curTick;
//				q.readyTime = atTick;
//				q.cacheName = cacheName;
//				qLenStat[mshr] = q;
////				std::cout << "added entry to queue for " << cacheName << "\n";
//			}
//		}
//		else {
//			if (qLenStat.find(mshr) != qLenStat.end()) {
//				qLenStat[mshr].exitTime = atTick;
//				avgQTime.delay = avgQTime.delay + qLenStat[mshr].exitTime - qLenStat[mshr].readyTime;	//time taken by a request to wait in a queue
//				avgQTime.numEntry = avgQTime.numEntry + 1;	//total number of requests
//				double avgQtime = (((double)avgQTime.delay/(double)avgQTime.numEntry)/(double)500);
//				double avgThr = (double)(((double)avgQTime.numEntry/(double)atTick)*(double)500);
//				std::cout << "THROU LITTLE\t" << qLenStat[mshr].cacheName << "\t" << mshr << "\t" << qLenStat[mshr].cycle << "\t" << qLenStat[mshr].readyTime << "\t" << qLenStat[mshr].exitTime << "\t";
//				std::cout << qLenStat[mshr].exitTime - qLenStat[mshr].readyTime << "\t" << std::fixed << avgQtime << "\t";
//				std::cout << avgQTime.numEntry << "\t" << atTick << "\t" << std::fixed << avgThr << "\t\t" << std::fixed <<(avgThr*avgQtime) << "\n";
//				qLenStat.erase(mshr);
//			}
//			else {
//				std::cout << "ERROR2\n";
//			}
//		}
//}
uint64_t mycount = 100;
uint32_t array_index = 0;
uint32_t array_flag = 0;
int my_Qsize = 0;
uint64_t prev_del_tick = 0;
uint64_t cur_del_tick = 0;
void calcGQSize(std::string cacheName, std::string mshr, uint64_t cur_Tick, uint64_t atTick, uint64_t qSize, std::string mode) {
//	if(k < 10) {

	if(mode == "I") {
			if (qLenStat.find(mshr) != qLenStat.end()) {
				std::cout << "ERROR1\n";
			}
			else {
				qLength q;
				q.cycle = cur_Tick;
				q.readyTime = atTick;
				q.cacheName = cacheName;
//				q.visited = false;
				qLenStat[mshr] = q;
//				std::cout << "added entry to queue for " << cacheName << "\n";
          		int Qsize = 0;
          		typedef map<std::string, qLength>::const_iterator MapIterator;
          		for (MapIterator iter = qLenStat.begin(); iter != qLenStat.end(); iter++)
          		{
          			if(iter->second.readyTime <= curTick()) {
          				Qsize++;
          			}
          			else {
          				continue;
          			}
//          			std::cout << iter->first << "\t" << iter->second.cacheName << "\t" << iter->second.cycle << "\t" << iter->second.readyTime << "\n";
          		}
//				std::cout << "ENTRY LITTLE\t" << qLenStat[mshr].cacheName << "\t" << mshr << "\t" << qLenStat[mshr].cycle << "\t" << qLenStat[mshr].readyTime  << "\t" << qLenStat.size() << "\t" << Qsize << "\t" << my_Qsize << "\n";
			}
		}
		else {
			if (qLenStat.find(mshr) != qLenStat.end()) {
				qLenStat[mshr].exitTime = atTick;

				cur_del_tick = qLenStat[mshr].exitTime;
          		int map_size = qLenStat.size();
          		uint64_t arr[map_size], c = 0;
          		typedef map<std::string, qLength>::const_iterator MapIterator;
          		for (MapIterator iter = qLenStat.begin(); iter != qLenStat.end(); iter++)
          		{
//          			if(iter->first == mshr) {
//          				continue;
//          			}
          			if(iter->second.readyTime <= cur_del_tick && iter->second.readyTime > prev_del_tick) {
//          			if(iter->second.readyTime <= cur_del_tick && !iter->second.visited) {
          				arr[c] = iter->second.readyTime;
//    					std::cout << "\nc " << arr[c] << "\t";
          				c++;
          			}
          			else {
          				continue;
          			}
//          			std::cout << iter->first << "\t" << iter->second.cacheName << "\t" << iter->second.cycle << "\t" << iter->second.readyTime << "\n";
          		}

          		arr[c++] = cur_del_tick;
//
				for(int i = 0; i < c; i++) {
//					std::cout << "\narr " << arr[i] << "\t";
//					uint64_t min = arr[i];
					for (int j = i+1; j < c; j++) {
						if(arr[i] > arr[j]) {
							uint64_t temp = arr[j];
							arr[j] = arr[i];
							arr[i] = temp;
						}
					}
				}
				for(int i = 0; i < c; i++) {
					array_cycle[array_index] = arr[i];
					if(i == c-1) {
						array_qLen[array_index] = -1;
					}
					else {
						array_qLen[array_index] = 1;
					}
					array_index++;

	          		if(array_index == array_length-1) {
	          			array_index = 0;
	          			std::string str1;
	          			std::ostringstream stringStream;
	          			uint32_t i = 0;
	          			uint64_t j = 0;
	          			for(i = 0,j = 0; (i < array_length) && (j < array_length); i++,j++) {
	          				stringStream << array_cycle[j];
	          				stringStream << "\t";
	          				stringStream << array_qLen[i];
	          				stringStream << "\n";
	          	//			std::string str1 = array_cycle[i].str() + "\t" + array_qLen[i].str() + "\n";
	          			}
	          			str1 = stringStream.str();
	          			strBuf.append(str1);

	          			outfile.open("cycle_qLen.txt");
	          			outfile << strBuf;
	          			outfile.flush();
	          			outfile.close();
	          		}
				}
//				array_cycle[array_index] = cur_del_tick;
//				array_qLen[array_index] = -1;
//				array_index++;
				prev_del_tick = cur_del_tick;

				avgQTime.delay = avgQTime.delay + qLenStat[mshr].exitTime - qLenStat[mshr].readyTime;	//time taken by a request to wait in a queue
				avgQTime.numEntry = avgQTime.numEntry + 1;	//total number of requests
				double avgQtime = (((double)avgQTime.delay/(double)avgQTime.numEntry)/(double)500);
				double avgThr = (double)(((double)avgQTime.numEntry/(double)atTick)*(double)500);
				if(mycount == 0) {
					std::cout << "THROU LITTLE\t" << qLenStat[mshr].cacheName << "\t" << mshr << "\t" << qLenStat[mshr].cycle << "\t" << qLenStat[mshr].readyTime << "\t" << qLenStat[mshr].exitTime << "\t";
					std::cout << qLenStat[mshr].exitTime - qLenStat[mshr].readyTime << "\t" << std::fixed << avgQtime << "\t";
					std::cout << avgQTime.numEntry << "\t" << atTick << "\t" << std::fixed << avgThr << "\t\t" << std::fixed <<(avgThr*avgQtime) << "\t" << (qLenStat.size()-1) << "\n";
					mycount = 100;
				}
				mycount--;
				qLenStat.erase(mshr);
          		int Qsize = 0;
          		typedef map<std::string, qLength>::const_iterator MapIterator;
          		for (MapIterator iter = qLenStat.begin(); iter != qLenStat.end(); iter++)
          		{
          			if(iter->second.readyTime <= curTick()) {
          				Qsize++;
          			}
          			else {
          				continue;
          			}
//          			std::cout << iter->first << "\t" << iter->second.cacheName << "\t" << iter->second.cycle << "\t" << iter->second.readyTime << "\n";
          		}
//
//          		if(array_index < array_length-1){
//          			array_cycle[array_index] = curTick();
//          			array_qLen[array_index] = Qsize;
//          			array_index++;
//          			--my_Qsize;
//          		}
//          		else {
//          			array_cycle[array_index] = curTick();
//          			array_qLen[array_index] = Qsize;
//          			array_index = 0;
//          			array_flag = 1;
//          			--my_Qsize;
//          		}
//				std::cout << Qsize  << "\n";
			}
			else {
				std::cout << "ERROR2\n";
			}
		}

//	if(qLenStat.size() > 0) {
////		std::cout << "Queue is not empty!!\t";
//		std::cout << "Qsize is " << qLenStat.size() << "\n";
//	}

//	if(cacheName.find("icache")!=std::string::npos) {
////		std::cout << "Inside1\n";
//		initQLenStats(qLenStatL1i, avgQTimeL1i, cacheName, mshr, curTick, atTick, qSize, mode);
//	}
//	if(cacheName.find("dcache")!=std::string::npos) {
//	//		std::cout << "Inside1\n";
//			initQLenStats(qLenStatL1d, avgQTimeL1d, cacheName, mshr, curTick, atTick, qSize, mode);
//		}
//	if(cacheName.find("l2") != std::string::npos) {
//		std::cout << "Inside2\n";
//		initQLenStats(qLenStatL2, avgQTimeL2, cacheName, mshr, curTick, atTick, qSize, mode);
//	}
//	if(cacheName.find("l3") != std::string::npos) {
////		std::cout << "Inside3\n";
//		initQLenStats(qLenStatL3, avgQTimeL3, cacheName, mshr, curTick, atTick, qSize, mode);
//	}

//	 std::cout << "qlenStat Size- " << qLenStat.size() << "\n";
//	typedef map<std::string, qLength>::const_iterator MapIterator;
//	for (MapIterator iter = qLenStat.begin(); iter != qLenStat.end(); iter++)
//	{
//	    std::cout << iter->first << "\t" << iter->second.cacheName << "\t" << iter->second.cycle << "\t" << iter->second.readyTime << "\n";
//	}
//	avgQSize[cacheName].delay + qSize*(atCycle-avgQSize[cacheName].numEntry);
//	std::cout << atTick << " Prev GlobalReadyList size " << qSize << "\t tick " << avgGlobalQSize.delay << "\t" << avgGlobalQSize.numEntry << "\n";
//	uint64_t atCycle = atTick;
//	avgGlobalQSize.delay = avgGlobalQSize.delay + qSize*(atCycle-avgGlobalQSize.numEntry);
//	avgGlobalQSize.numEntry = atTick;	//last cycle at which global queue size changed
//	std::cout << atTick << " GlobalReadyList size " << qSize << "\t tick " << avgGlobalQSize.delay << "\t" << avgGlobalQSize.numEntry << "\n";
//	printLittlesLawStats(cacheName+"-AvgGlobalQSize", avgGlobalQSize.delay, avgGlobalQSize.numEntry);
//	k++;
//			}
}

void calcTotalCyclesBusy(uint64_t atTick, std::string stageName, std::string type)
{
	if(totalWaitingForL3 > 0) {
		if(stageName.find("l2.mem_side") != std::string::npos && type == "Req") {
//			std::cout << "came @ l2 memside\n";
			totalWaitingForL3++;
			totalCyclesBusyL3 = totalCyclesBusyL3 + (atTick - lastActiveCycleL3);
			lastActiveCycleL3 = atTick;
			std::cout << atTick << "\t(push)totalWaitingForL3 " << totalWaitingForL3 <<"\ttotalCyclesBusyL3 " << totalCyclesBusyL3 << "\n";
		}
		if(stageName.find("l3.cpu_side") != std::string::npos && type == "Req") {
//			std::cout << "came @ l3 cpuside\n";
			totalWaitingForL3--;
			totalCyclesBusyL3 = totalCyclesBusyL3 + (atTick - lastActiveCycleL3);
			lastActiveCycleL3 = atTick;
			std::cout << atTick << "\t(pop)totalWaitingForL3 " << totalWaitingForL3 <<"\ttotalCyclesBusyL3 " << totalCyclesBusyL3 << "\n";
		}
	}
	else {
		if(stageName.find("l3.cpu_side") != std::string::npos && type == "Req") {
			std::cout << "something is wrong in the L3 bus!!\n";
		}
		else {
			std::cout << "new entry\n";
			totalWaitingForL3++;
			lastActiveCycleL3 = atTick;
			std::cout << atTick << "\t(push)totalWaitingForL3 " << totalWaitingForL3 <<"\ttotalCyclesBusyL3 " << totalCyclesBusyL3 << "\n";
		}
	}
}


void printDelayPath() {

	std::cout << "map1 size- " << map1.size() << "\n";
	typedef map<uint64_t, PacketInfo>::const_iterator MapIterator;
	for (MapIterator iter = map1.begin(); iter != map1.end(); iter++)
	{
		if(iter->second.vec.size() > 0) {
	    std::cout << "map1 Request: " << iter->first << "\n";
	    std::cout << "map1 \t" << iter->second.pktAddr << "\t" << iter->second.masterId << "\t" << iter->second.arrivalTime << "\t" << iter->second.finishTime << "\n";
	    for(std::vector<PacketStageInfo>::const_iterator it = iter->second.vec.begin(); it != iter->second.vec.end(); ++it) {
	    	std::cout << "map1 \t" << it->atTick << "\t" << it->stageName << "\t" << it->type << "\n";
	    }
		}
	}
}

void printStageDelayPath() {

	std::cout << "map2 size- " << map2.size() << "\n";
	typedef map<uint64_t, PacketDelayInfo>::const_iterator MapIterator;
	for (MapIterator iter = map2.begin(); iter != map2.end(); iter++)
	{
	    std::cout << "map2 Request: " << iter->first << "\n";
	    std::cout << "map2 \t" << iter->second.pktAddr << "\t" << iter->second.masterId << "\t" << iter->second.arrivalTime << "\t" << iter->second.finishTime << "\n";
	    for(std::vector<PacketDelayPathInfo>::const_iterator it = iter->second.vec.begin(); it != iter->second.vec.end(); ++it) {
	    	std::cout << "map2 \t" << it->stageName << "\t" << it->delay << "\n";
	    }
	}
}

void initDelayPath(uint64_t rid, uint64_t pktAddr, int masterId, uint64_t arrivalTime) {

	PacketInfo pi;
	pi.masterId = masterId;
	pi.arrivalTime = arrivalTime;
	pi.pktAddr = pktAddr;
	pi.finishTime = 0;
	pi.l1Hit = 0;
	pi.l2Hit = 0;
	pi.l3Hit = 0;
	map1[rid] = pi;
	totalRequests++;
//	std::cout << "packet_info\t" << pi.atStage << "\t" << psi.atTick << "\t" << pi.masterId << "\t" << pid << "\t" << psi.stageName << "\n";

}

void addToDelayPath(uint64_t rid, uint64_t pktAddr, uint64_t atTick, std::string stageName, std::string type, bool isFinish, int isHit) {

//	bool myFlag = false;
	if(curTick() > 5000000000){
	  if (map1.find(rid) != map1.end()) {
//	    std::cout << "map1 contains the packet!\n";
    	if(map1[rid].pktAddr != pktAddr) {
//    		std::cout << "MISMATCH-------------CHECK---------------\n";
    	}
    	else if(!isFinish) {
	    	PacketStageInfo psi;// = new PacketStageInfo();
	    	psi.stageName = stageName;
	    	psi.atTick = atTick;
	    	psi.type = type;
//	    	for(std::vector<PacketStageInfo>::iterator itr = map1[rid].vec.begin(); itr != map1[rid].vec.end(); ++itr) {
//	    		if(stageName.find("l2.mem_side") != std::string::npos && itr->stageName.find("l2.mem_side") != std::string::npos && itr->type == "Req") {
//	    			myFlag = true;
//	    		}
//	    	}
	    	map1[rid].vec.push_back(psi);
	    	if((stageName.find("icache") != std::string::npos || stageName.find("dcache") != std::string::npos) && isHit == 1) {
	    		map1[rid].l1Hit = 1;
	    	}
	    	else if(stageName.find("l2") != std::string::npos && isHit == 1) {
	    		map1[rid].l2Hit = 1;
	    	}
	    	else if(stageName.find("l3") != std::string::npos && isHit == 1) {
	    		map1[rid].l3Hit = 1;
	    	}
//	    	std::cout << "packet_info\t" << psi.atTick << "\t" << rid << "\t" << map1[rid].pktAddr <<"\t" << psi.stageName << "\t" << psi.type << "\n";
//	    	if((stageName.find("l2.mem_side") != std::string::npos || stageName.find("l3.cpu_side") != std::string::npos) && type.find("Req") != std::string::npos && !myFlag) {
////		    	std::cout << "cycleDelay_info\t" << psi.atTick << "\t" << rid << "\t" << map1[rid].pktAddr <<"\t" << psi.stageName << "\t" << psi.type << "\n";
//	    		calcTotalCyclesBusy(atTick, stageName, type);
//	    	}
    	}
	    else {
	    	map1[rid].finishTime = atTick;
//	    	std::cout << "calcDelayPath in IF!\n";
	    	calcDelayPath(rid);
	    }
	  }
	  else {
//		  std::cout << "map1 doesn't contain the packet!*****CHECK*******\n";
		  PacketInfo pi;
		  pi.masterId = 100;
		  pi.arrivalTime = atTick;
		  pi.pktAddr = pktAddr;
		  pi.finishTime = 0;
		  map1[rid] = pi;
		  PacketStageInfo psi;// = new PacketStageInfo();
		  psi.stageName = stageName;
		  psi.atTick = atTick;
		  psi.type = type;
		  map1[rid].vec.push_back(psi);
		  totalRequests++;
	    	if((stageName.find("icache") != std::string::npos || stageName.find("dcache") != std::string::npos) && isHit == 1) {
	    		map1[rid].l1Hit = 1;
	    	}
	    	else {
	    		map1[rid].l1Hit = 0;
	    	}
	    	if(stageName.find("l2") != std::string::npos && isHit == 1) {
	    		map1[rid].l2Hit = 1;
	    	}
	    	else {
	    		map1[rid].l2Hit = 0;
	    	}
	    	if(stageName.find("l3") != std::string::npos && isHit == 1) {
	    		map1[rid].l3Hit = 1;
	    	}
	    	else {
	    		map1[rid].l3Hit = 0;
	    	}
//		  std::cout << "packet_info\t" << psi.atTick << "\t" << rid << "\t" << map1[rid].pktAddr <<"\t" << psi.stageName << "\t" << psi.type << "\n";
	  }
	}
}
int gl = 500000;
void calcDelayPath(int rid) {
	bool flag[19];
	bool isInst = true;
	for(int i = 0; i < 19; i++) {
		flag[i] = false;
	}
//	if(gl-- == 0) {
//			printDelayPath();
//			gl = 500000;
//	}
//	printDelayPath();
//	std::cout << "caclDelayPath for rid" << rid << " from map1\n";
	if(map1.find(rid) != map1.end()){
//	    std::cout << "map1 contains the packet!\n";
    	PacketDelayInfo pdi;// = new PacketDelayInfo();
	    PacketInfo pi = map1[rid];
	    pdi.masterId = pi.masterId;
	    pdi.arrivalTime = pi.arrivalTime;
	    pdi.finishTime = pi.finishTime;
	    pdi.pktAddr = pi.pktAddr;
	    map2[rid] = pdi;
	    if(map1[rid].finishTime > map1[rid].arrivalTime) {
			 totalDelay = totalDelay + (map1[rid].finishTime - map1[rid].arrivalTime)/500;
			 totalRequestsFinished++;
		 }

//	    if(rid == 6907715) {
//	    	std::cout << "STOP!!\n";
//	    }

	    for(std::vector<PacketStageInfo>::iterator it = pi.vec.begin(); it != pi.vec.end(); ++it) {
	    	if(it->stageName.find("icache.cpu_side") != std::string::npos && !flag[0] && it->type == "Req") {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("icache.access") != std::string::npos && itr->type == "Req") {
	    				string stageName = "icache_access";
	    				addStageDelay(rid, it->atTick, itr->atTick, stageName, icacheDelay, totalL1);
	    				isInst = true;
	    				flag[0] = true;
	    				if(map1[rid].l1Hit == 1) {
	    					if(map1[rid].l2Hit == 1 || map1[rid].l3Hit == 1) {
	    						std::cout << "Cannot be hit in l2 and l3\n";
	    					}
	    					if(map1[rid].finishTime > map1[rid].arrivalTime) {
		    					 totalL1Hit.delay = totalL1Hit.delay + (map1[rid].finishTime - map1[rid].arrivalTime)/500;
		    					 totalL1Hit.numEntry++;
	    					}
	    				}
	    				//calcCacheDelay(pdpi.delay, 1);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}
	    	else if(it->stageName.find("dcache.cpu_side") != std::string::npos && !flag[1] && it->type == "Req") {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("dcache.access") != std::string::npos && itr->type == "Req") {
	    				string stageName = "dcache_access";
	    				addStageDelay(rid, it->atTick, itr->atTick, stageName, dcacheDelay, totalL1);
	    				isInst = false;
	    				flag[1] = true;
	    				if(map1[rid].l1Hit == 1) {
	    					if(map1[rid].l2Hit == 1 || map1[rid].l3Hit == 1) {
	    						std::cout << "Cannot be hit in l2 and l3\n";
	    					}
	    					if(map1[rid].finishTime > map1[rid].arrivalTime) {
		    					 totalL1Hit.delay = totalL1Hit.delay + (map1[rid].finishTime - map1[rid].arrivalTime)/500;
		    					 totalL1Hit.numEntry++;
	    					}
	    				}
//	    				calcCacheDelay(pdpi.delay, 2);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}
	    	else if(it->stageName.find("icache.mem_side") != std::string::npos && !flag[2] && it->type == "Req") {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("l2.cpu_side") != std::string::npos && itr->type == "Req") {
	    				string stageName = "l2_bus_access";
	    				addStageDelay(rid, it->atTick, itr->atTick, stageName, l2busDelay, totalL2Bus);
	    				isInst = true;
	    				flag[2] = true;
//	    			    if(map1[rid].finishTime > map1[rid].arrivalTime) {
//	    					 totalL2.delay = totalL2.delay + (map1[rid].finishTime - map1[rid].arrivalTime)/500;
//	    					 totalL2.numEntry++;
//	    				 }
	    				 totalIcacheRequestsL2++;
		    				if(map1[rid].l2Hit == 1) {
		    					if(map1[rid].l1Hit == 1 || map1[rid].l3Hit == 1) {
		    						std::cout << "Cannot be hit in l1 and l3\n";
		    					}
		    					if(map1[rid].finishTime > map1[rid].arrivalTime) {
			    					 totalL2Hit.delay = totalL2Hit.delay + (map1[rid].finishTime - map1[rid].arrivalTime)/500;
			    					 totalL2Hit.numEntry++;
		    					}
		    				}
//	    				 calcL2BusDelay(pdpi.delay);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}
	    	else if(it->stageName.find("dcache.mem_side") != std::string::npos && !flag[3] && it->type == "Req") {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("l2.cpu_side") != std::string::npos && itr->type == "Req") {
	    				string stageName = "l2_bus_access";
	    				addStageDelay(rid, it->atTick, itr->atTick, stageName, l2busDelay, totalL2Bus);
	    				isInst = false;
	    				flag[3] = true;
//	    			    if(map1[rid].finishTime > map1[rid].arrivalTime) {
//	    					 totalL2.delay = totalL2.delay + (map1[rid].finishTime - map1[rid].arrivalTime)/500;
//	    					 totalL2.numEntry++;
//	    			    }
		    				if(map1[rid].l2Hit == 1) {
		    					if(map1[rid].l1Hit == 1 || map1[rid].l3Hit == 1) {
		    						std::cout << "Cannot be hit in l1 and l3\n";
		    					}
		    					if(map1[rid].finishTime > map1[rid].arrivalTime) {
			    					 totalL2Hit.delay = totalL2Hit.delay + (map1[rid].finishTime - map1[rid].arrivalTime)/500;
			    					 totalL2Hit.numEntry++;
		    					}
		    				}
//	    				 calcL2BusDelay(pdpi.delay);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}
	    	else if(it->stageName.find("l2.cpu_side") != std::string::npos && !flag[4] && it->type == "Req") {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("l2.access") != std::string::npos && itr->type == "Req") {
	    				string stageName = "l2_access";
	    				addStageDelay(rid, it->atTick, itr->atTick, stageName, l2Delay, totalL2);
	    				flag[4] = true;
//	    				 if(counterL2-- == 0) {
//	    					 std::cout << "totalRequests - " << totalRequests << "\ttotalRequestsFinished - " << totalRequestsFinished << "\n";
//	    					 std::cout << "totalDelay of finished requests - " << totalDelay << "\ttotalRequests - " << totalRequestsFinished << "\n";
//	    					 std::cout << "TotalL2\n";
//	    					 printStats(totalL2);
//	    					 std::cout << "TotalL2Hit\n";
//	    					 printStats(totalL2Hit);
//	    					 std::cout << "TotalL2Bus\n";
//	    					 printStats(totalL2Bus);
//	    					 std::cout << "l2Delay\n";
//	    					 printMap(l2Delay);
//	    					 std::cout << "l2busDelay\n";
//	    					 printMap(l2busDelay);
//	    					 counterL2 = 500;
//	    				 }
//	    				calcCacheDelay(pdpi.delay, 3);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}
	    	else if(it->stageName.find("l2.mem_side") != std::string::npos && !flag[5] && it->type == "Req") {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("l3.cpu_side") != std::string::npos && itr->type == "Req") {
	    				string stageName = "l3_bus_access";
	    				addStageDelay(rid, it->atTick, itr->atTick, stageName, l3busDelay, totalL3Bus);
	    				flag[5] = true;
//	    			    if(map1[rid].finishTime > map1[rid].arrivalTime) {
//	    					 totalL3.delay = totalL3.delay + (map1[rid].finishTime - map1[rid].arrivalTime)/500;
//	    					 totalL3.numEntry++;
//	    			    }
	    			    if(isInst) {
	    					 totalIcacheRequestsL3++;
	    				 }
		    				if(map1[rid].l3Hit == 1) {
		    					if(map1[rid].l1Hit == 1 || map1[rid].l2Hit == 1) {
		    						std::cout << "Cannot be hit in l1 and l2\n";
		    					}
		    					if(map1[rid].finishTime > map1[rid].arrivalTime) {
			    					 totalL3Hit.delay = totalL3Hit.delay + (map1[rid].finishTime - map1[rid].arrivalTime)/500;
			    					 totalL3Hit.numEntry++;
		    					}
		    				}
//	    				calcL3BusDelay(pdpi.delay);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
//	    				if((pdpi.delay/500 )== 155) {
//	    					std::cout << "packets came together - 155 " << rid << "\t" << map2[rid].pktAddr << "\t" << itr->atTick << "\t" << it->atTick << "\n";
//	    				}
//	    				if((pdpi.delay/500 )== 124) {
//	    					std::cout << "packets came together - 124 " << rid << "\t" << map2[rid].pktAddr << "\t" << itr->atTick << "\t" << it->atTick << "\n";
//	    				}
	    				break;
	    			}
	    		}
	    	}
	    	else if(it->stageName.find("l3.cpu_side") != std::string::npos && !flag[6] && it->type == "Req") {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("l3.access") != std::string::npos && itr->type == "Req") {
	    				string stageName = "l3_access";
	    				addStageDelay(rid, it->atTick, itr->atTick, stageName, l3Delay, totalL3);
	    				flag[6] = true;
//	    				 if(counterL3-- == 0) {
//	    					 std::cout << "totalRequests - " << totalRequests << "\ttotalRequestsFinished - " << totalRequestsFinished << "\n";
//	    					 std::cout << "totalDelay of finished requests - " << totalDelay << "\ttotalRequests - " << totalRequestsFinished << "\n";
//	    					 std::cout << "TotalL3\n";
//	    					 printStats(totalL3);
//	    					 std::cout << "TotalL3Hit\n";
//	    					 printStats(totalL3Hit);
//	    					 std::cout << "TotalL3Bus\n";
//	    					 printStats(totalL3Bus);
//	    					 std::cout << "l3Delay\n";
//	    					 printMap(l3Delay);
//	    					 std::cout << "l3busDelay\n";
//	    					 printMap(l3busDelay);
//	    					 counterL3 = 500;
//	    				 }
//	    				calcCacheDelay(pdpi.delay, 4);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}
	    	else if(it->stageName.find("l3.mem_side") != std::string::npos && !flag[7] && it->type == "Req") {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("membus.master") != std::string::npos && itr->type == "Req") {
	    				string stageName = "membus_delay";
	    				addStageDelay(rid, it->atTick, itr->atTick, stageName, membusDelay, totalMem);
	    				flag[7] = true;
//	    			    if(map1[rid].finishTime > map1[rid].arrivalTime) {
//	    					 totalMem.delay = totalMem.delay + (map1[rid].finishTime - map1[rid].arrivalTime)/500;
//	    					 totalMem.numEntry++;
//	    			    }
//	    				calcMemBusDelay(pdpi.delay);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}
	    	else if((it->stageName.find("addingToReadQ") != std::string::npos || it->stageName.find("addingToWriteQ") != std::string::npos) && !flag[9]) {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("addingToRespQ") != std::string::npos || itr->stageName.find("WriteDoneEvent") != std::string::npos) {
	    				string stageName = "RWQDelay";
	    				addStageDelay(rid, it->atTick, itr->atTick, stageName, memRWQDelay, totalMemRWQ);
	    				flag[9] = true;
//	    				calcCacheDelay(pdpi.delay, 5);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}
	    	else if((it->stageName.find("addingToRespQ") != std::string::npos || it->stageName.find("WriteDoneEvent") != std::string::npos) && !flag[10]) {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("DRAM_Access") != std::string::npos) {
	    				string stageName = "RespQDelay";
	    				addStageDelay(rid, it->atTick, itr->atTick, stageName, memRespQDelay, totalMemRespQ);
	    				flag[10] = true;
//	    				calcCacheDelay(pdpi.delay,6);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}
	    	else if(it->stageName.find("DRAM_Access") != std::string::npos && !flag[11]) {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("DRAMRespTime") != std::string::npos) {
	    				string stageName = "mem_ctrl_responseDelay";
	    				addStageDelay(rid, it->atTick, itr->atTick, stageName, memAccessTime, totalMemAccess);
	    				flag[11] = true;
//	    				 if(counterMembus-- == 0) {
//	    					 std::cout << "totalRequests - " << totalRequests << "\ttotalRequestsFinished - " << totalRequestsFinished << "\n";
//	    					 std::cout << "totalDelay of finished requests - " << totalDelay << "\ttotalRequests - " << totalRequestsFinished << "\n";
//	    					 std::cout << "TotalMem\n";
//	    					 printStats(totalMem);
//	    					 std::cout << "TotalMemRWQ\n";
//	    					 printStats(totalMemRWQ);
//	    					 std::cout << "TotalMemRespQ\n";
//	    					 printStats(totalMemRespQ);
//	    					 std::cout << "TotalMemAccess\n";
//	    					 printStats(totalMemAccess);
////	    					 std::cout << "TotalMemRespTime\n";
////	    					 printStats(totalMemRespTime);
//	    					 std::cout << "membusDelay\n";
//	    					 printMap(membusDelay);
//	    					 std::cout << "memRWQDelay\n";
//	    					 printMap(memRWQDelay);
//	    					 std::cout << "memRespQDelay\n";
//	    					 printMap(memRespQDelay);
//	    					 std::cout << "memAccessTime\n";
//	    					 printMap(memAccessTime);
//	    					 counterMembus = 500;
//	    				 }
//	    				calcCacheDelay(pdpi.delay,8);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}
	    	else if(it->stageName.find("mem_ctrls.port") != std::string::npos && !flag[12] && it->type == "Resp") {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("l3.mem_side") != std::string::npos && itr->type == "Resp") {
	    				string stageName = "membus_delay_resp";
//	    				addStageDelay(rid, it->atTick, itr->atTick, stageName);
	    				flag[12] = true;
//	    				calcMemBusRDelay(pdpi.delay);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}
	    	else if(it->stageName.find("l3.mem_side") != std::string::npos && !flag[13] && it->type == "Resp") {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("l3.access") != std::string::npos && itr->type == "Resp") {
	    				string stageName = "l3_access_resp";
//	    				addStageDelay(rid, it->atTick, itr->atTick, stageName);
	    				flag[13] = true;
//	    				calcCacheDelay(pdpi.delay, 9);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}
	    	else if(it->stageName.find("l3.cpu_side") != std::string::npos && !flag[14] && it->type == "Resp") {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("l2.mem_side") != std::string::npos && itr->type == "Resp") {
	    				string stageName = "l3_busDelay_resp";
//	    				addStageDelay(rid, it->atTick, itr->atTick, stageName);
	    				flag[14] = true;
//	    				calcL3BusRDelay(pdpi.delay);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}
	    	else if(it->stageName.find("l2.mem_side") != std::string::npos && !flag[15] && it->type == "Resp") {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("l2.access") != std::string::npos && itr->type == "Resp") {
	    				string stageName = "l2_access_resp";
//	    				addStageDelay(rid, it->atTick, itr->atTick, stageName);
	    				flag[15] = true;
//	    				calcCacheDelay(pdpi.delay, 10);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}	    	else if(it->stageName.find("l2.cpu_side") != std::string::npos && !flag[16] && it->type == "Resp") {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("icache.mem_side") != std::string::npos && itr->type == "Resp") {
	    				string stageName = "l2_busDelay_resp";
//	    				addStageDelay(rid, it->atTick, itr->atTick, stageName);
	    				flag[16] = true;
//	    				calcL2BusRDelay(pdpi.delay);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}	    	else if(it->stageName.find("l2.cpu_side") != std::string::npos && !flag[17] && it->type == "Resp") {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("dcache.mem_side") != std::string::npos && itr->type == "Resp") {
	    				string stageName = "l2_busDelay_resp";
//	    				addStageDelay(rid, it->atTick, itr->atTick, stageName);
	    				flag[17] = true;
//	    				calcL2BusRDelay(pdpi.delay);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}	    	else if(it->stageName.find("icache.mem_side") != std::string::npos && !flag[18] && it->type == "Resp") {
	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    			if(itr->stageName.find("icache.access") != std::string::npos && itr->type == "Resp") {
	    				string stageName = "icache_access_resp";
//	    				addStageDelay(rid, it->atTick, itr->atTick, stageName);
	    				flag[18] = true;
//	    				calcCacheDelay(pdpi.delay, 11);
//	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    				break;
	    			}
	    		}
	    	}
	    	else if(it->stageName.find("dcache.mem_side") != std::string::npos && !flag[19] && it->type == "Resp") {
	    	    		for(std::vector<PacketStageInfo>::iterator itr = pi.vec.begin(); itr != pi.vec.end(); ++itr) {
	    	    			if(itr->stageName.find("dcache.access") != std::string::npos && itr->type == "Resp") {
	    	    				string stageName = "dcache_access_resp";
//	    	    				addStageDelay(rid, it->atTick, itr->atTick, stageName);
	    	    				flag[19] = true;
//	    	    				calcCacheDelay(pdpi.delay, 12);
//	    	    				std::cout << "final_delay_values\t" << rid << "\t" << map2[rid].pktAddr << "\t" << pdpi.stageName << "\t" << pdpi.delay << "\n";
	    	    				break;
	    	    			}
	    	    		}
	    	    	}
	    }
	    }
//	  }
	 else {
		 std::cout << "map1 doesn't contain the packet!*****CHECK calcDelayPath*******\n";
//		 std::cout << "packet_info\t" << rid << "\t" << atTick << "\t" << pktAddr <<"\t" << stageName << "\t" << type << "\n";
	 }
//	std::cout << "erasing rid" << rid << " from map1\n";
//		printStageDelayPath();
	map1.erase(rid);
	map2.erase(rid);

//	printStageDelayPath();
}

//---------CHANGED-----------

EventQueue *
getEventQueue(uint32_t index)
{
    while (numMainEventQueues <= index) {
        numMainEventQueues++;
        mainEventQueue.push_back(
            new EventQueue(csprintf("MainEventQueue-%d", index)));
    }

    return mainEventQueue[index];
}

#ifndef NDEBUG
Counter Event::instanceCounter = 0;
#endif

Event::~Event()
{
    assert(!scheduled());
    flags = 0;
}

const std::string
Event::name() const
{
#ifndef NDEBUG
    return csprintf("Event_%d", instance);
#else
    return csprintf("Event_%x", (uintptr_t)this);
#endif
}


Event *
Event::insertBefore(Event *event, Event *curr)
{
    // Either way, event will be the top element in the 'in bin' list
    // which is the pointer we need in order to look into the list, so
    // we need to insert that into the bin list.
    if (!curr || *event < *curr) {
        // Insert the event before the current list since it is in the future.
        event->nextBin = curr;
        event->nextInBin = NULL;
    } else {
        // Since we're on the correct list, we need to point to the next list
        event->nextBin = curr->nextBin;  // curr->nextBin can now become stale

        // Insert event at the top of the stack
        event->nextInBin = curr;
    }

    return event;
}

void
EventQueue::insert(Event *event)
{
    // Deal with the head case
    if (!head || *event <= *head) {
        head = Event::insertBefore(event, head);
        return;
    }

    // Figure out either which 'in bin' list we are on, or where a new list
    // needs to be inserted
    Event *prev = head;
    Event *curr = head->nextBin;
    while (curr && *curr < *event) {
        prev = curr;
        curr = curr->nextBin;
    }

    // Note: this operation may render all nextBin pointers on the
    // prev 'in bin' list stale (except for the top one)
    prev->nextBin = Event::insertBefore(event, curr);
}

Event *
Event::removeItem(Event *event, Event *top)
{
    Event *curr = top;
    Event *next = top->nextInBin;

    // if we removed the top item, we need to handle things specially
    // and just remove the top item, fixing up the next bin pointer of
    // the new top item
    if (event == top) {
        if (!next)
            return top->nextBin;
        next->nextBin = top->nextBin;
        return next;
    }

    // Since we already checked the current element, we're going to
    // keep checking event against the next element.
    while (event != next) {
        if (!next)
            panic("event not found!");

        curr = next;
        next = next->nextInBin;
    }

    // remove next from the 'in bin' list since it's what we're looking for
    curr->nextInBin = next->nextInBin;
    return top;
}

void
EventQueue::remove(Event *event)
{
    if (head == NULL)
        panic("event not found!");

    assert(event->queue == this);

    // deal with an event on the head's 'in bin' list (event has the same
    // time as the head)
    if (*head == *event) {
        head = Event::removeItem(event, head);
        return;
    }

    // Find the 'in bin' list that this event belongs on
    Event *prev = head;
    Event *curr = head->nextBin;
    while (curr && *curr < *event) {
        prev = curr;
        curr = curr->nextBin;
    }

    if (!curr || *curr != *event)
        panic("event not found!");

    // curr points to the top item of the the correct 'in bin' list, when
    // we remove an item, it returns the new top item (which may be
    // unchanged)
    prev->nextBin = Event::removeItem(event, curr);
}

Event *
EventQueue::serviceOne()
{
    std::lock_guard<EventQueue> lock(*this);
    Event *event = head;
    Event *next = head->nextInBin;
    event->flags.clear(Event::Scheduled);
//    Tick time;

//    std::cout << curTick() << "\t" << event->name() << "\t" << event->description() << "\t EventQ cur_tick: " << getCurTick() << "\thead instance: " << getHead()->instance << "\n";
    DPRINTF(PktTrace, "%s: ServicingEvent:\t %s: \t %d\n", event->name(), event->description(), getHead()->instance);
    //-----CHANGED----
//    		std::cout << "Num of Main queues=" << numMainEventQueues << "\n";
//    		dumpMainQueue();
//    		std::cout << "serviceOne(): event_instance(ID)= " << event->instance << "\tEvent name=" << event->name() <<"\n";
//    		printf("when (clock) %lu \n", event->when());
//    		std::cout << "Event flag=" << std::hex << event->flags << "\n";
    	    //std::cout << "doSimLoop(): next= " << next << "\n";
    	//-----CHANGED----


    if (next) {
        // update the next bin pointer since it could be stale
        next->nextBin = head->nextBin;

        // pop the stack
        head = next;
    } else {
        // this was the only element on the 'in bin' list, so get rid of
        // the 'in bin' list and point to the next bin list
        head = head->nextBin;
    }

    // handle action
    if (!event->squashed()) {
        // forward current cycle to the time when this event occurs.
        setCurTick(event->when());

        //-----CHANGED----
//        time = curTick();
        //std::cout<<time<<"\n";
//        std::cout << curTick() << "\t" << time << "\t" << event->instance << "\t" << event->instanceCounter << "\t" << event->name() << "\t" << event->description() << "\n" ;

        if(strcmp(event->description(),"EventWrapped") == 0) {
        	//std::cout << "memory hierarchy event\n";
//        	std::cout << "delay_path \tStage1 " << curTick() << "\n";
        }
        //-----CHANGED----

//        std::cout << "-------------------------- time---------------------------\n";
//        std::cout << curTick() << "\n";
        event->process();
        if (event->isExitEvent()) {
            assert(!event->flags.isSet(Event::AutoDelete) ||
                   !event->flags.isSet(Event::IsMainQueue)); // would be silly
            return event;
        }
    } else {
        event->flags.clear(Event::Squashed);
    }

    if (event->flags.isSet(Event::AutoDelete) && !event->scheduled())
        delete event;

    return NULL;
}

void
Event::serialize(CheckpointOut &cp) const
{
    SERIALIZE_SCALAR(_when);
    SERIALIZE_SCALAR(_priority);
    short _flags = flags;
    SERIALIZE_SCALAR(_flags);
}

void
Event::unserialize(CheckpointIn &cp)
{
    assert(!scheduled());

    UNSERIALIZE_SCALAR(_when);
    UNSERIALIZE_SCALAR(_priority);

    FlagsType _flags;
    UNSERIALIZE_SCALAR(_flags);

    // Old checkpoints had no concept of the Initialized flag
    // so restoring from old checkpoints always fail.
    // Events are initialized on construction but original code
    // "flags = _flags" would just overwrite the initialization.
    // So, read in the checkpoint flags, but then set the Initialized
    // flag on top of it in order to avoid failures.
    assert(initialized());
    flags = _flags;
    flags.set(Initialized);

    // need to see if original event was in a scheduled, unsquashed
    // state, but don't want to restore those flags in the current
    // object itself (since they aren't immediately true)
    if (flags.isSet(Scheduled) && !flags.isSet(Squashed)) {
        flags.clear(Squashed | Scheduled);
    } else {
        DPRINTF(Checkpoint, "Event '%s' need to be scheduled @%d\n",
                name(), _when);
    }
}

void
EventQueue::checkpointReschedule(Event *event)
{
    // It's safe to call insert() directly here since this method
    // should only be called when restoring from a checkpoint (which
    // happens before thread creation).
    if (event->flags.isSet(Event::Scheduled))
        insert(event);
}
void
EventQueue::dump() const
{
    cprintf("============================================================\n");
    cprintf("EventQueue Dump  (cycle %d)\n", curTick());
    cprintf("------------------------------------------------------------\n");

    if (empty())
        cprintf("<No Events>\n");
    else {
        Event *nextBin = head;
        while (nextBin) {
            Event *nextInBin = nextBin;
            while (nextInBin) {
                nextInBin->dump();
                nextInBin = nextInBin->nextInBin;
            }

            nextBin = nextBin->nextBin;
        }
    }

    cprintf("============================================================\n");
}

bool
EventQueue::debugVerify() const
{
    std::unordered_map<long, bool> map;

    Tick time = 0;
    short priority = 0;

    Event *nextBin = head;
    while (nextBin) {
        Event *nextInBin = nextBin;
        while (nextInBin) {
            if (nextInBin->when() < time) {
                cprintf("time goes backwards!");
                nextInBin->dump();
                return false;
            } else if (nextInBin->when() == time &&
                       nextInBin->priority() < priority) {
                cprintf("priority inverted!");
                nextInBin->dump();
                return false;
            }

            if (map[reinterpret_cast<long>(nextInBin)]) {
                cprintf("Node already seen");
                nextInBin->dump();
                return false;
            }
            map[reinterpret_cast<long>(nextInBin)] = true;

            time = nextInBin->when();
            priority = nextInBin->priority();

            nextInBin = nextInBin->nextInBin;
        }

        nextBin = nextBin->nextBin;
    }

    return true;
}

Event*
EventQueue::replaceHead(Event* s)
{
    Event* t = head;
    head = s;
    return t;
}

void
dumpMainQueue()
{
    for (uint32_t i = 0; i < numMainEventQueues; ++i) {
        mainEventQueue[i]->dump();
    }
}


const char *
Event::description() const
{
    return "generic";
}

void
Event::trace(const char *action)
{
    // This DPRINTF is unconditional because calls to this function
    // are protected by an 'if (DTRACE(Event))' in the inlined Event
    // methods.
    //
    // This is just a default implementation for derived classes where
    // it's not worth doing anything special.  If you want to put a
    // more informative message in the trace, override this method on
    // the particular subclass where you have the information that
    // needs to be printed.
    DPRINTFN("%s event %s @ %d\n", description(), action, when());
}

void
Event::dump() const
{
    cprintf("Event %s (%s)\n", name(), description());
    cprintf("Flags: %#x\n", flags);
#ifdef EVENTQ_DEBUG
    cprintf("Created: %d\n", whenCreated);
#endif
    if (scheduled()) {
#ifdef EVENTQ_DEBUG
        cprintf("Scheduled at  %d\n", whenScheduled);
#endif
        cprintf("Scheduled for %d, priority %d\n", when(), _priority);
    } else {
        cprintf("Not Scheduled\n");
    }
}

EventQueue::EventQueue(const string &n)
    : objName(n), head(NULL), _curTick(0)
{
}

void
EventQueue::asyncInsert(Event *event)
{
    async_queue_mutex.lock();
    async_queue.push_back(event);
    async_queue_mutex.unlock();
}

void
EventQueue::handleAsyncInsertions()
{
    assert(this == curEventQueue());
    async_queue_mutex.lock();

    while (!async_queue.empty()) {
        insert(async_queue.front());
        async_queue.pop_front();
    }

    async_queue_mutex.unlock();
}
