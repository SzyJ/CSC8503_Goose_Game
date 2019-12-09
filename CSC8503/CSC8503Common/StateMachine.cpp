#include "StateMachine.h"
#include "State.h"
#include "StateTransition.h"

using namespace NCL::CSC8503;

void StateMachine::AddState(State* s) {
    m_AllStates.emplace_back(s);
    if (m_ActiveState == nullptr) {
        m_ActiveState = s;
    }
}

void StateMachine::AddTransition(StateTransition* t) {
    m_AllTransitions.insert(std::make_pair(t->GetSourceState(), t));
}

void StateMachine::Update() {
    if (m_ActiveState) {
        m_ActiveState->Update();
        // Get the transition set starting from this state node ;
        std::pair < TransitionIterator, TransitionIterator > range =
        m_AllTransitions.equal_range(m_ActiveState);
        // Iterate through them all
        for (auto& i = range.first; i != range.second; ++i) {
            if (i->second->CanTransition()) { // some transition is true !
                State * newState = i->second->GetDestinationState();
                m_ActiveState = newState;
            }
        }
    }
}
