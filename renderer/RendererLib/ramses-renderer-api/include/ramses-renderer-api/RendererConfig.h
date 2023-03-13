//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERAPI_RENDERERCONFIG_H
#define RAMSES_RENDERERAPI_RENDERERCONFIG_H

#include "ramses-renderer-api/Types.h"
#include "ramses-framework-api/StatusObject.h"
#include <chrono>

namespace CLI
{
    class App;
}

namespace ramses
{
    class IBinaryShaderCache;
    class IRendererResourceCache;

    /**
    * @brief The RendererConfig holds a set of parameters to be used
    * to initialize a renderer.
    */
    class RAMSES_API RendererConfig : public StatusObject
    {
    public:
        /**
        * @brief Default constructor of RendererConfig
        */
        RendererConfig();

        /**
        * @brief Copy constructor of RendererConfig
        * @param[in] other Other instance of RendererConfig
        */
        RendererConfig(const RendererConfig& other);

        /**
        * @brief Destructor of RendererConfig
        */
        virtual ~RendererConfig();

        /**
        * @brief Register command line options for the CLI11 command line parser
        *
        * Creates an option group "Renderer Options" and registers command line options
        * After parsing the command line with CLI::App::parse() this config object is assigned with the values provided by command line
        *
        * @param[in] cli CLI11 command line parser
        */
        void registerOptions(CLI::App& cli);

        /**
        * @brief Set the Binary Shader Cache to be used in Renderer.
        * @param[in] cache the binary shader cache to be used by the Renderer
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setBinaryShaderCache(IBinaryShaderCache& cache);

        /**
        * @brief Set the resource cache implementation to be used by the renderer.
        * @param[in] cache the resource cache to be used by the renderer.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setRendererResourceCache(IRendererResourceCache& cache);

        /**
        * @brief Enable the renderer to communicate with the system compositor.
        *        This flag needs to be enabled before calling any of the system compositor
        *        related calls, otherwise an error will be reported when issuing such commands
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t enableSystemCompositorControl();

        /**
         * @brief      Set the maximum time to wait for the system compositor frame callback
         *             before aborting and skipping rendering of current frame. This is an
         *             advanced function to be used by experts only. Be warned that the
         *             synchronization of frame callbacks with the system compositor and
         *             the display controller vsync is a sensitive topic and can majorly
         *             influence system performance.
         *
         * @param[in]  waitTimeInUsec  The maximum time wait for a frame callback, in microseconds
         *
         * @return     StatusOK for success, otherwise the returned status can
         *             be used to resolve error message using
         *             getStatusMessage().
         */
        status_t setFrameCallbackMaxPollTime(uint64_t waitTimeInUsec);

        /**
        * @brief Set the Wayland display name to connect system compositor to.
        *        This will override the default behavior which is to use WAYLAND_DISPLAY environment variable
        *
        * @param[in] waylandDisplay Wayland display name to use for connection
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setSystemCompositorWaylandDisplay(const char* waylandDisplay);

        /**
        * @brief Get the current setting of Wayland display name
        *
        * @return Wayland display name to use for connection, empty means default
        */
        [[nodiscard]] const char* getSystemCompositorWaylandDisplay() const;

        /**
        * @brief   Set the desired reporting period for first display loop timings.
        * @details The values are reported periodically via the renderer callback
        *          ramses::IRendererEventHandler::renderThreadLoopTimings.
        *          Only the first display is measured.
        *          A value of zero disables reporting and is the default.
        *
        * @param[in] period Cyclic time period after which timing information should be reported
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds period);

        /**
        * @brief Get the current reporting period for renderThread loop timings
        *
        * @return Reporting period for renderThread loop timings
        */
        [[nodiscard]] std::chrono::milliseconds getRenderThreadLoopTimingReportingPeriod() const;

        /**
        * Stores internal data for implementation specifics of RendererConfig.
        */
        class RendererConfigImpl& impl;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        RendererConfig& operator=(const RendererConfig& other) = delete;
    };
}

#endif
