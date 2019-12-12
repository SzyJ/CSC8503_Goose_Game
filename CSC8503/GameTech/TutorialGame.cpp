#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"

#include "../CSC8503Common/PositionConstraint.h"
#include "../../Common/Maths.h"

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() {
    m_World = new GameWorld();
    m_Renderer = new GameTechRenderer(*m_World);
    m_Physics = new PhysicsSystem(*m_World);

    m_ForceMagnitude = 10.0f;
    m_UseGravity = false;
    m_InSelectionMode = false;

    m_GameState = new GameState("main");

    Debug::SetRenderer(m_Renderer);

    InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
    auto loadFunc = [](const string& name, OGLMesh** into) {
        *into = new OGLMesh(name);
        (*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
        (*into)->UploadToGPU();
    };

    loadFunc("cube.msh", &m_CubeMesh);
    loadFunc("sphere.msh", &m_SphereMesh);
    loadFunc("goose.msh", &m_GooseMesh);
    loadFunc("CharacterA.msh", &m_KeeperMesh);
    loadFunc("CharacterM.msh", &m_CharA);
    loadFunc("CharacterF.msh", &m_CharB);
    loadFunc("Apple.msh", &m_AppleMesh);

    m_BasicTex = (OGLTexture*) TextureLoader::LoadAPITexture("checkerboard.png");
    m_BasicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

    InitCamera();
    InitWorld();
}

TutorialGame::~TutorialGame() {
    delete m_CubeMesh;
    delete m_SphereMesh;
    delete m_GooseMesh;
    delete m_BasicTex;
    delete m_BasicShader;

    delete m_Physics;
    delete m_Renderer;
    delete m_World;
}

void TutorialGame::UpdateGame(float dt) {
    if (!m_InSelectionMode) {
        m_World->GetMainCamera()->UpdateCamera(dt);
    }

    UpdateKeys();

    if (m_UseGravity) {
        Debug::Print("(G)ravity on", Vector2(10, 40));
    } else {
        Debug::Print("(G)ravity off", Vector2(10, 40));
    }

    UpdateObjectGravity();

    SelectObject();
    if (m_InSelectionMode) {
        MoveSelectedObject();
    }

    UpdateAppleForces();

    UpdateGooseOrientation();

    UpdateKeeperForces();

    m_World->UpdateWorld(dt);
    m_Renderer->Update(dt);
    m_Physics->Update(dt);

    Debug::FlushRenderables();
    m_Renderer->Render();
}

void TutorialGame::UpdateAppleForces() {
    for (size_t index = 0; index < m_AppleChain.size(); ++index) {
        Vector3 targetPosition;

        GameObject* thisApple = m_AppleChain.at(index);
        if (index == 0) {
            Vector3 direction;
            direction = m_Goose->GetTransform().GetWorldPosition() - thisApple->GetTransform().GetWorldPosition();
            float objDistance = direction.Length();
            objDistance -= (0.7f + 0.7f + 0.4f);

            targetPosition = direction.Normalised() * objDistance;

        } else {
            Vector3 direction;
            direction = m_AppleChain.at(index - 1)->GetTransform().GetWorldPosition() - thisApple->GetTransform().GetWorldPosition();

            float objDistance = direction.Length();
            objDistance -= (0.7f + 0.7f + 0.4f);

            targetPosition = direction.Normalised() * objDistance;
        }

        const float forceStrength = (15.0f * 5.0f) - (5.0f * 1.0f);

        thisApple->GetPhysicsObject()->AddForce(targetPosition * forceStrength);
    }
}

void TutorialGame::UpdateObjectGravity() {
    std::vector<GameObject*>::const_iterator first;
    std::vector<GameObject*>::const_iterator last;
    m_World->GetObjectIterators(first, last);

    const float mapSize = static_cast<float>((m_GameState->GetMapWidth() - 0.5f) * m_GameState->GetNodeSize());

    for (auto obj = first; obj < last; ++obj) {
        auto trans = (*obj)->GetTransform();

        Vector3 objGravDir(0.0f, 0.0f, 0.0f);

        if (trans.GetWorldPosition().x < -0.5f * m_GameState->GetNodeSize()) {
            objGravDir.x = 1.0f;
        } else if (trans.GetWorldPosition().x > mapSize) {
            objGravDir.x = -1.0f;
        }

        if (trans.GetWorldPosition().z < -0.5f * m_GameState->GetNodeSize()) {
            objGravDir.z = 1.0f;
        } else if (trans.GetWorldPosition().z > mapSize) {
            objGravDir.z = -1.0f;
        }

        if (trans.GetWorldPosition().y >= 0.0f) {
            objGravDir.y = -1.0f;
        } else {
            objGravDir.y = 1.0f;
        }
        
        objGravDir.Normalise();

        (*obj)->GetPhysicsObject()->SetGravityDirection(objGravDir);
    }
}

void TutorialGame::UpdateKeeperForces() {
    if (m_AppleChain.empty()) {
        return;
    }

    // Perform pathfinding
    Vector3 keeperPos = m_Keeper->GetTransform().GetWorldPosition();

    const float tolerence = 1.0f;

    if (keeperPos.x + tolerence > m_NextWaypoint.x &&
        keeperPos.x - tolerence < m_NextWaypoint.x &&
        keeperPos.z + tolerence > m_NextWaypoint.z &&
        keeperPos.z - tolerence < m_NextWaypoint.z) {

        // Checkpoint reached: recalculate path
        NavigationPath path;
        bool found = m_GameState->GetNavigationGrid()->FindPath(keeperPos, m_Goose->GetConstTransform().GetWorldPosition(), path);

        if (!found) {
            return;
        }

        Vector3 lastWP;
        Vector3 thisWP;

        path.PopWaypoint(lastWP);
        path.PopWaypoint(lastWP);

        m_NextWaypoint = lastWP;

        if (m_InSelectionMode) {
            while(path.PopWaypoint(thisWP)) {
                thisWP.y = 11.0f;
                lastWP.y = 11.0f;
            
                Debug::DrawLine(lastWP, thisWP, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
            
                lastWP = thisWP;
            }            
        }

    }

    if (m_InSelectionMode) {
        m_GameState->GetNavigationGrid()->DebugDrawGrid();
    }

    Vector3 dir((m_NextWaypoint - keeperPos));
    dir.y = 0.0f;
    dir.Normalise();

    m_Keeper->GetPhysicsObject()->AddForce(dir * 100.0f);
    
    float targetAngle = atan2(dir.x, dir.z);

    Quaternion targetOrientation;
    targetOrientation.x = 0.0f;
    targetOrientation.y = sin(targetAngle * 0.5f);
    targetOrientation.z = 0.0f;
    targetOrientation.w = cos(targetAngle * 0.5f);
    m_Keeper->GetTransform().SetLocalOrientation(targetOrientation);
}

void TutorialGame::UpdateGooseOrientation() {
    const float snappiness = 3.0f;

    Vector3 flatVel = m_Goose->GetPhysicsObject()->GetLinearVelocity();
    flatVel.Normalise();

    if (flatVel.x > 0.0f - FLT_EPSILON &&
        flatVel.x < 0.0f + FLT_EPSILON &&
        flatVel.z > 0.0f - FLT_EPSILON &&
        flatVel.z < 0.0f + FLT_EPSILON) {

        return;
    }

    float targetAngle = atan2(flatVel.x, flatVel.z);

    Quaternion targetOrientation;
    targetOrientation.x = 0.0f;
    targetOrientation.y = sin(targetAngle * 0.5f);
    targetOrientation.z = 0.0f;
    targetOrientation.w = cos(targetAngle * 0.5f);

    Quaternion currentOrientation = m_Goose->GetTransform().GetLocalOrientation();
    currentOrientation.Normalise();
    float currentAngle = asin(currentOrientation.y) * 2.0f;
    m_Goose->GetPhysicsObject()->AddTorque(Vector3(0.0f, (targetAngle - currentAngle) * snappiness, 0.0f));
}

void TutorialGame::UpdateKeys() {
    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
        InitWorld(); //We can reset the simulation at any time with F1
    }

    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
        InitCamera(); //F2 will reset the camera to a specific default place
    }

    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
        m_UseGravity = !m_UseGravity; //Toggle gravity!
        m_Physics->UseGravity(m_UseGravity);
    }
    //Running certain physics updates in a consistent order might cause some
    //bias in the calculations - the same objects might keep 'winning' the constraint
    //allowing the other one to stretch too much etc. Shuffling the order so that it
    //is random every frame can help reduce such bias.
    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
        m_World->ShuffleConstraints(true);
    }
    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
        m_World->ShuffleConstraints(false);
    }

    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
        m_World->ShuffleObjects(true);
    }
    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
        m_World->ShuffleObjects(false);
    }

    MoveGoose();
    UpdateCamPosition();
}

