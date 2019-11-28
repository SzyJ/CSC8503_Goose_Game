#include "GameTechRenderer.h"
#include "../CSC8503Common/GameObject.h"
#include "../../Common/Camera.h"
#include "../../Common/Vector2.h"
#include "../../Common/Vector3.h"

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

#define SHADOWSIZE 4096

Matrix4 biasMatrix = Matrix4::Translation(Vector3(0.5, 0.5, 0.5)) * Matrix4::Scale(Vector3(0.5, 0.5, 0.5));

GameTechRenderer::GameTechRenderer(GameWorld& world) : OGLRenderer(*Window::GetWindow()), m_GameWorld(world) {
    glEnable(GL_DEPTH_TEST);

    m_ShadowShader = new OGLShader("GameTechShadowVert.glsl", "GameTechShadowFrag.glsl");

    glGenTextures(1, &m_ShadowTex);
    glBindTexture(GL_TEXTURE_2D, m_ShadowTex);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &m_ShadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D, m_ShadowTex, 0);
    glDrawBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(1, 1, 1, 1);

    //Set up the light properties
    m_LightColour = Vector4(0.8f, 0.8f, 0.5f, 1.0f);
    m_LightRadius = 1000.0f;
    m_LightPosition = Vector3(-200.0f, 60.0f, -200.0f);
}

GameTechRenderer::~GameTechRenderer() {
    glDeleteTextures(1, &m_ShadowTex);
    glDeleteFramebuffers(1, &m_ShadowFBO);
}

void GameTechRenderer::RenderFrame() {
    glEnable(GL_CULL_FACE);
    glClearColor(1, 1, 1, 1);
    BuildObjectList();
    SortObjectList();
    RenderShadowMap();
    RenderCamera();
    glDisable(GL_CULL_FACE); //Todo - text indices are going the wrong way...
}

void GameTechRenderer::BuildObjectList() {
    std::vector<GameObject*>::const_iterator first;
    std::vector<GameObject*>::const_iterator last;

    m_GameWorld.GetObjectIterators(first, last);

    m_ActiveObjects.clear();

    for (std::vector<GameObject*>::const_iterator i = first; i != last; ++i) {
        if ((*i)->IsActive()) {
            const RenderObject* g = (*i)->GetRenderObject();
            if (g) {
                m_ActiveObjects.emplace_back(g);
            }
        }
    }
}

void GameTechRenderer::SortObjectList() {}

void GameTechRenderer::RenderShadowMap() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);

    glCullFace(GL_FRONT);

    BindShader(m_ShadowShader);
    int mvpLocation = glGetUniformLocation(m_ShadowShader->GetProgramID(), "mvpMatrix");

    Matrix4 shadowViewMatrix = Matrix4::BuildViewMatrix(m_LightPosition, Vector3(0, 0, 0), Vector3(0, 1, 0));
    Matrix4 shadowProjMatrix = Matrix4::Perspective(100.0f, 500.0f, 1, 45.0f);

    Matrix4 mvMatrix = shadowProjMatrix * shadowViewMatrix;

    m_ShadowMatrix = biasMatrix * mvMatrix; //we'll use this one later on

    for (const auto& i : m_ActiveObjects) {
        Matrix4 modelMatrix = (*i).GetTransform()->GetWorldMatrix();
        Matrix4 mvpMatrix = mvMatrix * modelMatrix;
        glUniformMatrix4fv(mvpLocation, 1, false, (float*) &mvpMatrix);
        BindMesh((*i).GetMesh());
        DrawBoundMesh();
    }

    glViewport(0, 0, currentWidth, currentHeight);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glCullFace(GL_BACK);
}

