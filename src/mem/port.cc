/*
 * Copyright (c) 2012,2015 ARM Limited
 * All rights reserved
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
 * Copyright (c) 2002-2005 The Regents of The University of Michigan
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
 *          Andreas Hansson
 *          William Wang
 */

/**
 * @file
 * Port object definitions.
 */
#include "mem/port.hh"
#include "debug/DelayPath.hh"
#include "base/trace.hh"
#include "mem/mem_object.hh"

Port::Port(const std::string &_name, MemObject& _owner, PortID _id)
    : portName(_name), id(_id), owner(_owner)
{
}

Port::~Port()
{
}

BaseMasterPort::BaseMasterPort(const std::string& name, MemObject* owner,
                               PortID _id)
    : Port(name, *owner, _id), _baseSlavePort(NULL)
{
}

BaseMasterPort::~BaseMasterPort()
{
}

BaseSlavePort&
BaseMasterPort::getSlavePort() const
{
    if (_baseSlavePort == NULL)
        panic("Cannot getSlavePort on master port %s that is not connected\n",
              name());

    return *_baseSlavePort;
}

bool
BaseMasterPort::isConnected() const
{
    return _baseSlavePort != NULL;
}

BaseSlavePort::BaseSlavePort(const std::string& name, MemObject* owner,
                             PortID _id)
    : Port(name, *owner, _id), _baseMasterPort(NULL)
{
}

BaseSlavePort::~BaseSlavePort()
{
}

BaseMasterPort&
BaseSlavePort::getMasterPort() const
{
    if (_baseMasterPort == NULL)
        panic("Cannot getMasterPort on slave port %s that is not connected\n",
              name());

    return *_baseMasterPort;
}

bool
BaseSlavePort::isConnected() const
{
    return _baseMasterPort != NULL;
}

/**
 * Master port
 */
MasterPort::MasterPort(const std::string& name, MemObject* owner, PortID _id)
    : BaseMasterPort(name, owner, _id), _slavePort(NULL)
{
}

MasterPort::~MasterPort()
{
}

void
MasterPort::bind(BaseSlavePort& slave_port)
{
    // bind on the level of the base ports
    _baseSlavePort = &slave_port;

    // also attempt to base the slave to the appropriate type
    SlavePort* cast_slave_port = dynamic_cast<SlavePort*>(&slave_port);

    // if this port is compatible, then proceed with the binding

//    std::cout << "bind: _masterPort id=" << this->getId() << "_masterPort name=" << this->name() << "\n";
//    std::cout << "bind: _baseSlavePort id=" << _baseSlavePort->getId() << "_baseSlavePort name=" << _baseSlavePort->name() << "\n";
   // std::cout << "bind: cast_slave_port id=" << cast_slave_port->getId() << "cast_slave_port name=" << cast_slave_port->name() << "\n";
    //std::cout << "bind: cast_slave_port=" << cast_slave_port << "\t ";

    if (cast_slave_port != NULL) {
        // master port keeps track of the slave port
        _slavePort = cast_slave_port;
        // slave port also keeps track of master port
        _slavePort->bind(*this);
    } else {
        fatal("Master port %s cannot bind to %s\n", name(),
              slave_port.name());
    }
}

void
MasterPort::unbind()
{
    if (_slavePort == NULL)
        panic("Attempting to unbind master port %s that is not connected\n",
              name());
    _slavePort->unbind();
    _slavePort = NULL;
    _baseSlavePort = NULL;
}

AddrRangeList
MasterPort::getAddrRanges() const
{
    return _slavePort->getAddrRanges();
}

Tick
MasterPort::sendAtomic(PacketPtr pkt)
{
    assert(pkt->isRequest());
    return _slavePort->recvAtomic(pkt);
}

void
MasterPort::sendFunctional(PacketPtr pkt)
{
    assert(pkt->isRequest());
    return _slavePort->recvFunctional(pkt);
}

void
MasterPort::addPort2Q(PacketPtr pkt, uint64_t portType)
{
//	std::cout << "Trace masterport::getcachestatus\n";
    return _slavePort->addPort2Queue(pkt, portType);

}