void TutorialGame::MoveGoose() {
    Matrix4 view = m_World->GetMainCamera()->BuildViewMatrix();
    Matrix4 camWorld = view.Inverse();

    Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

    //forward is more tricky -  camera forward is 'into' the screen...
    //so we can take a guess, and use the cross of straight up, and
    //the right axis, to hopefully get a vector that's good enough!

    Vector3 fwdAxis = Vector3::Cross(-m_Goose->GetPhysicsObject()->GetGravityDirection(), rightAxis);

    const float moveForce = 100.0f;

    if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
        m_Goose->SetSleep(false);
        m_Goose->GetPhysicsObject()->AddForce(-rightAxis * moveForce);
    }

    if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
        m_Goose->SetSleep(false);
        m_Goose->GetPhysicsObject()->AddForce(rightAxis * moveForce);
    }

    if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
        m_Goose->SetSleep(false);
        m_Goose->GetPhysicsObject()->AddForce(fwdAxis * moveForce);
    }

    if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
        m_Goose->SetSleep(false);
        m_Goose->GetPhysicsObject()->AddForce(-fwdAxis * moveForce);
    }

    if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE)) {
        m_Goose->SetSleep(false);
        m_Goose->GetPhysicsObject()->AddForce(-m_Goose->GetPhysicsObject()->GetGravityDirection() * moveForce);
    }

}

