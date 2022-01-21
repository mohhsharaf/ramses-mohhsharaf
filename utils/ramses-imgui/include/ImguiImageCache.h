//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ImguiWidgets.h"
#include <unordered_map>

namespace ramses_internal
{
    class ImguiImageCache
    {
    public:
        explicit ImguiImageCache(ramses::Scene* scene);

        imgui::Image get(const TextureResource* res);

    private:
        ramses::Scene* m_scene;
        std::unordered_map<const TextureResource*, imgui::Image> m_images;
    };
}

