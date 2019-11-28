#pragma once

#include "../../Common/Matrix4.h"
#include "../../Common/TextureBase.h"
#include "../../Common/ShaderBase.h"
#include "../../Common/Vector4.h"

namespace NCL {

    using namespace NCL::Rendering;
    class MeshGeometry;

    namespace CSC8503 {

        class Transform;
        using namespace Maths;

        class RenderObject {
        public:

           RenderObject(Transform* parentTransform, MeshGeometry* mesh, TextureBase* tex, ShaderBase* shader)
                : m_Transform(parentTransform), m_Mesh(mesh), m_Texture(tex), m_Shader(shader), m_Colour(1.0f, 1.0f, 1.0f, 1.0f) { }

           ~RenderObject() = default;

            void SetDefaultTexture(TextureBase* t) {
                m_Texture = t;
            }

            TextureBase* GetDefaultTexture() const {
                return m_Texture;
            }

            MeshGeometry* GetMesh() const {
                return m_Mesh;
            }

            Transform* GetTransform() const {
                return m_Transform;
            }

            ShaderBase* GetShader() const {
                return m_Shader;
            }

            void SetColour(const Vector4& c) {
                m_Colour = c;
            }

            Vector4 GetColour() const {
                return m_Colour;
            }

        protected:
            MeshGeometry* m_Mesh;
            TextureBase* m_Texture;
            ShaderBase* m_Shader;
            Transform* m_Transform;
            Vector4 m_Colour;
        };

    }
}
