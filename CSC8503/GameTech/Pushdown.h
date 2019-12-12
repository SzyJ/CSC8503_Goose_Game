#pragma once

#include <stack>

enum class GameState {
    Main_Menu,
    Solo_Honk,
    Host_Honk,
    Join_Honk
};

class Pushdown {
public:
    Pushdown() = default;
    ~Pushdown() = default;

    void AddState(GameState newState) {
        m_MenuStack.push(newState);
        m_CurrentState = newState;
    }

    GameState GetCurrentState() { return m_CurrentState; }

    bool PopState() {
        m_MenuStack.pop();
        m_CurrentState = m_MenuStack.top();
    }

private:
    std::stack<GameState> m_MenuStack;
    GameState m_CurrentState;
};