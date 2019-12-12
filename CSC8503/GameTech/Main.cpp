#include "../../Common/Window.h"

#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"

#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

#include "../CSC8503Common/NavigationGrid.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

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
void MultiHonk(Window* w);

int main() {
    Window* w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);

    if (!w->HasInitialised()) {
        return -1;
    }

    w->ShowOSPointer(false);
    w->LockMouseToWindow(true);

    //TestNetworking();

    MultiHonk(w);
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
    Window::DestroyGameWindow();
}

void MultiHonk(Window* w) {
    NetworkedGame* g;
    g = new NetworkedGame();

    g->StartAsServer();

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
    Window::DestroyGameWindow();
}

