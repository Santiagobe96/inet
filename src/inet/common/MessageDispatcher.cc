//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/MessageDispatcher.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

namespace inet {

Define_Module(MessageDispatcher);

// TODO: optimize gate access throughout this class
// TODO: factoring out some methods could also help

void MessageDispatcher::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        WATCH_MAP(socketIdToGateIndex);
        WATCH_MAP(interfaceIdToGateIndex);
        WATCH_MAP(serviceToGateIndex);
        WATCH_MAP(protocolToGateIndex);
    }
}

void MessageDispatcher::arrived(cMessage *message, cGate *inGate, const SendOptions& options, simtime_t time)
{
    Enter_Method_Silent();
    cGate *outGate = nullptr;
    if (message->isPacket()) {
        auto packet = check_and_cast<Packet *>(message);
        outGate = handlePacket(packet, inGate);
        handlePacketProcessed(packet);
    }
    else
        outGate = handleMessage(check_and_cast<Message *>(message), inGate);
    outGate->deliver(message, options, time);
    updateDisplayString();
}

bool MessageDispatcher::canPushSomePacket(cGate *inGate) const
{
    int size = gateSize("out");
    for (int i = 0; i < size; i++) {
        auto outGate = const_cast<MessageDispatcher *>(this)->gate("out", i);
        auto consumer = findConnectedModule<queueing::IPassivePacketSink>(outGate);
        if (consumer != nullptr && !dynamic_cast<MessageDispatcher *>(consumer) && !consumer->canPushSomePacket(outGate->getPathEndGate()))
            return false;
    }
    return true;
}

bool MessageDispatcher::canPushPacket(Packet *packet, cGate *inGate) const
{
    auto outGate = const_cast<MessageDispatcher *>(this)->handlePacket(packet, inGate);
    auto consumer = findConnectedModule<queueing::IPassivePacketSink>(outGate);
    return consumer != nullptr && !dynamic_cast<MessageDispatcher *>(consumer) && consumer->canPushPacket(packet, outGate->getPathEndGate());
}

void MessageDispatcher::pushPacket(Packet *packet, cGate *inGate)
{
    Enter_Method("pushPacket");
    take(packet);
    auto outGate = handlePacket(packet, inGate);
    auto consumer = findConnectedModule<IPassivePacketSink>(outGate);
    handlePacketProcessed(packet);
    pushOrSendPacket(packet, outGate, consumer);
    updateDisplayString();
}

void MessageDispatcher::pushPacketStart(Packet *packet, cGate *inGate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    auto outGate = handlePacket(packet, inGate);
    auto consumer = findConnectedModule<IPassivePacketSink>(outGate);
    pushOrSendPacketStart(packet, outGate, consumer, datarate);
    updateDisplayString();
}

void MessageDispatcher::pushPacketEnd(Packet *packet, cGate *inGate, bps datarate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    auto outGate = handlePacket(packet, inGate);
    auto consumer = findConnectedModule<IPassivePacketSink>(outGate);
    handlePacketProcessed(packet);
    pushOrSendPacketEnd(packet, outGate, consumer, datarate);
    updateDisplayString();
}

void MessageDispatcher::handleCanPushPacketChanged(cGate *outGate)
{
    int size = gateSize("in");
    for (int i = 0; i < size; i++) {
        auto inGate = gate("in", i);
        auto producer = findConnectedModule<queueing::IActivePacketSource>(inGate);
        if (producer != nullptr && !dynamic_cast<MessageDispatcher *>(producer) && outGate->getOwnerModule() != inGate->getOwnerModule())
            producer->handleCanPushPacketChanged(inGate->getPathStartGate());
    }
}