void TutorialGame::UpdateCamPosition() {
    const float maxCamDist = 15.0f;


    Vector3 facingDir = m_World->GetMainCamera()->GetPointingDirection();
    Vector3 camPosOffset = m_Goose->GetTransform().GetWorldPosition();

    facingDir.z = -facingDir.z;
    facingDir.x = -facingDir.x;

    camPosOffset -= facingDir * maxCamDist;

    m_World->GetMainCamera()->SetPosition(camPosOffset);

    //Vector3 goosePos = m_Goose->GetTransform().GetWorldPosition();
    //const float mapSize = static_cast<float>((m_GameState->GetMapWidth() * 0.5f) * m_GameState->GetNodeSize());
    //if (goosePos.x < 0 || goosePos.z < 0) {
    //    m_World->GetMainCamera()->SetRoll(-90.0f);
    //} else if (goosePos.x > mapSize || goosePos.z < mapSize) {
    //    m_World->GetMainCamera()->SetRoll(90.0f);
    //} else if (goosePos.y > 0.0f) {
    //    m_World->GetMainCamera()->SetRoll(0.0f);
    //} else {
    //    m_World->GetMainCamera()->SetRoll(180.0f);
    //
    //}
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
        m_InSelectionMode = !m_InSelectionMode;
        if (m_InSelectionMode) {
            Window::GetWindow()->ShowOSPointer(true);
            Window::GetWindow()->LockMouseToWindow(false);
        } else {
            Window::GetWindow()->ShowOSPointer(false);
            Window::GetWindow()->LockMouseToWindow(true);
        }
    }

    if (m_InSelectionMode) {
        m_Renderer->DrawString("Press Esc to change to camera mode!", Vector2(10, 0));
    } else {
        m_Renderer->DrawString("Press Esc to change to select mode!", Vector2(10, 0));
    }
    return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
    m_ForceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

    if (m_InSelectionMode) {
        m_Renderer->DrawString(" Click Force :" + std::to_string(m_ForceMagnitude), Vector2(10, 20)); // Draw debug text at 10 ,20
    }

    if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
        Ray ray = CollisionDetection::BuildRayFromMouse(*m_World->GetMainCamera());

        RayCollision closestCollision;

        if (m_World->Raycast(ray, closestCollision, true)) {

            if (closestCollision.Node) {
                ((GameObject*) closestCollision.Node)->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * m_ForceMagnitude, closestCollision.CollidedAt);
            }
        }
    }
}

