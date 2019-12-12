#include "NetworkObject.h"

using namespace NCL;
using namespace CSC8503;

bool NetworkObject::ReadPacket(GamePacket& p) {
    if (p.Type == Delta_State) {
        return ReadDeltaPacket((DeltaPacket&) p);
    }
    if (p.Type == Full_State) {
        return ReadFullPacket((FullPacket&) p);
    }
    return false; //this isn't a packet we care about!
}

bool NetworkObject::WritePacket(GamePacket** p, bool deltaFrame, int stateID) {
    if (deltaFrame) {
        if (!WriteDeltaPacket(p, stateID)) {
            return WriteFullPacket(p);
        }
    }
    return WriteFullPacket(p);
}

//Client objects recieve these packets
bool NetworkObject::ReadDeltaPacket(DeltaPacket& p) {
    if (p.FullID != m_LastFullState.m_StateID) {
        m_DeltaErrors++; //can't delta this frame
        return false;
    }

    UpdateStateHistory(p.FullID);

    Vector3 fullPos = m_LastFullState.m_Position;
    Quaternion fullOrientation = m_LastFullState.m_Orientation;

    fullPos.x += p.Pos[0];
    fullPos.y += p.Pos[1];
    fullPos.z += p.Pos[2];

    fullOrientation.x += ((float) p.Orientation[0]) / 127.0f;
    fullOrientation.y += ((float) p.Orientation[1]) / 127.0f;
    fullOrientation.z += ((float) p.Orientation[2]) / 127.0f;
    fullOrientation.w += ((float) p.Orientation[3]) / 127.0f;

    m_Object.GetTransform().SetWorldPosition(fullPos);
    m_Object.GetTransform().SetLocalOrientation(fullOrientation);

    return true;
}

bool NetworkObject::ReadFullPacket(FullPacket& p) {
    if (p.FullState.m_StateID < m_LastFullState.m_StateID) {
        return false; // received an 'old' packet, ignore!
    }
    m_LastFullState = p.FullState;

    m_Object.GetTransform().SetWorldPosition(m_LastFullState.m_Position);
    m_Object.GetTransform().SetLocalOrientation(m_LastFullState.m_Orientation);

    m_StateHistory.emplace_back(m_LastFullState);

    return true;
}

bool NetworkObject::WriteDeltaPacket(GamePacket** p, int stateID) {
    NetworkState state;
    if (!GetNetworkState(stateID, state)) {
        return false; //can't delta!
    }

    DeltaPacket* dp = new DeltaPacket();

    dp->ObjectID = m_NetworkID;    
    dp->FullID = stateID;

    Vector3 currentPos = m_Object.GetTransform().GetWorldPosition();
    Quaternion currentOrientation = m_Object.GetTransform().GetWorldOrientation();

    currentPos -= state.m_Position;
    currentOrientation -= state.m_Orientation;

    dp->Pos[0] = (char) currentPos.x;
    dp->Pos[1] = (char) currentPos.y;
    dp->Pos[2] = (char) currentPos.z;

    dp->Orientation[0] = (char) (currentOrientation.x * 127.0f);
    dp->Orientation[1] = (char) (currentOrientation.y * 127.0f);
    dp->Orientation[2] = (char) (currentOrientation.z * 127.0f);
    dp->Orientation[3] = (char) (currentOrientation.w * 127.0f);

    *p = dp;
    return true;
}

bool NetworkObject::WriteFullPacket(GamePacket** p) {
    auto* fp = new FullPacket();

    fp->ObjectID = m_NetworkID;
    fp->FullState.m_Position = m_Object.GetTransform().GetWorldPosition();
    fp->FullState.m_Orientation = m_Object.GetTransform().GetWorldOrientation();
    fp->FullState.m_StateID = m_LastFullState.m_StateID++;

    *p = fp;
    return true;
}

NetworkState& NetworkObject::GetLatestNetworkState() {
    return m_LastFullState;
}

bool NetworkObject::GetNetworkState(int stateID, NetworkState& state) {
    for (auto i = m_StateHistory.begin(); i < m_StateHistory.end(); ++i) {
        if ((*i).m_StateID == stateID) {
            state = (*i);
            return true;
        }
    }
    return false;
}

void NetworkObject::UpdateStateHistory(int minID) {
    for (auto i = m_StateHistory.begin(); i < m_StateHistory.end();) {
        if ((*i).m_StateID < minID) {
            i = m_StateHistory.erase(i);
        } else {
            ++i;
        }
    }
}