cGate *MessageDispatcher::handlePacket(Packet *packet, cGate *inGate)
{
    const auto& socketInd = packet->findTag<SocketInd>();
    if (socketInd != nullptr) {
        int socketId = socketInd->getSocketId();
        auto it = socketIdToGateIndex.find(socketId);
        if (it != socketIdToGateIndex.end())
            return gate("out", it->second);
        else
            throw cRuntimeError("handlePacket(): Unknown socket, id = %d, sender = %s", socketId, inGate->getPathStartGate()->getOwnerModule()->getFullName());
    }
    const auto& dispatchProtocolReq = packet->findTag<DispatchProtocolReq>();;
    if (dispatchProtocolReq != nullptr) {
        const auto& packetProtocolTag = packet->findTag<PacketProtocolTag>();;
        auto servicePrimitive = dispatchProtocolReq->getServicePrimitive();
        // TODO: KLUDGE: eliminate this by adding ServicePrimitive to every DispatchProtocolReq
        if (servicePrimitive == static_cast<ServicePrimitive>(-1)) {
            if (packetProtocolTag != nullptr && dispatchProtocolReq->getProtocol() == packetProtocolTag->getProtocol())
                servicePrimitive = SP_INDICATION;
            else
                servicePrimitive = SP_REQUEST;
        }
        auto protocol = dispatchProtocolReq->getProtocol();
        if (servicePrimitive == SP_REQUEST) {
            auto it = serviceToGateIndex.find(Key(protocol->getId(), servicePrimitive));
            if (it != serviceToGateIndex.end())
                return gate("out", it->second);
            else
                throw cRuntimeError("handlePacket(): Unknown protocol: id = %d, name = %s, sender = %s", protocol->getId(), protocol->getName(), inGate->getPathStartGate()->getOwnerModule()->getFullName());
        }
        else if (servicePrimitive == SP_INDICATION) {
            auto it = protocolToGateIndex.find(Key(protocol->getId(), servicePrimitive));
            if (it != protocolToGateIndex.end())
                return gate("out", it->second);
            else
                throw cRuntimeError("handlePacket(): Unknown protocol: id = %d, name = %s, sender = %s", protocol->getId(), protocol->getName(), inGate->getPathStartGate()->getOwnerModule()->getFullName());
        }
        else
            throw cRuntimeError("handlePacket(): Unknown service primitive");
    }
    const auto& interfaceReq = packet->findTag<InterfaceReq>();
    if (interfaceReq != nullptr) {
        int interfaceId = interfaceReq->getInterfaceId();
        auto it = interfaceIdToGateIndex.find(interfaceId);
        if (it != interfaceIdToGateIndex.end())
            return gate("out", it->second);
        else
            throw cRuntimeError("handlePacket(): Unknown interface: id = %d, sender = %s", interfaceId, inGate->getPathStartGate()->getOwnerModule()->getFullName());
    }
    throw cRuntimeError("handlePacket(): Unknown packet: %s(%s), sender = %s", packet->getName(), packet->getClassName(), inGate->getPathStartGate()->getOwnerModule()->getFullName());
}

