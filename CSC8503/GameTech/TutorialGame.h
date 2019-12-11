#pragma once

#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "GameLoader.h"

namespace NCL {
    namespace CSC8503 {

        class TutorialGame {
        public:
            TutorialGame();
            virtual ~TutorialGame();

            virtual void UpdateGame(float dt);

        protected:
            void InitialiseAssets();

            void InitCamera();
            void UpdateKeys();

            void InitWorld();

            /*
            These are some of the world/object creation functions I created when testing the functionality
            in the module. Feel free to mess around with them to see different objects being created in different
            test scenarios (constraints, collision types, and so on). 
            */
            void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
            void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
            void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
            void BridgeConstraintTest();
            void SimpleGJKTest();

            bool SelectObject();
            void MoveSelectedObject();
            void MoveGoose();
            void UpdateCamPosition();

            void UpdateAppleForces();
            void UpdateKeeperForces();
            void UpdateGooseOrientation();

            void UpdateObjectGravity();

            GameObject* AddFloorToWorld(const Vector3& position);
            GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
            GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
            //IT'S HAPPENING
            GameObject* AddGooseToWorld(const Vector3& position);
            GameObject* AddParkKeeperToWorld(const Vector3& position);
            GameObject* AddCharacterToWorld(const Vector3& position);
            GameObject* AddAppleToWorld(const Vector3& position);

            GameObject* AddWaterTile(const Vector3& position, const Vector3& size);
            void AddWorldTiles();

            GameState* m_GameState;

            std::vector<GameObject*> m_AppleChain;
            GameObject* m_Goose;
            GameObject* m_Keeper;

            GameObject* m_Water;

            GameTechRenderer* m_Renderer;
            PhysicsSystem* m_Physics;
            GameWorld* m_World;
            bool m_UseGravity;
            bool m_InSelectionMode;
            float m_ForceMagnitude;

            OGLMesh* m_CubeMesh = nullptr;
            OGLMesh* m_SphereMesh = nullptr;
            OGLTexture* m_BasicTex = nullptr;
            OGLShader* m_BasicShader = nullptr;

            //Coursework Meshes
            OGLMesh* m_GooseMesh = nullptr;
            OGLMesh* m_KeeperMesh = nullptr;
            OGLMesh* m_AppleMesh = nullptr;
            OGLMesh* m_CharA = nullptr;
            OGLMesh* m_CharB = nullptr;

            //Coursework Additional functionality	
            Vector3 m_LockedOffset = Vector3(0, 14, 20);
        };

    }
}
