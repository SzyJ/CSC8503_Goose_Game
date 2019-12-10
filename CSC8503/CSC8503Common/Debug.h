#pragma once

#include "../../Plugins/OpenGLRendering/OGLRenderer.h"
#include <vector>
#include <string>

namespace NCL {

    class Debug {
    public:
        static void Print(const std::string& text, const Vector2& pos, const Vector4& colour = Vector4(1, 1, 1, 1));
        static void DrawLine(const Vector3& startpoint, const Vector3& endpoint, const Vector4& colour = Vector4(1, 1, 1, 1));

        static void DrawCube(const Vector3& position, const float size, const Vector4& colour = Vector4(1, 1, 1, 1));

        static void SetRenderer(OGLRenderer* r) {
            s_Renderer = r;
        }

        static void FlushRenderables();

    protected:
        struct DebugStringEntry {
            std::string Data;
            Vector2 Position;
            Vector4 Colour;
        };

        struct DebugLineEntry {
            Vector3 Start;
            Vector3 End;
            Vector4 Colour;
        };

        Debug() = default;
        ~Debug() = default;

        static std::vector<DebugStringEntry> s_StringEntries;
        static std::vector<DebugLineEntry> s_LineEntries;
        static OGLRenderer* s_Renderer;
    };

}
