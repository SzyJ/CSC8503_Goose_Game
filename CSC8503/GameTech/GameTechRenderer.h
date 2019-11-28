#pragma once

#include "../../Plugins/OpenGLRendering/OGLRenderer.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"

#include "../CSC8503Common/GameWorld.h"

namespace NCL {

    class Maths::Vector3;
    class Maths::Vector4;

    namespace CSC8503 {

        class RenderObject;

        class GameTechRenderer : public OGLRenderer {
        public:
            GameTechRenderer(GameWorld& world);
            ~GameTechRenderer();

        protected:
            vector<const RenderObject*> m_ActiveObjects;

            //shadow mapping things
            OGLShader* m_ShadowShader = nullptr;
            GLuint m_ShadowTex;
            GLuint m_ShadowFBO;
            Matrix4 m_ShadowMatrix;
            Vector4 m_LightColour;
            float m_LightRadius;
            Vector3 m_LightPosition;

            OGLShader* m_DefaultShader;
            GameWorld& m_GameWorld;

            void RenderFrame() override;

            void BuildObjectList();
            void SortObjectList();
            void RenderShadowMap();
            void RenderCamera();

            void SetupDebugMatrix(OGLShader* s) override;
        };

    }
}
