#include "../../Common/Window.h"

#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"

#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

#include "../CSC8503Common/NavigationGrid.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"
#include "Pushdown.h"

using namespace NCL;
using namespace CSC8503;

class TestPacketReceiver : public PacketReceiver {
public:
    TestPacketReceiver(string name) {
        this->m_Name = name;
    }

    void ReceivePacket(int type, GamePacket * payload, int source) {
        if (type == String_Message) {
            StringPacket * realPacket = (StringPacket*)payload;
            string msg = realPacket->GetStringFromData();
            std::cout << m_Name << " received message : " << msg << std::endl;
        }
    }

protected:
    string m_Name;
};

void TestStateMachine() {
    StateMachine* testMachine = new StateMachine();
    
    int someData = 0;
    
    StateFunc AFunc = [](void* data) {
        int* realData = (int*)data;
        (*realData)++;
        std::cout << "In State A!" << std::endl;
    };
    StateFunc BFunc = [](void* data) {
        int* realData = (int*)data;
        (*realData)--;
        std::cout << "In State B!" << std::endl;
    };
    
    GenericState* stateA = new GenericState(AFunc, (void*)&someData);
    GenericState* stateB = new GenericState(BFunc, (void*)&someData);

    testMachine->AddState(stateA);
    testMachine->AddState(stateB);

    auto* transitionA = new GenericTransition<int&, int>(
        GenericTransition<int&, int>::GreaterThanTransition, someData, 10, stateA, stateB); // if greater than 10 , A to B

    auto* transitionB = new GenericTransition<int&, int>(
        GenericTransition<int&, int>::EqualsTransition, someData, 0, stateB, stateA); // if equals 0 , B to A

    testMachine->AddTransition(transitionA);
    testMachine->AddTransition(transitionB);

    for (int i = 0; i < 100; ++i) {
        testMachine->Update(); // run the state machine !
    }
    delete testMachine;
}

void TestNetworking() {
    NetworkBase::Initialise();

    TestPacketReceiver serverReceiver(" Server ");
    TestPacketReceiver clientReceiver(" Client ");

    int port = NetworkBase::GetDefaultPort();

    GameServer* server = new GameServer(port, 1);
    GameClient* client = new GameClient();

    server->RegisterPacketHandler(String_Message, &serverReceiver);
    client->RegisterPacketHandler(String_Message, &clientReceiver);
    bool canConnect = client->Connect(127, 0, 0, 1, port);

    for (int i = 0; i < 100; ++i) {
        server->SendGlobalPacket(
            StringPacket(" Server says hello ! " + std::to_string(i)));

        client->SendPacket(
            StringPacket(" Client says hello ! " + std::to_string(i)));

        server->UpdateServer();
        client->UpdateClient();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    NetworkBase::Destroy();
}

vector<Vector3> testNodes;
NavigationGrid* testGrid;

void TestPathfinding() {
    testGrid = new NavigationGrid("main_path.gmap");
    NavigationPath outPath;

    Vector3 startPos(80, 0, 10);
    Vector3 endPos(80, 0, 80);

    bool found = testGrid->FindPath(startPos, endPos, outPath);

    std::cout << "Path " << (found ? "" : "not ") << "Found!" << std::endl;

    Vector3 pos;

    while (outPath.PopWaypoint(pos)) {
        testNodes.push_back(pos);
    }
}

void DisplayPathfinding() {
    for (int i = 1; i < testNodes.size(); ++i) {
        Vector3 a = testNodes[i - 1];
        Vector3 b = testNodes[i];

        a.y = 10.0f;
        b.y = 10.0f;

        Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
    }

    //testGrid->DebugDrawGrid();
}


/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/
void SoloHonk(Window* w);
void MultiHonk(Window* w, bool host);

void MainMenu(Window* w, Pushdown& gameState);

int main() {
    Window* w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);

    if (!w->HasInitialised()) {
        return -1;
    }

    w->ShowOSPointer(false);
    w->LockMouseToWindow(true);

    Pushdown gameState;
    gameState.AddState(GameMenuState::Main_Menu);

    while (gameState.HasStates()) {
        if (gameState.GetCurrentState() == GameMenuState::Main_Menu) {
            MainMenu(w, gameState);
        } else if (gameState.GetCurrentState() == GameMenuState::Solo_Honk) {
            SoloHonk(w);
            gameState.PopState();
        } else if (gameState.GetCurrentState() == GameMenuState::Host_Honk) {
            MultiHonk(w, true);
            gameState.PopState();
        } else if (gameState.GetCurrentState() == GameMenuState::Join_Honk) {
            MultiHonk(w, false);
            gameState.PopState();
        }
    }

    Window::DestroyGameWindow();
}