cGate *MessageDispatcher::handleMessage(Message *message, cGate *inGate)
{
    const auto& socketReq = message->findTag<SocketReq>();
    if (socketReq != nullptr) {
        int socketReqId = socketReq->getSocketId();
        auto it = socketIdToGateIndex.find(socketReqId);
        if (it == socketIdToGateIndex.end())
            socketIdToGateIndex[socketReqId] = inGate->getIndex();
        else if (it->first != socketReqId)
            throw cRuntimeError("handleMessage(): Socket is already registered: id = %d, gate = %d, new gate = %d (from %s)", socketReqId, it->second, inGate->getIndex(), inGate->getPathStartGate()->getOwnerModule()->getFullName());
    }
    const auto& socketInd = message->findTag<SocketInd>();
    if (socketInd != nullptr) {
        int socketId = socketInd->getSocketId();
        auto it = socketIdToGateIndex.find(socketId);
        if (it != socketIdToGateIndex.end())
            return gate("out", it->second);
        else
            throw cRuntimeError("handleMessage(): Unknown socket, id = %d", socketId);
    }
    const auto& dispatchProtocolReq = message->findTag<DispatchProtocolReq>();;
    if (dispatchProtocolReq != nullptr) {
        auto servicePrimitive = dispatchProtocolReq->getServicePrimitive();
        // TODO: KLUDGE: eliminate this by adding ServicePrimitive to every DispatchProtocolReq
        if (servicePrimitive == static_cast<ServicePrimitive>(-1))
            servicePrimitive = SP_REQUEST;
        auto protocol = dispatchProtocolReq->getProtocol();
        if (servicePrimitive == SP_REQUEST) {
            auto it = serviceToGateIndex.find(Key(protocol->getId(), servicePrimitive));
            if (it != serviceToGateIndex.end())
                return gate("out", it->second);
            else
                throw cRuntimeError("handleMessage(): Unknown protocol: id = %d, name = %s", protocol->getId(), protocol->getName());
        }
        else if (servicePrimitive == SP_INDICATION) {
            auto it = protocolToGateIndex.find(Key(protocol->getId(), servicePrimitive));
            if (it != protocolToGateIndex.end())
                return gate("out", it->second);
            else
                throw cRuntimeError("handlePacket(): Unknown protocol: id = %d, name = %s, sender = %s", protocol->getId(), protocol->getName(), inGate->getPathStartGate()->getOwnerModule()->getFullName());
        }
        else
            throw cRuntimeError("handlePacket(): Unknown service primitive");
    }
    const auto& interfaceReq = message->findTag<InterfaceReq>();
    if (interfaceReq != nullptr) {
        int interfaceId = interfaceReq->getInterfaceId();
        auto it = interfaceIdToGateIndex.find(interfaceId);
        if (it != interfaceIdToGateIndex.end())
            return gate("out", it->second);
        else
            throw cRuntimeError("handleMessage(): Unknown interface: id = %d", interfaceId);
    }
    throw cRuntimeError("handleMessage(): Unknown message: %s(%s)", message->getName(), message->getClassName());
}

void MessageDispatcher::handleRegisterService(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
    auto key = Key(protocol.getId(), servicePrimitive);
    auto it = serviceToGateIndex.find(key);
    if (it != serviceToGateIndex.end()) {
        if (it->second != g->getIndex())
            throw cRuntimeError("handleRegisterService(): service is already registered: %s", protocol.str().c_str());
    }
    else {
        serviceToGateIndex[key] = g->getIndex();
        auto gateName = g->getType() == cGate::INPUT ? "out" : "in";
        int size = gateSize(gateName);
        for (int i = 0; i < size; i++)
            if (i != g->getIndex())
                registerService(protocol, gate(gateName, i), servicePrimitive);
    }
}

void MessageDispatcher::handleRegisterProtocol(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    auto key = Key(protocol.getId(), servicePrimitive);
    auto it = protocolToGateIndex.find(key);
    if (it != protocolToGateIndex.end()) {
        if (it->second != g->getIndex())
            throw cRuntimeError("handleRegisterProtocol(): protocol is already registered: %s", protocol.str().c_str());
    }
    else {
        protocolToGateIndex[key] = g->getIndex();
        auto gateName = g->getType() == cGate::INPUT ? "out" : "in";
        int size = gateSize(gateName);
        for (int i = 0; i < size; i++)
            if (i != g->getIndex())
                registerProtocol(protocol, gate(gateName, i), servicePrimitive);
    }
}

void MessageDispatcher::handleRegisterInterface(const InterfaceEntry &interface, cGate *out, cGate *in)
{
    Enter_Method("handleRegisterInterface");
    auto it = interfaceIdToGateIndex.find(interface.getInterfaceId());
    if (it != interfaceIdToGateIndex.end()) {
        if (it->second != out->getIndex())
            throw cRuntimeError("handleRegisterInterface(): Interface is already registered: %s", interface.str().c_str());
    }
    else {
        interfaceIdToGateIndex[interface.getInterfaceId()] = out->getIndex();
        int size = gateSize("out");
        for (int i = 0; i < size; i++)
            if (i != in->getIndex())
                registerInterface(interface, gate("in", i), gate("out", i));
    }
}

} // namespace inet

