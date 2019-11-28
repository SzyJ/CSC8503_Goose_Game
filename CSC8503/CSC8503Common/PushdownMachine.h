#pragma once

#include <stack>

namespace NCL {
    namespace CSC8503 {

        class PushdownState;

        class PushdownMachine {
        public:
            PushdownMachine() = default;
            ~PushdownMachine() = default;

            void Update();

        protected:
            PushdownState* m_ActiveState = nullptr;
            std::stack<PushdownState*> m_StateStack;
        };

    }
}