bool
MasterPort::sendTimingReq(PacketPtr pkt)
{
    assert(pkt->isRequest());

    //    -----CHANGED----
//    if(pkt->getAddr() == 960){
//    	std::cout << curTick() <<"\t" << pkt->getAddr() << "\tPacketQueue::schedSendTiming()\t" << name() << "\n";
//    	std::cout << "\t\theaderDelay " <<pkt->headerDelay << "\tpayloadDelay " << pkt->payloadDelay << "\n";
//    	//	    	std::cout << "Packet=" << pkt->getAddr() << "\tCache::recvTimingResp()\t" << name() << "\tcurTick: "<< curTick() << "\n";
//    	}
//    //    -----CHANGED----
//    //    -----CHANGED----
//        if(pkt->getAddr() == 960){
//        	std::cout << curTick() << "\t" << pkt->getAddr() << "\tMasterPort::sendTimingReq()\t" << name() << "\n";
//        	std::cout << "SlavePort- " << _slavePort->name() << "\n";
//        	std::cout << "\t\t\tpkt headerDelay " << pkt->headerDelay << "\tpayLoadDelay " << pkt->payloadDelay << "\n";
//        }
//        if((!name().compare("system.membus.master[4]")) && pkt->getAddr() == 960){
//        	std::cout << "Going to access membus!!\n";
//        }
    //    -----CHANGED----

    //-----CHANGED----
//    std::cout << "Packet addr=" << pkt->getAddr() << "\n";
//          std::cout << "sendTimingReq(): MasterPort sending timing req to slave port=" << _slavePort->name() << " with id=" << _slavePort->getId() << ", owner of the port=" << _slavePort->owner.name() << "\n";
    //-----CHANGED----

    //-----CHANGED----
//      std::cout << "delay_path \tStage1 \t" << curTick() << "\t" << pkt->req->rid << "\t" << pkt->getAddr() << "\t" << name() << "\tRequest" << "\n";
      addToDelayPath(pkt->req->rid, pkt->getAddr(), curTick(), name(), "Req", false, 2);
      //-----CHANGED----
//      std::cout << curTick() << "\tTrace " << name() << " sendTimingReq(port.cc):\t" << pkt->getAddr() << "\t" << pkt->req->rid << "\t" << pkt->req->getseqNum() << "\n";

    return _slavePort->recvTimingReq(pkt);
}

bool
MasterPort::sendTimingSnoopResp(PacketPtr pkt)
{
    assert(pkt->isResponse());

    //-----CHANGED----
//    std::cout << "delay_path \tStage1 \t" << curTick() << "\t" << pkt->req->rid << "\t" << pkt->getAddr() << "\t" << name() << "\tSnoopResp" << "\n";
    addToDelayPath(pkt->req->rid, pkt->getAddr(), curTick(), name(), "SnoopResp", false, 2);
    //-----CHANGED----
//    std::cout << curTick() << "\tTrace " << name() << " sendTimingSnoopResp(port.cc):\t" << pkt->getAddr() << "\t" << pkt->req->rid << "\t" << pkt->req->getseqNum() << "\n";

    return _slavePort->recvTimingSnoopResp(pkt);
}

void
MasterPort::sendRetryResp()
{
	  //    -----CHANGED----
//	        if(pkt->getAddr() == 960){
//	        	std::cout << "Packet=" << pkt->getAddr() << "\tMasterPort::sendRetryResp()\t" << name() << "\tcurTick: "<< curTick() << "\n";
//	        }
	    //    -----CHANGED----
//    std::cout << curTick() << "\tTrace " << name() << " sendRetryResp(port.cc):\n";

    _slavePort->recvRespRetry();
}

void
MasterPort::printAddr(Addr a)
{
    Request req(a, 1, 0, Request::funcMasterId);
    Packet pkt(&req, MemCmd::PrintReq);
    Packet::PrintReqState prs(std::cerr);
    pkt.senderState = &prs;

    sendFunctional(&pkt);
}

/**
 * Slave port
 */
