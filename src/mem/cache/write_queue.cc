/*
 * Copyright (c) 2012-2013, 2015-2016 ARM Limited
 * All rights reserved.
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2003-2005 The Regents of The University of Michigan
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
 * Authors: Erik Hallnor
 *          Andreas Sandberg
 *          Andreas Hansson
 */

/** @file
 * Definition of WriteQueue class functions.
 */

#include "mem/cache/write_queue.hh"

using namespace std;

WriteQueue::WriteQueue(const std::string &_label,
                       int num_entries, int reserve)
    : Queue<WriteQueueEntry>(_label, num_entries, reserve)
      {	//std::cout << "Queue created for the entry type WriteQueue, has " << num_entries << " entries\n";
      }

WriteQueueEntry *
WriteQueue::allocate(Addr blk_addr, unsigned blk_size, PacketPtr pkt,
                    Tick when_ready, Counter order)
{
    assert(!freeList.empty());
    WriteQueueEntry *entry = freeList.front();
    assert(entry->getNumTargets() == 0);
    freeList.pop_front();

    entry->allocate(blk_addr, blk_size, pkt, when_ready, order);
    entry->allocIter = allocatedList.insert(allocatedList.end(), entry);
    entry->readyIter = addToReadyList(entry);

    allocated += 1;
    return entry;
}

int readCoreIdWB(std::string name) {		//reading first integer from a string as a core id

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


WriteQueueEntry *
WriteQueue::allocateL3RQ(WriteQueueEntry *wb_entry, std::string cacheName)
{
	int Qsize = 0;
//	if(timer < 11) {
	std::cout << "WB entry " << wb_entry << "\n";
        for (auto i = readyList.begin(); i != readyList.end(); ++i) {
	        	std::cout << "wb_entry " << *i << "\t" <<  curTick() << "\t" << (*i)->readyTime << "\n";
        	if((*i)->readyTime <= curTick()) {
        		Qsize++;
        	}
        	else {
        		continue;
        	}
        }
        std::ostringstream stringStream;
        stringStream << wb_entry;
        std::string addr_wb_entry = stringStream.str();
//        std::string buffer;
//        sprintf (buffer, "Little %llx", wb_entry);
//        std::cout << "LITTLE " << buffer << "\n";
	calcGQSize(cacheName+"GlobalWB", addr_wb_entry, curTick(), wb_entry->readyTime, Qsize, "I");
	wb_entry->allocIterGlobal = allocatedList.insert(allocatedList.end(), wb_entry);
	wb_entry->readyIterGlobal = addToReadyList(wb_entry);
    Qsize = 0;

	   int id = readCoreId(cacheName);
//	   std::cout << curTick() << "\t" << int(curTick()/500) << "\t" << name() << "\t" << cacheName << "\t(WB allocate)core id " << id << "\t" << wb_entry << "\t" <<  wb_entry->readyTime << "\n";
	   setPriority(id, 2);

//    std::cout << "WB ready list size: " << readyList.size() << "\n";
//
//    for (auto i = readyList.begin(); i != readyList.end(); ++i) {
//    	std::cout << curTick() << "\twb_entry " << *i << "\t" <<  (*i)->readyTime << "\n";
//    }
//
//    std::cout << "WB allocated list size: " << allocatedList.size() << "\n";
//
//    for (auto i = allocatedList.begin(); i != allocatedList.end(); ++i) {
//     	std::cout << curTick() << "\twb_entry " << *i << "\t" <<  (*i)->readyTime << "\n";
//    }

//	std::cout << wb_entry->readyTime << " Incremented GlobalReadyList size " << Qsize << "\n";
//	timer++;
//	}
//    if (mshr->getTarget()->pkt->req->masterId() == 9 || mshr->getTarget()->pkt->req->masterId() == 10) {
//    	schedC3 = allocatedList.size();
//    }
//    else {
//    	schedC4 = allocatedList.size();
//    }
//    schedTRL3 = schedC1 + schedC2 + schedC3 + schedC4;
    return wb_entry;

}


void
WriteQueue::markInService(WriteQueueEntry *entry)
{
    // for a normal eviction, such as a writeback or a clean evict,
    // there is no more to do as we are done from the perspective of
    // this cache, and for uncacheable write we do not need the entry
    // as part of the response handling
    entry->popTarget();
    deallocate(entry);
}