void TutorialGame::InitCamera() {
    m_World->GetMainCamera()->SetNearPlane(0.5f);
    m_World->GetMainCamera()->SetFarPlane(500.0f);
    m_World->GetMainCamera()->SetPitch(-15.0f);
    m_World->GetMainCamera()->SetYaw(315.0f);
    m_World->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
}

void TutorialGame::InitWorld() {
    m_World->ClearAndErase();
    m_Physics->Clear();

    InitMixedGridWorld(10, 10, 3.5f, 3.5f);

    const float spawnOffset = 15.0f;

    Vector3 goosePos = m_GameState->GetNavigationGrid()->GetGoosePosition();
    goosePos.y += m_GameState->HeightAt(goosePos.x, goosePos.z) * 1.0f;
    goosePos.y += spawnOffset;
    m_Goose = AddGooseToWorld(goosePos);

    for (auto position : m_GameState->GetNavigationGrid()->GetApplePositions()) {
        position.y += m_GameState->HeightAt(position.x, position.z) * 1.0f;
        position.y += spawnOffset;
        m_AppleChain.push_back(AddAppleToWorld(position));
    }

    Vector3 keeperPos = m_GameState->GetNavigationGrid()->GetKeeperPosition();
    keeperPos.y += m_GameState->HeightAt(keeperPos.x, keeperPos.z) * 1.0f;
    keeperPos.y += spawnOffset;
    m_Keeper = AddParkKeeperToWorld(keeperPos);
    m_NextWaypoint = keeperPos;
    //AddCharacterToWorld();

    //AddWaterTile(Vector3(10.f, 10.0f, -10.0f), Vector3(5.0f, 5.0f, 5.0f));

    AddWorldTiles();

    //AddFloorToWorld(Vector3(0, -2, 0));
}

//From here on it's functions to add in objects to the world!

void TutorialGame::AddWorldTiles() {
    unsigned int xTileCount = m_GameState->GetMapWidth();
    unsigned int yTileCount = m_GameState->GetMapHeight();
    unsigned int tileSize = m_GameState->GetNodeSize();

    const float baseHeight = 5.0f;
    const float heightScale = 0.5f;

    for (unsigned int y = 0; y < yTileCount; ++y) {

        for (unsigned int x = 0; x < xTileCount; ++x) {
            float xPos = static_cast<float>(x * tileSize);
            float yPos = static_cast<float>(y * tileSize);

            Vector3 position(xPos, 0.0f, yPos);
            if (m_GameState->GetMapTiles()[(y * xTileCount) + x] == MapTile::Ground) {
                auto cube = AddCubeToWorld(position, Vector3(static_cast<float>(tileSize) * 0.5f, (m_GameState->HeightAt(xPos, yPos) * heightScale) + baseHeight, static_cast<float>(tileSize) * 0.5f), 0.0f);
                cube->GetRenderObject()->SetColour(Vector4(0.3f, 0.37f, 0.15f, 1.0f));
            } else {
                AddWaterTile(position, Vector3(static_cast<float>(tileSize) * 0.5f, (m_GameState->HeightAt(xPos, yPos) * heightScale) + baseHeight, static_cast<float>(tileSize) * 0.5f));
            }
        }

    }
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
    GameObject* floor = new GameObject("Ground");

    Vector3 floorSize = Vector3(100, 2, 100);
    auto* volume = new AABBVolume(floorSize);
    floor->SetBoundingVolume((CollisionVolume*) volume);
    //floor->GetTransform().SetLocalOrientation(Quaternion(0.0f, 0.5f, 0.5f, 1.0f));
    floor->GetTransform().SetWorldScale(floorSize);
    floor->GetTransform().SetWorldPosition(position);

    floor->SetRenderObject(new RenderObject(&floor->GetTransform(), m_CubeMesh, m_BasicTex, m_BasicShader));
    floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

    floor->GetPhysicsObject()->SetInverseMass(0);
    floor->GetPhysicsObject()->InitCubeInertia();

    m_World->AddGameObject(floor);

    return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
    GameObject* sphere = new GameObject("Sphere");

    Vector3 sphereSize = Vector3(radius, radius, radius);
    SphereVolume* volume = new SphereVolume(radius);
    sphere->SetBoundingVolume((CollisionVolume*) volume);
    sphere->GetTransform().SetWorldScale(sphereSize);
    sphere->GetTransform().SetWorldPosition(position);

    sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), m_SphereMesh, m_BasicTex, m_BasicShader));
    sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

    sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
    sphere->GetPhysicsObject()->InitSphereInertia();

    m_World->AddGameObject(sphere);

    return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
    GameObject* cube = new GameObject("Cube");

    auto* volume = new AABBVolume(dimensions);

    cube->SetBoundingVolume((CollisionVolume*) volume);

    cube->GetTransform().SetWorldPosition(position);
    cube->GetTransform().SetWorldScale(dimensions);

    cube->SetRenderObject(new RenderObject(&cube->GetTransform(), m_CubeMesh, m_BasicTex, m_BasicShader));
    cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

    cube->GetPhysicsObject()->SetInverseMass(inverseMass);
    cube->GetPhysicsObject()->InitCubeInertia();

    m_World->AddGameObject(cube);

    return cube;
}

