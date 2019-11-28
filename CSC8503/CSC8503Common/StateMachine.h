#pragma once

#include <vector>
#include <map>

namespace NCL {
    namespace CSC8503 {

        class State;

        class StateTransition;

        typedef std::multimap<State*, StateTransition*> TransitionContainer;

        typedef TransitionContainer::iterator TransitionIterator;

        class StateMachine {
        public:
            StateMachine() = default;
            ~StateMachine() = default;

            void AddState(State* s);
            void AddTransition(StateTransition* t);

            void Update();

        protected:
            State* m_ActiveState = nullptr;
            std::vector<State*> m_AllStates;
            TransitionContainer m_AllTransitions;
        };

    }
}