void GameTechRenderer::RenderCamera() {
    float screenAspect = (float) currentWidth / (float) currentHeight;
    Matrix4 viewMatrix = m_GameWorld.GetMainCamera()->BuildViewMatrix();
    Matrix4 projMatrix = m_GameWorld.GetMainCamera()->BuildProjectionMatrix(screenAspect);

    OGLShader* activeShader = nullptr;
    int projLocation = 0;
    int viewLocation = 0;
    int modelLocation = 0;
    int colourLocation = 0;
    int hasVColLocation = 0;
    int hasTexLocation = 0;
    int shadowLocation = 0;

    int lightPosLocation = 0;
    int lightColourLocation = 0;
    int lightRadiusLocation = 0;

    int cameraLocation = 0;

    //TODO - PUT IN FUNCTION
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_ShadowTex);

    for (const auto& i : m_ActiveObjects) {
        OGLShader* shader = (OGLShader*) (*i).GetShader();
        BindShader(shader);

        BindTextureToShader((OGLTexture*) (*i).GetDefaultTexture(), "mainTex", 0);

        if (activeShader != shader) {
            projLocation = glGetUniformLocation(shader->GetProgramID(), "projMatrix");
            viewLocation = glGetUniformLocation(shader->GetProgramID(), "viewMatrix");
            modelLocation = glGetUniformLocation(shader->GetProgramID(), "modelMatrix");
            shadowLocation = glGetUniformLocation(shader->GetProgramID(), "shadowMatrix");
            colourLocation = glGetUniformLocation(shader->GetProgramID(), "objectColour");
            hasVColLocation = glGetUniformLocation(shader->GetProgramID(), "hasVertexColours");
            hasTexLocation = glGetUniformLocation(shader->GetProgramID(), "hasTexture");

            lightPosLocation = glGetUniformLocation(shader->GetProgramID(), "lightPos");
            lightColourLocation = glGetUniformLocation(shader->GetProgramID(), "lightColour");
            lightRadiusLocation = glGetUniformLocation(shader->GetProgramID(), "lightRadius");

            cameraLocation = glGetUniformLocation(shader->GetProgramID(), "cameraPos");
            glUniform3fv(cameraLocation, 1, (float*) &m_GameWorld.GetMainCamera()->GetPosition());

            glUniformMatrix4fv(projLocation, 1, false, (float*) &projMatrix);
            glUniformMatrix4fv(viewLocation, 1, false, (float*) &viewMatrix);

            glUniform3fv(lightPosLocation, 1, (float*) &m_LightPosition);
            glUniform4fv(lightColourLocation, 1, (float*) &m_LightColour);
            glUniform1f(lightRadiusLocation, m_LightRadius);

            int shadowTexLocation = glGetUniformLocation(shader->GetProgramID(), "shadowTex");
            glUniform1i(shadowTexLocation, 1);

            activeShader = shader;
        }

        Matrix4 modelMatrix = (*i).GetTransform()->GetWorldMatrix();
        glUniformMatrix4fv(modelLocation, 1, false, (float*) &modelMatrix);

        Matrix4 fullShadowMat = m_ShadowMatrix * modelMatrix;
        glUniformMatrix4fv(shadowLocation, 1, false, (float*) &fullShadowMat);

        glUniform4fv(colourLocation, 1, (float*) &i->GetColour());

        glUniform1i(hasVColLocation, !(*i).GetMesh()->GetColourData().empty());

        glUniform1i(hasTexLocation, (OGLTexture*) (*i).GetDefaultTexture() ? 1 : 0);

        BindMesh((*i).GetMesh());
        DrawBoundMesh();
    }
}

void GameTechRenderer::SetupDebugMatrix(OGLShader* s) {
    float screenAspect = (float) currentWidth / (float) currentHeight;
    Matrix4 viewMatrix = m_GameWorld.GetMainCamera()->BuildViewMatrix();
    Matrix4 projMatrix = m_GameWorld.GetMainCamera()->BuildProjectionMatrix(screenAspect);

    Matrix4 vp = projMatrix * viewMatrix;

    int matLocation = glGetUniformLocation(s->GetProgramID(), "viewProjMatrix");

    glUniformMatrix4fv(matLocation, 1, false, (float*) &vp);
}
