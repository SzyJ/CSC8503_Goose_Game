#include "Debug.h"

using namespace NCL;

OGLRenderer* Debug::s_Renderer = nullptr;

std::vector<Debug::DebugStringEntry> Debug::s_StringEntries;

std::vector<Debug::DebugLineEntry> Debug::s_LineEntries;

void Debug::Print(const std::string& text, const Vector2& pos, const Vector4& colour) {
    DebugStringEntry newEntry;

    newEntry.Data = text;
    newEntry.Position = pos;
    newEntry.Colour = colour;

    s_StringEntries.emplace_back(newEntry);
}

void Debug::DrawLine(const Vector3& startpoint, const Vector3& endpoint, const Vector4& colour) {
    DebugLineEntry newEntry;

    newEntry.Start = startpoint;
    newEntry.End = endpoint;
    newEntry.Colour = colour;

    s_LineEntries.emplace_back(newEntry);
}

void Debug::FlushRenderables() {
    if (!s_Renderer) {
        return;
    }
    for (const auto& i : s_StringEntries) {
        s_Renderer->DrawString(i.Data, i.Position);
    }

    for (const auto& i : s_LineEntries) {
        s_Renderer->DrawLine(i.Start, i.End, i.Colour);
    }

    s_StringEntries.clear();
    s_LineEntries.clear();
}
