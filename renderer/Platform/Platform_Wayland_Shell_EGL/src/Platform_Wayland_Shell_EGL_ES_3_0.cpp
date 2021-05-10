//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Wayland_Shell_EGL/Platform_Wayland_Shell_EGL_ES_3_0.h"
#include "Window_Wayland_Shell/Window_Wayland_Shell.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/DisplayConfig.h"

namespace ramses_internal
{
    IPlatform* Platform_Base::CreatePlatform(const RendererConfig& rendererConfig)
    {
        return new Platform_Wayland_Shell_EGL_ES_3_0(rendererConfig);
    }

    Platform_Wayland_Shell_EGL_ES_3_0::Platform_Wayland_Shell_EGL_ES_3_0(const RendererConfig& rendererConfig)
        : Platform_Wayland_EGL(rendererConfig)
    {
    }

    bool Platform_Wayland_Shell_EGL_ES_3_0::createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        auto window = std::make_unique<Window_Wayland_Shell>(displayConfig, windowEventHandler, 0u);
        if (window->init())
        {
            m_windowEventsPollingManager.addWindow(window.get());
            m_window = std::move(window);
            return true;
        }

        return false;
    }

    void Platform_Wayland_Shell_EGL_ES_3_0::destroyWindow()
    {
        m_windowEventsPollingManager.removeWindow(static_cast<Window_Wayland*>(m_window.get()));
        Platform_Wayland_EGL::destroyWindow();
    }
}
