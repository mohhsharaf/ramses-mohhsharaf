//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"
#include "ramses-utils.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "ramses-renderer-api/RendererSceneControl.h"

#include <thread>
#include <vector>
#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <iostream>
#include "CLI/CLI.hpp"


constexpr const char* const vertexShader = R"##(
#version 300 es

in vec2 a_position;
void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
}
)##";

constexpr const char* const fragmentShader = R"##(
#version 300 es

uniform sampler2D textureSampler;
out highp vec4 fragColor;
void main(void)
{
    highp vec2 ts = vec2(textureSize(textureSampler, 0));
    if(gl_FragCoord.x < ts.x && gl_FragCoord.y < ts.y)
    {
        fragColor = texelFetch(textureSampler,
#ifdef FLIP_Y
                                ivec2(gl_FragCoord.xy),
#else
                                ivec2(gl_FragCoord.x, ts.y-gl_FragCoord.y),
#endif
                                0);
    }
    else
    {
        fragColor = vec4(0.5, 0.3, 0.1, 0.2);
    }
}
)##";


class StreamSourceViewer
{
public:
    StreamSourceViewer(ramses::RamsesClient& ramsesClient, ramses::RamsesRenderer& ramsesRenderer, ramses::displayId_t display, ramses::sceneId_t sceneId, bool flipY, ramses::EScenePublicationMode publicationMode, uint32_t displayWidth, uint32_t displayHeight)
        : m_ramsesClient(ramsesClient)
        , m_ramsesRenderer(ramsesRenderer)
        , m_displayId(display)
    {
        ramses::SceneConfig conf;
        m_scene = m_ramsesClient.createScene(sceneId, conf);
        auto camera = m_scene->createPerspectiveCamera("my camera");
        camera->setViewport(0, 0, displayWidth, displayHeight);
        camera->setFrustum(19.f, static_cast<float>(displayWidth) / static_cast<float>(displayHeight), 0.1f, 1500.f);
        camera->setTranslation(0.0f, 0.0f, 5.0f);
        m_renderPass = m_scene->createRenderPass("my render pass");
        m_renderPass->setClearFlags(ramses::EClearFlags_None);
        m_renderPass->setCamera(*camera);
        m_renderGroup = m_scene->createRenderGroup();
        m_renderPass->addRenderGroup(*m_renderGroup);

        // prepare triangle geometry: vertex position array and index array
        float vertexPositionsArray[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};
        m_vertexPositions = m_scene->createArrayResource(ramses::EDataType::Vector2F, 4, vertexPositionsArray);

        uint16_t indicesArray[] = {0, 1, 2, 2, 1, 3};
        m_indices = m_scene->createArrayResource(ramses::EDataType::UInt16, 6, indicesArray);


        static const uint8_t textureData[] = {1u, 1u, 1u, 1u};
        const ramses::MipLevelData mipLevelData(sizeof(textureData), textureData);
        m_texture = m_scene->createTexture2D(ramses::ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "");

        ramses::EffectDescription effectDesc;

        effectDesc.setVertexShader(vertexShader);
        effectDesc.setFragmentShader(fragmentShader);
        if (flipY)
            effectDesc.addCompilerDefine("FLIP_Y");
        m_effect = m_scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
        m_scene->flush();
        m_scene->publish(publicationMode);
    }

    void handleStreamAvailable(ramses::waylandIviSurfaceId_t streamSource)
    {
        createStreamEntry(streamSource);
    }

    void handleStreamUnavailable(ramses::waylandIviSurfaceId_t streamSource)
    {
        destroyStreamEntry(streamSource);
    }

    void handleSceneFlushed(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag)
    {
        if(sceneId != m_scene->getSceneId())
            return;

        auto it = findStreamEntry(sceneVersionTag);
        assert(it != m_streamEntries.end());
        createAndLinkStreamBuffer(*it);
    }

private:
    struct StreamEntry
    {
        ramses::waylandIviSurfaceId_t      streamSource{0u};
        ramses::MeshNode*           meshNode = nullptr;
        ramses::Appearance*         appearance = nullptr;
        ramses::GeometryBinding*    geometryBinding = nullptr;
        ramses::TextureSampler*     textureSampler = nullptr;
        ramses::dataConsumerId_t    textureConsumerId;
        ramses::sceneVersionTag_t   flushVersionTag;
        ramses::streamBufferId_t    streamBuffer;
    };
    using StreamEntries=std::vector<StreamEntry>;

