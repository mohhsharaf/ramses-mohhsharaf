//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCOMMANDBUFFER_H
#define RAMSES_RENDERERCOMMANDBUFFER_H

#include "RendererLib/RendererCommands.h"
#include <mutex>
#include <condition_variable>

namespace ramses_internal
{
    class RendererCommandBuffer
    {
    public:
        template <typename T>
        void enqueueCommand(T cmd);
        void addAndConsumeCommandsFrom(RendererCommands& cmds);
        void swapCommands(RendererCommands& cmds);

        std::tuple<std::mutex&, std::condition_variable&> getCVarForNewCommands();
        void swapCommands_unsafe(RendererCommands& cmds);

    private:
        std::mutex m_lock;
        RendererCommands m_commands;

        std::condition_variable m_newCommandsCvar;
    };

    template <typename T>
    void RendererCommandBuffer::enqueueCommand(T cmd)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_commands.push_back(std::move(cmd));
        m_newCommandsCvar.notify_all();
    }
}

#endif
