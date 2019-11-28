#pragma once
#include "NavigationMap.h"
#include <string>

namespace NCL {
    namespace CSC8503 {
        class NavigationMesh : public NavigationMap {
        public:
            NavigationMesh() = default;
            NavigationMesh(const std::string& filename);
            virtual ~NavigationMesh() = default;

            bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) override;

        protected:

        };

    }
}