void MainMenu(Window* w, Pushdown& gameState) {
    int SelectionIndex = 0;
    bool choiceMade = false;

    auto* world = new GameWorld();
    auto* renderer = new GameTechRenderer(*world);

    std::string choiceArray[4];

    choiceArray[0] = "Solo Honk";
    choiceArray[1] = "Host Honk";
    choiceArray[2] = "Join Honk";
    choiceArray[3] = "Quit";

    while (!choiceMade) {
        w->UpdateWindow();

        for (int i = 0; i < 4; ++i) {

            Vector4 col;
            if (SelectionIndex == i) {
                col = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
            } else {
                col = Vector4(0.3f, 0.3f, 0.3f, 1.0f);
            }

            renderer->DrawString(choiceArray[i], Vector2(200.0f, 200.0f - (i * 20.0f)),  col);

        }

        renderer->Render();

        if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::UP) ||
            Window::GetKeyboard()->KeyPressed(KeyboardKeys::W)) {
            --SelectionIndex;
            if (SelectionIndex < 0) {
                SelectionIndex = 0;
            }
        } else if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::DOWN) ||
            Window::GetKeyboard()->KeyPressed(KeyboardKeys::S)) {
            ++SelectionIndex;
            if (SelectionIndex > 3) {
                SelectionIndex = 3;
            }
        } else if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN) ||
            Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE)) {
            choiceMade = true;
        }

    }

    delete renderer;
    delete world;
    

    switch(SelectionIndex) {
    case 0:
        gameState.AddState(GameMenuState::Solo_Honk);
        break;
    case 1:
        gameState.AddState(GameMenuState::Host_Honk);
        break;
    case 2:
        gameState.AddState(GameMenuState::Join_Honk);
        break;
    case 3:
        gameState.PopState();
        break;
    }

}

void SoloHonk(Window* w) {
    TutorialGame* g;
    g = new TutorialGame();

    while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::END)) {
        float dt = w->GetTimer()->GetTimeDeltaSeconds();

        if (dt > 1.0f) {
            std::cout << "Skipping large time delta" << std::endl;
            continue; //must have hit a breakpoint or something to have a 1 second frame time!
        }
        if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
            w->ShowConsole(true);
        }
        if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
            w->ShowConsole(false);
        }

        w->SetTitle("Solo-Honk Goose Game | FPS: " + std::to_string(1.0f / dt));

        g->UpdateGame(dt);
    }
}

void MultiHonk(Window* w, bool host) {
    NetworkedGame* g;
    g = new NetworkedGame();

    if (host) {
        g->StartAsServer();
    } else {
        g->StartAsClient(127,0,0,1);
    }

    while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::END)) {
        float dt = w->GetTimer()->GetTimeDeltaSeconds();

        if (dt > 1.0f) {
            std::cout << "Skipping large time delta" << std::endl;
            continue; //must have hit a breakpoint or something to have a 1 second frame time!
        }
        if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
            w->ShowConsole(true);
        }
        if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
            w->ShowConsole(false);
        }

        w->SetTitle("Multi-Honk Goose Game | FPS: " + std::to_string(1.0f / dt));

        g->UpdateGame(dt);
    }
}

