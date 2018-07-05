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
 * Declaration of a high-level queue structure
 */

#ifndef __MEM_CACHE_QUEUE_HH__
#define __MEM_CACHE_QUEUE_HH__

#include <cassert>

#include "base/trace.hh"
#include "debug/Drain.hh"
#include "debug/CacheStatus.hh"
#include "mem/cache/queue_entry.hh"
#include "sim/drain.hh"
//#include "mem/cache/base.hh"


/**
 * A high-level queue interface, to be used by both the MSHR queue and
 * the write buffer.
 */
template<class Entry>
class Queue : public Drainable
{
  protected:
    /** Local label (for functional print requests) */
    const std::string label;

    /**
     * The total number of entries in this queue. This number is set
     * as the number of entries requested plus any reserve. This
     * allows for the same number of effective entries while still
     * maintaining an overflow reserve.
     */
    const int numEntries;

    /**
     * The number of entries to hold as a temporary overflow
     * space. This is used to allow temporary overflow of the number
     * of entries as we only check the full condition under certain
     * conditions.
     */
    const int numReserve;

    /**  Actual storage. */
    std::vector<Entry> entries;

  public:
    /** Holds pointers to all allocated entries. */
    typename Entry::List allocatedList;

    /** Holds pointers to entries that haven't been sent downstream. */
    typename Entry::List readyList;
  protected:
    /** Holds non allocated entries. */
    typename Entry::List freeList;

    typename Entry::Iterator addToReadyList(Entry* entry)
    {
    	  //-----CHANGED----
//    	std::cout << "name() in queue.hh " << name() << "\n";
//    	std::cout << "mshr entry: " << entry << "\t" << "mshr order " << entry->order << "\n";
//    	std::cout << "targetList size: " << entry->targets.size() << "\n";
//    	for (auto i = entry->targets.begin(); i != entry->targets.end(); ++i) {
//    		std::cout << (*i).pkt->req << "\t" << (*i).pkt->req->contextId() << "\t" << (*i).pkt->getAddr() << "\tmasterId() " << (*i).pkt->req->masterId() << "\n";
//    	}
    	  //-----CHANGED----

        if (readyList.empty() ||
            readyList.back()->readyTime <= entry->readyTime) {
            return readyList.insert(readyList.end(), entry);
        }

        for (auto i = readyList.begin(); i != readyList.end(); ++i) {
            if ((*i)->readyTime > entry->readyTime) {
                return readyList.insert(i, entry);
            }
        }
        assert(false);
        return readyList.end();  // keep stupid compilers happy
    }

    /** The number of entries that are in service. */
    int _numInService;

    /** The number of currently allocated entries. */
    int allocated;

  public:


    //--------CHANGED-------
    int totalMSHRentries;
    uint64_t totalMSHRDelay;
    //--------CHANGED-------

    /**
     * Create a queue with a given number of entries.
     *
     * @param num_entries The number of entries in this queue.
     * @param reserve The extra overflow entries needed.
     */
    Queue(const std::string &_label, int num_entries, int reserve) :
        label(_label), numEntries(num_entries + reserve),
        numReserve(reserve), entries(numEntries), _numInService(0),
        allocated(0)
    {
        for (int i = 0; i < numEntries; ++i) {
            freeList.push_back(&entries[i]);
        }
        //-----CHANGED----
    	totalMSHRentries = 0;
    	totalMSHRDelay = 0;
    	std::cout << "Queue created for the entry above\n";
        //-----CHANGED----
    }

    bool isEmpty() const
    {
        return allocated == 0;
    }

    bool isFull() const
    {
        return (allocated >= numEntries - numReserve);
    }

    int numInService() const
    {
        return _numInService;
    }

