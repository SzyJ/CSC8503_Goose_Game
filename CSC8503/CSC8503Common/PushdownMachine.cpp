#include "PushdownMachine.h"
#include "PushdownState.h"
using namespace NCL::CSC8503;

void PushdownMachine::Update() {
    if (m_ActiveState) {
        PushdownState* newState = nullptr;
        PushdownState::PushdownResult result = m_ActiveState->PushdownUpdate(&newState);

        switch (result) {
        case PushdownState::Pop: {
            m_ActiveState->OnSleep();
            m_StateStack.pop();
            if (m_StateStack.empty()) {
                m_ActiveState = nullptr; //??????
            } else {
                m_ActiveState = m_StateStack.top();
                m_ActiveState->OnAwake();
            }
        }
        break;
        case PushdownState::Push: {
            m_ActiveState->OnSleep();
            m_StateStack.push(newState);
            newState->OnAwake();
        }
        break;
        }
    }
}
