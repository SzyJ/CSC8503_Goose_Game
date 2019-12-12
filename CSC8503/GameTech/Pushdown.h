#pragma once

#include <stack>

enum class GameMenuState {
    Main_Menu,
    Solo_Honk,
    Host_Honk,
    Join_Honk
};

class Pushdown {
public:
    Pushdown() = default;
    ~Pushdown() = default;

    void AddState(GameMenuState newState) {
        m_MenuStack.push(newState);
        m_CurrentState = newState;
    }

    GameMenuState GetCurrentState() { return m_CurrentState; }

    bool HasStates() { return !m_MenuStack.empty(); }

    bool PopState() {
        m_MenuStack.pop();
        if (m_MenuStack.empty()) {
            return false;
        }

        m_CurrentState = m_MenuStack.top();
        return true;
    }

private:
    std::stack<GameMenuState> m_MenuStack;
    GameMenuState m_CurrentState;
};