SlavePort::SlavePort(const std::string& name, MemObject* owner, PortID id)
    : BaseSlavePort(name, owner, id), _masterPort(NULL)
{
}

SlavePort::~SlavePort()
{
}

void
SlavePort::unbind()
{
    _baseMasterPort = NULL;
    _masterPort = NULL;
}

void
SlavePort::bind(MasterPort& master_port)
{
    _baseMasterPort = &master_port;
    _masterPort = &master_port;

//    std::cout << "bind: _slavePort id=" << this->getId() << "_slavePort name=" << this->name() << "\n";
//    std::cout << "bind: _baseMasterPort id=" << _baseMasterPort->getId() << "_baseMasterPort name=" << _baseMasterPort->name() << "\n";
}

Tick
SlavePort::sendAtomicSnoop(PacketPtr pkt)
{
    assert(pkt->isRequest());
    return _masterPort->recvAtomicSnoop(pkt);
}

void
SlavePort::sendFunctionalSnoop(PacketPtr pkt)
{
    assert(pkt->isRequest());
    return _masterPort->recvFunctionalSnoop(pkt);
}

bool
SlavePort::sendTimingResp(PacketPtr pkt)
{
    assert(pkt->isResponse());

    //    -----CHANGED----
//           if(pkt->getAddr() == 960){
//           	std::cout << curTick() << "\t" << pkt->getAddr() << "\tSlavePort::sendTimingResp()\t" << name() << "\n";
//           	std::cout << "MasterPort- " << _masterPort->name() << "\n";
//           	std::cout << "\t\t\tpkt headerDelay " << pkt->headerDelay << "\tpayLoadDelay " << pkt->payloadDelay << "\n";
//           }
//           if((!name().compare("system.membus.master[4]")) && pkt->getAddr() == 960){
//           	std::cout << "Going to access membus!!\n";
//           }
       //    -----CHANGED----

    //-----CHANGED----
//      std::cout << "delay_path \tStage1 \t" << curTick() << "\t" << pkt->req->rid << "\t" << pkt->getAddr() << "\t" << name() << "\tResponse" << "\n";
      addToDelayPath(pkt->req->rid, pkt->getAddr(), curTick(), name(), "Resp", false, 2);
      //-----CHANGED----
//      std::cout << curTick() << "\tTrace " << name() << " sendTimingResp(port.cc):\t" << pkt->getAddr() << "\t" << pkt->req->rid << "\t" << pkt->req->getseqNum() << "\n";

    return _masterPort->recvTimingResp(pkt);
}

void
SlavePort::sendTimingSnoopReq(PacketPtr pkt)
{
    assert(pkt->isRequest());

    //-----CHANGED----
//      std::cout << "delay_path \tStage1 \t" << curTick() << "\t" << pkt->req->rid << "\t" << pkt->getAddr() << "\t" << name() << "\tSnoopReq" << "\n";
      addToDelayPath(pkt->req->rid, pkt->getAddr(), curTick(), name(), "SnoopReq", false, 2);
      //-----CHANGED----

//      std::cout << curTick() << "\tTrace " << name() << " sendTimingSnoopReq(port.cc):\t" << pkt->getAddr() << "\t" << pkt->req->rid << "\t" << pkt->req->getseqNum() << "\n";

    _masterPort->recvTimingSnoopReq(pkt);
}

void
SlavePort::sendRetryReq()
{
	  //    -----CHANGED----
//	        if(pkt->getAddr() == 960){
//	        	std::cout << "Packet=" << pkt->getAddr() << "\tSlavePort::sendRetryReq()\t" << name() << "\tcurTick: "<< curTick() << "\n";
//	        }
	    //    -----CHANGED----

//    std::cout << curTick() << "\tTrace " << name() << " sendRetryReq(port.cc):\n";

    _masterPort->recvReqRetry();
}

void
SlavePort::sendRetrySnoopResp()
{
//    std::cout << curTick() << "\tTrace " << name() << " sendRetrySnoopResp(port.cc):\n";

    _masterPort->recvRetrySnoopResp();
}