GameObject* TutorialGame::AddGooseToWorld(const Vector3& position) {
    float size = 1.0f;
    float inverseMass = 1.0f;

    GameObject* goose = new GameObject("Goose");


    SphereVolume* volume = new SphereVolume(size);
    goose->SetBoundingVolume((CollisionVolume*) volume);

    goose->GetTransform().SetWorldScale(Vector3(size, size, size));
    goose->GetTransform().SetWorldPosition(position);

    goose->SetRenderObject(new RenderObject(&goose->GetTransform(), m_GooseMesh, nullptr, m_BasicShader));
    goose->SetPhysicsObject(new PhysicsObject(&goose->GetTransform(), goose->GetBoundingVolume()));

    goose->GetPhysicsObject()->SetInverseMass(inverseMass);
    goose->GetPhysicsObject()->InitSphereInertia();

    m_World->AddGameObject(goose);

    return goose;
}

GameObject* TutorialGame::AddParkKeeperToWorld(const Vector3& position) {
    float meshSize = 4.0f;
    float inverseMass = 0.5f;

    GameObject* keeper = new GameObject("Keeper");

    //auto* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
    auto* volume = new SphereVolume(meshSize);
    keeper->SetBoundingVolume((CollisionVolume*) volume);

    keeper->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
    keeper->GetTransform().SetWorldPosition(position);

    keeper->SetRenderObject(new RenderObject(&keeper->GetTransform(), m_KeeperMesh, nullptr, m_BasicShader));
    keeper->SetPhysicsObject(new PhysicsObject(&keeper->GetTransform(), keeper->GetBoundingVolume()));

    keeper->GetPhysicsObject()->SetInverseMass(inverseMass);
    keeper->GetPhysicsObject()->InitCubeInertia();

    m_World->AddGameObject(keeper);

    return keeper;
}

GameObject* TutorialGame::AddCharacterToWorld(const Vector3& position) {
    float meshSize = 4.0f;
    float inverseMass = 0.5f;

    auto pos = m_KeeperMesh->GetPositionData();

    Vector3 minVal = pos[0];
    Vector3 maxVal = pos[0];

    for (auto& i : pos) {
        maxVal.y = max(maxVal.y, i.y);
        minVal.y = min(minVal.y, i.y);
    }

    GameObject* character = new GameObject("Character");

    float r = rand() / (float) RAND_MAX;


    AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
    character->SetBoundingVolume((CollisionVolume*) volume);

    character->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
    character->GetTransform().SetWorldPosition(position);

    character->SetRenderObject(new RenderObject(&character->GetTransform(), r > 0.5f ? m_CharA : m_CharB, nullptr, m_BasicShader));
    character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

    character->GetPhysicsObject()->SetInverseMass(inverseMass);
    character->GetPhysicsObject()->InitCubeInertia();

    m_World->AddGameObject(character);

    return character;
}