    /**
     * Find the first WriteQueueEntry that matches the provided address.
     * @param blk_addr The block address to find.
     * @param is_secure True if the target memory space is secure.
     * @return Pointer to the matching WriteQueueEntry, null if not found.
     */
    Entry* findMatch(Addr blk_addr, bool is_secure) const
    {
        for (const auto& entry : allocatedList) {
            // we ignore any entries allocated for uncacheable
            // accesses and simply ignore them when matching, in the
            // cache we never check for matches when adding new
            // uncacheable entries, and we do not want normal
            // cacheable accesses being added to an WriteQueueEntry
            // serving an uncacheable access
            if (!entry->isUncacheable() && entry->blkAddr == blk_addr &&
                entry->isSecure == is_secure) {
                return entry;
            }
        }
        return nullptr;
    }

    bool checkFunctional(PacketPtr pkt, Addr blk_addr)
    {
        pkt->pushLabel(label);
        for (const auto& entry : allocatedList) {
            if (entry->blkAddr == blk_addr && entry->checkFunctional(pkt)) {
                pkt->popLabel();
                return true;
            }
        }
        pkt->popLabel();
        return false;
    }

    /**
     * Find any pending requests that overlap the given request.
     * @param blk_addr Block address.
     * @param is_secure True if the target memory space is secure.
     * @return A pointer to the earliest matching WriteQueueEntry.
     */
    Entry* findPending(Addr blk_addr, bool is_secure) const
    {
        for (const auto& entry : readyList) {
            if (entry->blkAddr == blk_addr && entry->isSecure == is_secure) {
                return entry;
            }
        }
        return nullptr;
    }

    /**
     * Returns the WriteQueueEntry at the head of the readyList.
     * @return The next request to service.
     */
    Entry* getNext() const
    {
        if (readyList.empty() || readyList.front()->readyTime > curTick()) {
            return nullptr;
        }

        //--------CHANGED--------
//        shuffleReadyList();
        //--------CHANGED--------

        return readyList.front();
    }

    Tick nextReadyTime() const
    {
        return readyList.empty() ? MaxTick : readyList.front()->readyTime;
    }

