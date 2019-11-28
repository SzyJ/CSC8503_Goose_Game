#pragma once

namespace NCL {
    namespace CSC8503 {

        class State {
        public:
            State() = default;
            virtual ~State() = default;

            virtual void Update() = 0; //Pure virtual base class
        };

        typedef void (*StateFunc)(void*);

        class GenericState : public State {
        public:
            GenericState(StateFunc someFunc, void* someData)
                : m_Func(someFunc), m_FuncData(someData) { }

            virtual void Update() {
                if (m_FuncData != nullptr) {
                    m_Func(m_FuncData);
                }
            }

        protected:
            StateFunc m_Func;
            void* m_FuncData;
        };

    }
}
