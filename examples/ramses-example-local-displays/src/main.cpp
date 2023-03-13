//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
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
#include "ramses-renderer-api/RendererSceneControl.h"
#include <unordered_set>
#include <thread>

/**
 * @example ramses-example-local-displays/src/main.cpp
 * @brief Example of a local client plus renderer and two displays
 */

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

uint64_t nowMs()
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
}

ramses::Scene* createScene1(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    //client scene
    ramses::Scene* clientScene = client.createScene(sceneId, ramses::SceneConfig(), "local displays example scene");

    //fill scene with content, i.e. use high level api
    // every scene needs a render pass with camera
    auto* camera = clientScene->createPerspectiveCamera("my camera");
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    camera->setTranslation(0.0f, 0.0f, 5.0f);
    ramses::RenderPass* renderPass = clientScene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = clientScene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsArray[] = { -1.f, 0.f, -6.f, 1.f, 0.f, -6.f, 0.f, 1.f, -6.f };
    ramses::ArrayResource* vertexPositions = clientScene->createArrayResource(ramses::EDataType::Vector3F, 3, vertexPositionsArray);
    uint16_t indicesArray[] = { 0, 1, 2 };
    ramses::ArrayResource* indices = clientScene->createArrayResource(ramses::EDataType::UInt16, 3, indicesArray);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-local-displays-test.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-local-displays-test.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    ramses::Effect* effect = clientScene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearance = clientScene->createAppearance(*effect, "triangle appearance");
    ramses::GeometryBinding* geometry = clientScene->createGeometryBinding(*effect, "triangle geometry");

    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);

    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* meshNode = clientScene->createMeshNode("triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setGeometryBinding(*geometry);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);

    appearance->setInputValueVector4f(colorInput, 1.0f, 1.0f, 0.3f, 1.0f);

    return clientScene;
}

ramses::Scene* createScene2(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    //client scene
    ramses::Scene* clientScene = client.createScene(sceneId, ramses::SceneConfig(), "local displays example scene");

    //fill scene with content, i.e. use high level api
    // every scene needs a render pass with camera
    auto* camera = clientScene->createPerspectiveCamera("my camera");
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    camera->setTranslation(0.0f, 0.0f, 5.0f);
    ramses::RenderPass* renderPass = clientScene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = clientScene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsArray[] = { -1.1f, 0.f, -6.1f, 1.1f, 0.f, -6.1f, 0.f, 1.1f, -6.1f };
    ramses::ArrayResource* vertexPositions = clientScene->createArrayResource(ramses::EDataType::Vector3F, 3, vertexPositionsArray);
    uint16_t indicesArray[] = { 2, 0, 1 };
    ramses::ArrayResource* indices = clientScene->createArrayResource(ramses::EDataType::UInt16, 3, indicesArray);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-local-displays-test.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-local-displays-test.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    ramses::Effect* effect = clientScene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearance = clientScene->createAppearance(*effect, "triangle appearance");
    ramses::GeometryBinding* geometry = clientScene->createGeometryBinding(*effect, "triangle geometry");

    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);

    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* meshNode = clientScene->createMeshNode("triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setGeometryBinding(*geometry);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);

    appearance->setInputValueVector4f(colorInput, 1.0f, 0.0f, 0.5f, 1.0f);

    return clientScene;
}

int main()
{
    //Ramses client
    ramses::RamsesFrameworkConfig config;
    config.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);  //needed for automated test of examples
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& client(*framework.createClient("ramses-local-client-test"));

    // Ramses renderer
    ramses::RendererConfig rendererConfig;
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    auto& sceneControlAPI = *renderer.getSceneControlAPI();
    framework.connect();

    ramses::sceneId_t sceneId1(1u);
    ramses::Scene* scene1 = createScene1(client, sceneId1);
    scene1->flush();
    scene1->publish();

    ramses::sceneId_t sceneId2(2u);
    ramses::Scene* scene2 = createScene2(client, sceneId2);
    scene2->flush();
    scene2->publish();

    /// [Displays Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.
    // Create displays and map scenes to them
    ramses::DisplayConfig displayConfig1;
    const ramses::displayId_t display1 = renderer.createDisplay(displayConfig1);

    sceneControlAPI.setSceneMapping(sceneId1, display1);
    sceneControlAPI.setSceneState(sceneId1, ramses::RendererSceneState::Rendered);
    sceneControlAPI.flush();

    ramses::DisplayConfig displayConfig2;
    //ivi surfaces must be unique for every display
    displayConfig2.setWaylandIviSurfaceID(ramses::waylandIviSurfaceId_t(displayConfig1.getWaylandIviSurfaceID().getValue() + 1));
    const ramses::displayId_t display2 = renderer.createDisplay(displayConfig2);
    renderer.flush();

    sceneControlAPI.setSceneMapping(sceneId2, display2);
    sceneControlAPI.setSceneState(sceneId2, ramses::RendererSceneState::Rendered);
    sceneControlAPI.flush();

    /// [Displays Example]
    /// Refresh first display 60fps but limit second display to 5fps
    renderer.setFramerateLimit(display1, 60);
    renderer.setFramerateLimit(display2, 5);

    renderer.startThread();
    renderer.flush();

    ramses::MeshNode* meshScene1 = ramses::RamsesUtils::TryConvert<ramses::MeshNode>(*scene1->findObjectByName("triangle mesh node"));
    ramses::MeshNode* meshScene2 = ramses::RamsesUtils::TryConvert<ramses::MeshNode>(*scene2->findObjectByName("triangle mesh node"));

    RendererEventHandler eventHandler;
    while (!eventHandler.isWindowClosed())
    {
        renderer.dispatchEvents(eventHandler);

        meshScene1->rotate(0.f, 0.f, 1.f);
        scene1->flush();
        meshScene2->rotate(0.f, 0.f, -1.f);
        scene2->flush();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
