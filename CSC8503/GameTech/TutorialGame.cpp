#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"

#include "../CSC8503Common/PositionConstraint.h"

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() {
    m_World = new GameWorld();
    m_Renderer = new GameTechRenderer(*m_World);
    m_Physics = new PhysicsSystem(*m_World);

    m_ForceMagnitude = 10.0f;
    m_UseGravity = false;
    m_InSelectionMode = false;

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
    if (m_LockedObject != nullptr) {
        LockedCameraMovement();
    }

    UpdateKeys();

    if (m_UseGravity) {
        Debug::Print("(G)ravity on", Vector2(10, 40));
    } else {
        Debug::Print("(G)ravity off", Vector2(10, 40));
    }

    SelectObject();
    MoveSelectedObject();

    m_World->UpdateWorld(dt);
    m_Renderer->Update(dt);
    m_Physics->Update(dt);

    Debug::FlushRenderables();
    m_Renderer->Render();
}

void TutorialGame::UpdateKeys() {
    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
        InitWorld(); //We can reset the simulation at any time with F1
        m_SelectionObject = nullptr;
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

    if (m_LockedObject) {
        LockedObjectMovement();
    } else {
        DebugObjectMovement();
    }
}

void TutorialGame::LockedObjectMovement() {
    Matrix4 view = m_World->GetMainCamera()->BuildViewMatrix();
    Matrix4 camWorld = view.Inverse();

    Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

    //forward is more tricky -  camera forward is 'into' the screen...
    //so we can take a guess, and use the cross of straight up, and
    //the right axis, to hopefully get a vector that's good enough!

    Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);

    if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
        m_SelectionObject->GetPhysicsObject()->AddForce(-rightAxis);
    }

    if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
        m_SelectionObject->GetPhysicsObject()->AddForce(rightAxis);
    }

    if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
        m_SelectionObject->GetPhysicsObject()->AddForce(fwdAxis);
    }

    if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
        m_SelectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
    }
}

void TutorialGame::LockedCameraMovement() {
    if (m_LockedObject != nullptr) {
        Vector3 objPos = m_LockedObject->GetTransform().GetWorldPosition();
        Vector3 camPos = objPos + m_LockedOffset;

        Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

        Matrix4 modelMat = temp.Inverse();

        Quaternion q(modelMat);
        Vector3 angles = q.ToEuler(); //nearly there now!

        m_World->GetMainCamera()->SetPosition(camPos);
        m_World->GetMainCamera()->SetPitch(angles.x);
        m_World->GetMainCamera()->SetYaw(angles.y);
    }
}


void TutorialGame::DebugObjectMovement() {
    //If we've selected an object, we can manipulate it with some key presses
    if (m_InSelectionMode && m_SelectionObject) {
        //Twist the selected object!
        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
            m_SelectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
        }

        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
            m_SelectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
        }

        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
            m_SelectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
        }

        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
            m_SelectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
        }

        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
            m_SelectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
        }

        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
            m_SelectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
        }

        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
            m_SelectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
        }

        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
            m_SelectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
        }
    }
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

        if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
            if (m_SelectionObject) {
                //set colour to deselected;
                m_SelectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
                m_SelectionObject = nullptr;
            }

            Ray ray = CollisionDetection::BuildRayFromMouse(*m_World->GetMainCamera());

            RayCollision closestCollision;
            if (m_World->Raycast(ray, closestCollision, true)) {
                m_SelectionObject = (GameObject*) closestCollision.Node;
                m_SelectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
                return true;
            } else {
                return false;
            }
        }
        if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
            if (m_SelectionObject) {
                if (m_LockedObject == m_SelectionObject) {
                    m_LockedObject = nullptr;
                } else {
                    m_LockedObject = m_SelectionObject;
                }
            }
        }
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

void TutorialGame::MoveSelectedObject() {}

void TutorialGame::InitCamera() {
    m_World->GetMainCamera()->SetNearPlane(0.5f);
    m_World->GetMainCamera()->SetFarPlane(500.0f);
    m_World->GetMainCamera()->SetPitch(-15.0f);
    m_World->GetMainCamera()->SetYaw(315.0f);
    m_World->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
    m_LockedObject = nullptr;
}

void TutorialGame::InitWorld() {
    m_World->ClearAndErase();
    m_Physics->Clear();

    InitMixedGridWorld(10, 10, 3.5f, 3.5f);
    AddGooseToWorld(Vector3(30, 2, 0));
    AddAppleToWorld(Vector3(35, 2, 0));

    AddParkKeeperToWorld(Vector3(40, 2, 0));
    AddCharacterToWorld(Vector3(45, 2, 0));

    AddFloorToWorld(Vector3(0, -2, 0));
}

//From here on it's functions to add in objects to the world!

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
    GameObject* floor = new GameObject();

    Vector3 floorSize = Vector3(100, 2, 100);
    AABBVolume* volume = new AABBVolume(floorSize);
    floor->SetBoundingVolume((CollisionVolume*) volume);
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
    GameObject* sphere = new GameObject();

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
    GameObject* cube = new GameObject();

    AABBVolume* volume = new AABBVolume(dimensions);

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

    GameObject* goose = new GameObject();


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

    GameObject* keeper = new GameObject();

    AABBVolume* volume = new AABBVolume(Vector3(0.3, 0.9f, 0.3) * meshSize);
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

    GameObject* character = new GameObject();

    float r = rand() / (float) RAND_MAX;


    AABBVolume* volume = new AABBVolume(Vector3(0.3, 0.9f, 0.3) * meshSize);
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
    GameObject* apple = new GameObject();

    SphereVolume* volume = new SphereVolume(0.7f);
    apple->SetBoundingVolume((CollisionVolume*) volume);
    apple->GetTransform().SetWorldScale(Vector3(4, 4, 4));
    apple->GetTransform().SetWorldPosition(position);

    apple->SetRenderObject(new RenderObject(&apple->GetTransform(), m_AppleMesh, nullptr, m_BasicShader));
    apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

    apple->GetPhysicsObject()->SetInverseMass(1.0f);
    apple->GetPhysicsObject()->InitSphereInertia();

    m_World->AddGameObject(apple);

    return apple;
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
    for (int x = 0; x < numCols; ++x) {
        for (int z = 0; z < numRows; ++z) {
            Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
            AddSphereToWorld(position, radius, 1.0f);
        }
    }
    AddFloorToWorld(Vector3(0, -2, 0));
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
    AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
    for (int x = 1; x < numCols + 1; ++x) {
        for (int z = 1; z < numRows + 1; ++z) {
            Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
            AddCubeToWorld(position, cubeDims, 1.0f);
        }
    }
    AddFloorToWorld(Vector3(0, -2, 0));
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