    StreamEntries::iterator findStreamEntry(ramses::waylandIviSurfaceId_t streamSource)
    {
        return std::find_if(m_streamEntries.begin(), m_streamEntries.end(), [streamSource](const auto& e){ return e.streamSource == streamSource;});
    }

    StreamEntries::iterator findStreamEntry(ramses::sceneVersionTag_t flushTag)
    {
        return std::find_if(m_streamEntries.begin(), m_streamEntries.end(), [flushTag](const auto& e){ return e.flushVersionTag == flushTag;});
    }

    void createStreamEntry(ramses::waylandIviSurfaceId_t streamSource)
    {
        assert(m_streamEntries.cend() == findStreamEntry(streamSource));

        StreamEntry streamEntry;
        streamEntry.streamSource = streamSource;
        createMesh(streamEntry);

        m_streamEntries.push_back(std::move(streamEntry));
    }

    void destroyStreamEntry(ramses::waylandIviSurfaceId_t streamSource)
    {
        auto it = findStreamEntry(streamSource);
        assert(it != m_streamEntries.end());

        unlinkAndDestroyStreamBuffer(*it);
        destroyMesh(*it);

        m_streamEntries.erase(it);
    }

    void createMesh(StreamEntry& streamEntry)
    {
        streamEntry.textureSampler = m_scene->createTextureSampler(ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Repeat, ramses::ETextureSamplingMethod_Nearest, ramses::ETextureSamplingMethod_Nearest, *m_texture);

        streamEntry.textureConsumerId = ramses::dataConsumerId_t{m_nextTextureConsumerId++};
        m_scene->createTextureConsumer(*streamEntry.textureSampler, streamEntry.textureConsumerId);

        streamEntry.appearance = m_scene->createAppearance(*m_effect);

        streamEntry.geometryBinding = m_scene->createGeometryBinding(*m_effect);
        streamEntry.geometryBinding->setIndices(*m_indices);
        ramses::AttributeInput positionsInput;
        m_effect->findAttributeInput("a_position", positionsInput);
        streamEntry.geometryBinding->setInputBuffer(positionsInput, *m_vertexPositions);

        ramses::UniformInput textureInput;
        m_effect->findUniformInput("textureSampler", textureInput);
        streamEntry.appearance->setInputTexture(textureInput, *streamEntry.textureSampler);

        // create a mesh node to define the triangle with chosen appearance
        streamEntry.meshNode = m_scene->createMeshNode();
        streamEntry.meshNode->setAppearance(*streamEntry.appearance);
        streamEntry.meshNode->setGeometryBinding(*streamEntry.geometryBinding);

        m_renderGroup->addMeshNode(*streamEntry.meshNode);

        streamEntry.flushVersionTag = ramses::sceneVersionTag_t{m_nextSceneFlushTag++};
        m_scene->flush(streamEntry.flushVersionTag);
    }

    void destroyMesh(StreamEntry& streamEntry)
    {
        m_renderGroup->removeMeshNode(*streamEntry.meshNode);
        m_scene->destroy(*streamEntry.meshNode);
        m_scene->destroy(*streamEntry.appearance);
        m_scene->destroy(*streamEntry.geometryBinding);
        m_scene->destroy(*streamEntry.textureSampler);

        m_scene->flush();
    }

    void createAndLinkStreamBuffer(StreamEntry& streamEntry)
    {
        streamEntry.streamBuffer = m_ramsesRenderer.createStreamBuffer(m_displayId, streamEntry.streamSource);
        m_ramsesRenderer.getSceneControlAPI()->linkStreamBuffer(streamEntry.streamBuffer, m_scene->getSceneId(), streamEntry.textureConsumerId);
        m_ramsesRenderer.flush();
        m_ramsesRenderer.getSceneControlAPI()->flush();
    }

    void unlinkAndDestroyStreamBuffer(StreamEntry& streamEntry)
    {
        m_ramsesRenderer.getSceneControlAPI()->unlinkData(m_scene->getSceneId(), streamEntry.textureConsumerId);
        m_ramsesRenderer.getSceneControlAPI()->flush();
        m_ramsesRenderer.destroyStreamBuffer(m_displayId, streamEntry.streamBuffer);
        m_ramsesRenderer.flush();
    }

    ramses::RamsesClient& m_ramsesClient;
    ramses::RamsesRenderer& m_ramsesRenderer;
    ramses::displayId_t m_displayId;