GameObject* TutorialGame::AddAppleToWorld(const Vector3& position) {
    GameObject* apple = new GameObject("Apple");

    SphereVolume* volume = new SphereVolume(0.7f);
    apple->SetBoundingVolume((CollisionVolume*) volume);
    apple->GetTransform().SetWorldScale(Vector3(2.0f,2.0f,2.0f));
    apple->GetTransform().SetWorldPosition(position);

    apple->SetRenderObject(new RenderObject(&apple->GetTransform(), m_AppleMesh, nullptr, m_BasicShader));
    apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

    apple->GetPhysicsObject()->SetInverseMass(1.0f);
    apple->GetPhysicsObject()->InitSphereInertia();

    apple->GetRenderObject()->SetColour(Vector4(0.4, 0.07, 0.03, 1.0f));
    m_World->AddGameObject(apple);

    return apple;
}

GameObject* TutorialGame::AddWaterTile(const Vector3& position, const Vector3& size) {
    GameObject* cube = new GameObject("Water");

    auto* volume = new AABBVolume(size);

    cube->SetBoundingVolume((CollisionVolume*)volume);

    cube->GetTransform().SetWorldPosition(position);
    cube->GetTransform().SetWorldScale(size);

    cube->SetRenderObject(new RenderObject(&cube->GetTransform(), m_CubeMesh, m_BasicTex, m_BasicShader));
    cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

    cube->GetPhysicsObject()->SetInverseMass(0.0f);
    cube->GetPhysicsObject()->InitCubeInertia();

    cube->GetRenderObject()->SetColour(Vector4(0.0f, 0.5f, 1.0f, 1.0f));

    m_World->AddGameObject(cube);

    return cube;
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
    for (int x = 0; x < numCols; ++x) {
        for (int z = 0; z < numRows; ++z) {
            Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
            AddSphereToWorld(position, radius, 1.0f);
        }
    }
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
    float sphereRadius = 1.0f;
    Vector3 cubeDims = Vector3(1, 1, 1);

    for (int x = 0; x < numCols; ++x) {
        for (int z = 0; z < numRows; ++z) {
            Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

            if (rand() % 2) {
                AddCubeToWorld(position, cubeDims);
            } else {
                AddSphereToWorld(position, sphereRadius);
            }
        }
    }
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
    for (int x = 1; x < numCols + 1; ++x) {
        for (int z = 1; z < numRows + 1; ++z) {
            Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
            AddCubeToWorld(position, cubeDims, 1.0f);
        }
    }
}

void TutorialGame::BridgeConstraintTest() {
    Vector3 cubeSize = Vector3(8, 8, 8);

    float invCubeMass = 5;
    int numLinks = 25;
    float maxDistance = 30;
    float cubeDistance = 20;

    Vector3 startPos = Vector3(500, 1000, 500);

    GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);

    GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

    GameObject* previous = start;

    for (int i = 0; i < numLinks; ++i) {
        GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
        PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
        m_World->AddConstraint(constraint);
        previous = block;
    }

    PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
    m_World->AddConstraint(constraint);
}

void TutorialGame::SimpleGJKTest() {
    Vector3 dimensions = Vector3(5, 5, 5);
    Vector3 floorDimensions = Vector3(100, 2, 100);

    GameObject* fallingCube = AddCubeToWorld(Vector3(0, 20, 0), dimensions, 10.0f);
    GameObject* newFloor = AddCubeToWorld(Vector3(0, 0, 0), floorDimensions, 0.0f);

    delete fallingCube->GetBoundingVolume();
    delete newFloor->GetBoundingVolume();

    fallingCube->SetBoundingVolume((CollisionVolume*) new OBBVolume(dimensions));
    newFloor->SetBoundingVolume((CollisionVolume*) new OBBVolume(floorDimensions));

}