    /**
     * Removes the given entry from the queue. This places the entry
     * on the free list.
     *
     * @param entry
     */
    void deallocate(Entry *entry)
    {
    	//-----CHANGED----
    	totalMSHRDelay += curTick() - entry->arrivalTime;
    	//-----CHANGED----

        allocatedList.erase(entry->allocIter);
        freeList.push_front(entry);
        allocated--;
        if (entry->inService) {
            _numInService--;
        } else {
//        	std::cout << entry->getTarget()->pkt->getAddr() << " readyList.erase at " << curTick() << "\n";
//        	std::cout << "readyList size before erase: " << readyList.size() << "\n";
            readyList.erase(entry->readyIter);
//        	std::cout << "readyList size after erase: " << readyList.size() << "\n";
        }
        entry->deallocate();

        DPRINTF(CacheStatus," (deleted): %d: \n", readyList.size());

//        std::cout << curTick() <<"  ----------MSHR contains-----------\n";
//            std::cout << "queue size: " << readyList.size() << "\n";
//            for (auto i = readyList.begin(); i != readyList.end(); ++i) {
//            	std::cout << curTick() << "\tmshr " << *i << "\t" << (*i)->readyTime << "\n";
//            }
        if (drainState() == DrainState::Draining && allocated == 0) {
            // Notify the drain manager that we have completed
            // draining if there are no other outstanding requests in
            // this queue.
            DPRINTF(Drain, "Queue now empty, signalling drained\n");
            signalDrainDone();
        }
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

    void deallocateL3RQ(Entry *entry, bool isWB, std::string cacheName)
    {
        allocatedList.erase(entry->allocIterGlobal);
        readyList.erase(entry->readyIterGlobal);
        if(isWB) {

     	   int id = readCoreId(cacheName);
//    	   std::cout << curTick() << "\t" << int(curTick()/500) << "\t" << name() << "\t" << cacheName << "\t(WB deallocate)core id " << id << "\t" << entry << "\n";
     	   setPriority(id, -2);
        }
        else {
//     	   std::cout << curTick() << "\t" << int(curTick()/500) << "\t" << name() << "\t" << cacheName << "\t(MSHR deallocate)core id " << entry->getTarget()->pkt->req->masterId() << "\t" << entry << "\n";
        	setPriority(entry->getTarget()->pkt->req->masterId(), -1);
        }


//        std::cout << "WB ready list size after deallocation: " << readyList.size() << "\n";
//
//           for (auto i = readyList.begin(); i != readyList.end(); ++i) {
//           	std::cout << curTick() << "\twb_entry " << *i << "\t" <<  (*i)->readyTime << "\n";
//           }
//
//           std::cout << "WB allocated list size after deallocation: " << allocatedList.size() << "\n";
//
//           for (auto i = allocatedList.begin(); i != allocatedList.end(); ++i) {
//            	std::cout << curTick() << "\twb_entry " << *i << "\t" <<  (*i)->readyTime << "\n";
//           }
    }


    DrainState drain() override
    {
        return allocated == 0 ? DrainState::Drained : DrainState::Draining;
    }


    //-----CHANGED------

    void shuffleReadyList() {
        std::cout << "shuffling readyList " << "\n";
        for (auto i = readyList.begin(); i != readyList.end(); ++i) {
        	if((*i)->readyTime <= curTick()) {
        		if ((*i)->getTarget()->pkt->req->masterId() == 9 || (*i)->getTarget()->pkt->req->masterId() == 10) {
        			readyList.erase((*i)->readyIter);
        			(*i)->readyIter = readyList.insert(readyList.begin(), (*i));
        			break;
        		}
        	}
        	else {
        		break;
        	}
        }
    }


    void dump()
    {
//        cprintf("============================================================\n");
//        cprintf("MSHRQueue Dump  (cycle %d)\n", curTick());
//        cprintf("------------------------------------------------------------\n");

        if (allocatedList.empty() && readyList.empty())
            cprintf("<No Events>\n");
        else {
//        	std::cout << "allocatedList size: " << allocatedList.size() << "\n";
//            cprintf("------------------------------------------------------------\n");
        	for (auto& entry : allocatedList) {
            	entry->print_mshr();
        	}
        	std::cout << "readyList size: " << readyList.size() << "\n";
//            cprintf("------------------------------------------------------------\n");
//        	printf("printing readyList\n");
        	printReadyList();
//        	printf("printing done!!\n");
        	for (auto& entry : readyList) {
        		entry->print_mshr();
        	}
//        	std::cout << "freeList size: " << freeList.size() << "\n";
//            cprintf("------------------------------------------------------------\n");
        	for (auto& entry : freeList) {
        		entry->print_mshr();
        	}
        }

//        cprintf("============================================================\n");
    }

    void printReadyList() {
        int count = 0;
        uint64_t rTime = 0;
        auto i = readyList.begin();
        rTime = (*i)->readyTime;
      //  printf("recvTime %lu, readyTime %lu \t", (*i)->recvTime, (*i)->readyTime);
        i++;
        for (; i != readyList.end(); ++i) {
        //	printf("recvTime %lu, readyTime %lu \t", (*i)->recvTime, (*i)->readyTime);
        	if ((*i)->readyTime == rTime) {
        		count++;
        	}
        	else {
        		rTime = (*i)->readyTime;
        		count = 0;
        	}
        	if(count > 0) {
//        		printf("'count' is more than 0\n");
        		std::cout << "count is " << count << "\n";
        		break;
        	}
        }
//        shuffleReadyList();
    }

    int ret_numEntries()
    {
        return numEntries;
    }


    	void print_queue() {
                    std::cout << "Allocated entries "<< allocated << "\t entries in service="<< _numInService <<"\n";
                    for (const auto& entry : allocatedList) {
                  	  std::cout << "entry->blkAddr " << entry->blkAddr << "\n";
                  	  //std::cout << "mshrQueue next ready time=" << mshrQueue.nextReadyTime() << "\n";
                    }
    }
          //-----CHANGED----

};

#endif //__MEM_CACHE_QUEUE_HH__
