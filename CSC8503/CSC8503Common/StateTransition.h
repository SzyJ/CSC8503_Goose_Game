#pragma once

namespace NCL {
    namespace CSC8503 {

        class State;

        class StateTransition {
        public:
            virtual bool CanTransition() const = 0;

            State* GetDestinationState() const {
                return m_DestinationState;
            }

            State* GetSourceState() const {
                return m_SourceState;
            }

        protected:
            State* m_SourceState;
            State* m_DestinationState;
        };

        template <class T, class U>
        class GenericTransition : public StateTransition {
        public:
            typedef bool (*GenericTransitionFunc)(T, U);

            GenericTransition(GenericTransitionFunc f, T testData, U otherData, State* srcState, State* destState) :
                m_DataA(testData), m_DataB(otherData), m_Func(f) {
                m_SourceState = srcState;
                m_DestinationState = destState;
            }
            ~GenericTransition() = default;

            virtual bool CanTransition() const override {
                if (m_Func) {
                    return m_Func(m_DataA, m_DataB);
                }
                return false;
            }

            static bool GreaterThanTransition(T dataA, U dataB) {
                return dataA > dataB;
            }

            static bool LessThanTransition(T dataA, U dataB) {
                return dataA < dataB;
            }

            static bool EqualsTransition(T dataA, U dataB) {
                return dataA == dataB;
            }

            static bool NotEqualsTransition(T dataA, U dataB) {
                return dataA != dataB;
            }

        protected:
            GenericTransitionFunc m_Func;
            T m_DataA;
            U m_DataB;
        };

    }
}