    ramses::Scene* m_scene                          = nullptr;
    ramses::RenderPass* m_renderPass                = nullptr;
    ramses::RenderGroup* m_renderGroup              = nullptr;
    const ramses::ArrayResource* m_vertexPositions  = nullptr;
    const ramses::ArrayResource* m_indices          = nullptr;
    const ramses::Texture2D* m_texture              = nullptr;
    ramses::Effect* m_effect                        = nullptr;
    uint32_t m_nextTextureConsumerId                = 0u;
    uint32_t m_nextSceneFlushTag                    = 0u;

    StreamEntries m_streamEntries;
};

class RendererEventHandler : public ramses::RendererEventHandlerEmpty
{
public:
    void windowClosed(ramses::displayId_t /*displayId*/) override
    {
        m_windowClosed = true;
    }

    [[nodiscard]] bool isWindowClosed() const
    {
        return m_windowClosed;
    }

private:
    bool m_windowClosed = false;
};

class RendererSceneControlEventHandler : public ramses::RendererSceneControlEventHandlerEmpty
{
public:
    explicit RendererSceneControlEventHandler(StreamSourceViewer& sceneCreator)
        : m_sceneCreator(sceneCreator)
    {
    }

    virtual void streamAvailabilityChanged(ramses::waylandIviSurfaceId_t streamId, bool available) override
    {
        if(available)
        {
            std::cout << std::endl << std::endl << "Stream " << streamId.getValue() << " available !" << std::endl;
            m_sceneCreator.handleStreamAvailable(streamId);
        }
        else
        {
            std::cout << std::endl << std::endl << "Stream " << streamId.getValue() << " unavailable !" << std::endl;
            m_sceneCreator.handleStreamUnavailable(streamId);
        }
    }

    virtual void sceneFlushed(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag) override
    {
        m_sceneCreator.handleSceneFlushed(sceneId, sceneVersionTag);
    }

private:
    StreamSourceViewer& m_sceneCreator;
};

int main(int argc, char* argv[])
{
    // default configuration
    float maxFps = 60.0f;
    bool  flipY  = false;
    ramses::RamsesFrameworkConfig config;
    config.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
    ramses::RendererConfig rendererConfig;
    ramses::DisplayConfig  displayConfig;
    displayConfig.setClearColor(0.5f, 0.f, 0.f, 1.f);

    CLI::App cli;
    try
    {
        cli.add_option("--fps", maxFps, "Frames per second")->default_val(maxFps);
        cli.add_flag("-y,--flip-y", flipY, "flip received stream vertically (on y-axis)");
        config.registerOptions(cli);
        rendererConfig.registerOptions(cli);
        displayConfig.registerOptions(cli);
    }
    catch (const CLI::Error& error)
    {
        // configuration error
        std::cerr << error.what();
        return -1;
    }

    CLI11_PARSE(cli, argc, argv);

    ramses::RamsesFramework framework(config);
    ramses::RamsesClient* ramsesClient(framework.createClient("stream viewer"));

    ramses::RamsesRenderer* renderer(framework.createRenderer(rendererConfig));
    auto sceneControlAPI = renderer->getSceneControlAPI();

    if (!ramsesClient || !renderer)
    {
        std::cerr << std::endl << "Failed to create either ramses client or ramses renderer." << std::endl;
        return 1;
    }

    renderer->startThread();

    const ramses::displayId_t display = renderer->createDisplay(displayConfig);
    renderer->setFramerateLimit(display, maxFps);
    renderer->flush();

    const ramses::sceneId_t sceneId{1u};
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
    displayConfig.getWindowRectangle(x, y, width, height);
    StreamSourceViewer sceneCreator(*ramsesClient, *renderer, display, sceneId, flipY, ramses::EScenePublicationMode_LocalOnly, width, height);

    sceneControlAPI->setSceneMapping(sceneId, display);
    sceneControlAPI->setSceneState(sceneId, ramses::RendererSceneState::Rendered);
    sceneControlAPI->flush();

    auto eventHandler = std::make_unique<RendererSceneControlEventHandler>(sceneCreator);

    RendererEventHandler rendererEventHandler;
    while (!rendererEventHandler.isWindowClosed())
    {
        renderer->dispatchEvents(rendererEventHandler);
        sceneControlAPI->dispatchEvents(*eventHandler);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
