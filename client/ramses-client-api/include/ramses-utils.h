//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSES_UTILS_H
#define RAMSES_RAMSES_UTILS_H

#include "ramses-client-api/TextureEnums.h"
#include "ramses-client-api/TextureSwizzle.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/APIExport.h"

#include <vector>
#include <string>
#include <string_view>

/**
 * @defgroup UtilsAPI The Ramses Utils API
 * This group contains all of the Ramses Utility API types.
 */

namespace ramses
{
    class RamsesObject;
    class Effect;
    class Node;
    class Scene;
    class Texture2D;
    class UniformInput;
    class DataObject;
    struct MipLevelData;
    struct CubeMipLevelData;

    /**
     * @ingroup UtilsAPI
     * @brief Temporary functions for convenience. All of these can be implemented on top
     * of the RAMSES Client API, but are offered here as convenience.
     */
    class RAMSES_API RamsesUtils
    {
    public:
        /**
        * @brief Converts object to a compatible object type.
        * Object can be converted to any of its base classes.
        * Eg. MeshNode can be converted to Node, SceneObject, ClientObject or RamsesObject.
        *
        * @param[in] obj RamsesObject to convert.
        * @return Pointer to an object of a specific type,
        *         NULL if object type is not compatible with desired object class.
        */
        template <typename T>
        static const T* TryConvert(const RamsesObject& obj);

        /**
        * @copydoc TryConvert()
        **/
        template <typename T>
        static T* TryConvert(RamsesObject& obj);

        /**
        * @brief Creates a Texture from the given png file.
        *
        * @param[in] pngFilePath Path to the png file to load
        * @param[in] scene Scene the texture object is to be created in
        * @param[in] swizzle Swizzling of texture color channels
        * @param[in] name Name for the created texture
        * @return Created texture object or nullptr on error
        */
        static Texture2D*  CreateTextureResourceFromPng(const char* pngFilePath, Scene& scene, const TextureSwizzle& swizzle = {}, std::string_view name = {});

        /**
        * @brief Creates a Texture from the given png memory buffer.
        *
        * @param[in] pngData Buffer with PNG data to load
        * @param[in] scene Scene the texture object is to be created in
        * @param[in] swizzle Swizzling of texture color channels
        * @param[in] name Name for the created texture
        * @return Created texture object or nullptr on error
        */
        static Texture2D*  CreateTextureResourceFromPngBuffer(const std::vector<unsigned char>& pngData, Scene& scene, const TextureSwizzle& swizzle = {}, std::string_view name = {});

        /**
        * @brief Generate mip maps from original texture 2D data. You obtain ownership of all the
        *        data returned in the mip map data object.
        * Note, that the original texture data gets copied and represents the first mip map level.
        * @see DeleteGeneratedMipMaps for deleting generated mip maps.
        * @param[in] width Width of the original texture.
        * @param[in] height Height of the original texture.
        * @param[in] bytesPerPixel Number of bytes stored per pixel in the original texture data.
        * @param[in] data Original texture data.
        * @param[out] mipMapCount Number of generated mip map levels.
        * @return generated mip map data. In case width or height are not values to the power of two,
        *         only the original mip map level is part of the result.
        *         You are responsible to destroy the generated data, e.g. by using RamsesUtils::DeleteGeneratedMipMaps
        */
        static MipLevelData* GenerateMipMapsTexture2D(uint32_t width, uint32_t height, uint8_t bytesPerPixel, uint8_t* data, size_t& mipMapCount);

        /**
        * @brief Creates a png from image data, e.g. data generated by RamsesClientService::readPixels.
        *        The image data is expected to be in the format rgba8. Width x Height x 4 (rgba8) have
        *        to exactly match the size of the image buffer, otherwise no png will be created.
        *        Also width * height cannot exceed the size 268435455 or png creation will fail.
        *
        * @param[in] filePath Path where the png will be saved
        * @param[in] imageData Buffer with rgba8 image data that should be saved to png
        * @param[in] width Width of the image
        * @param[in] height Height of the image
        * @return Success of png creation
        */
        static bool SaveImageBufferToPng(const std::string& filePath, const std::vector<uint8_t>& imageData, uint32_t width, uint32_t height);

        /**
        * @brief Creates a png from image data, e.g. data generated by RamsesClientService::readPixels.
        *        The image data is expected to be in the format rgba8. Width x Height x 4 (rgba8) have
        *        to exactly match the size of the image buffer, otherwise no png will be created.
        *        Also width * height cannot exceed the size 268435455 or png creation will fail.
        *        The image data can be flipped vertically, as the data coming from a function like RamsesClientService::readPixels
        *        gets the data from OpenGL which has the origin in the lower left corner, whereas png has the
        *        origin in the upper left corner. So to capture what you see on screen you have to set the flag
        *        flipImageBufferVertically to true.
        *
        * @param[in] filePath Path where the png will be saved
        * @param[in] imageData Buffer with rgba8 image data that should be saved to png
        * @param[in] width Width of the image
        * @param[in] height Height of the image
        * @param[in] flipImageBufferVertically Vertical Flipping of image data
        * @return Success of png creation
        */
        static bool SaveImageBufferToPng(const std::string& filePath, std::vector<uint8_t>& imageData, uint32_t width, uint32_t height, bool flipImageBufferVertically);

        /**
        * @brief Generate mip maps from original texture cube data. You obtain ownership of all the
        *        data returned in the mip map data object.
        * Note, that the original texture data gets copied and represents the first mip map level.
        * @see DeleteGeneratedMipMaps for deleting generated mip maps.
        * @param[in] faceWidth Width of the original texture.
        * @param[in] faceHeight Height of the original texture.
        * @param[in] bytesPerPixel Number of bytes stored per pixel in the original texture data.
        * @param[in] data Original texture data. Face data is expected in order [PX, NX, PY, NY, PZ, NZ]
        * @param[out] mipMapCount Number of generated mip map levels.
        * @return generated mip map data. In case width or height are not values to the power of two,
        *         only the original mip map level is part of the result.
        *         You are responsible to destroy the generated data, e.g. using RamsesUtils::DeleteGeneratedMipMaps
        */
        static CubeMipLevelData* GenerateMipMapsTextureCube(uint32_t faceWidth, uint32_t faceHeight, uint8_t bytesPerPixel, uint8_t* data, size_t& mipMapCount);

        /**
        * @brief Deletes mip map data created with RamsesUtils::GenerateMipMapsTexture2D.
        * @param[in, out] data Generated mip map data.
        * @param[in] mipMapCount Number of mip map levels in the generated data.
        */
        static void DeleteGeneratedMipMaps(MipLevelData*& data, size_t mipMapCount);

        /**
        * @brief Deletes mip map data created with RamsesUtils::GenerateMipMapsTextureCube.
        * @param[in, out] data Generated mip map data.
        * @param[in] mipMapCount Number of mip map levels in the generated data.
        */
        static void DeleteGeneratedMipMaps(CubeMipLevelData*& data, size_t mipMapCount);

        /**
        * @brief Returns the identifier of a node, which is printed in the renderer logs. The identifier is guaranteed to be
        * unique within a Scene until the Node is destroyed. If a Node is destroyed, a newly created Node can get the identifier
        * of the destroyed Node.
        * @param[in] node The node
        * @return the identifier of the given node.
        */
        static nodeId_t GetNodeId(const Node& node);

        /**
        * @brief   Convenience method to set perspective camera frustum using FOV and aspect ratio
        *          (like in #ramses::PerspectiveCamera::setFrustum) to two #ramses::DataObject instances
        *          which are or will be bound to a #ramses::PerspectiveCamera using #ramses::Camera::bindFrustumPlanes.
        * @details Use case example: just create the two data objects, bind them to one or more cameras
        *          and then simply use this method whenever projection parameters need to change.
        *          If not all parameters need to be modified, simply query the parameter you want to keep
        *          unchanged from the camera (e.g. #ramses::PerspectiveCamera::getAspectRatio) and use the same value here.
        *
        * @param[in] fov The vertical field of view to be set, must be > 0.
        *                This is the full vertical opening angle in degrees.
        * @param[in] aspectRatio Ratio between frustum width and height, must be > 0.
        *                        This value is generally independent from the viewport width and height
        *                        but typically matches the viewport aspect ratio.
        * @param[in] nearPlane Near plane of the camera frustum, must be > 0.
        * @param[in] farPlane Far plane of the camera frustum, must be > nearPlane.
        * @param[in] frustumPlanesData Data object where resulting first 4 frustum planes will be set to, must be created with #ramses::EDataType::Vector4F.
        * @param[in] nearFarPlanesData Data object where resulting near/far frustum planes will be set to, must be created with #ramses::EDataType::Vector2F.
        * @return True for success, false otherwise.
        */
        static bool SetPerspectiveCameraFrustumToDataObjects(float fov, float aspectRatio, float nearPlane, float farPlane, DataObject& frustumPlanesData, DataObject& nearFarPlanesData);

        /**
         * @brief Convenience wrapper for RamsesClient::loadSceneFromMemory with automatic deleter
         *
         * For details refer to #ramses::RamsesClient::loadSceneFromMemory.
         * This helper function automatically adds a deleter to the provided unique_ptr that is compiled into caller side
         * and uses the caller heap.
         * This methods can be used on all platforms as a convenience helper for memory that was allocated with new[].
         *
         * On Windows this allows safe passing of ownership from caller into ramses when ramses is used as dll.
         * This method should not be used when memory was not allocated with new[]. A custom deleter should also be
         * provided when used on windows and the memory originates from yet another dll than the direct caller.
         *
         * @param[in] client The ramses client to use for loading the scene
         * @param[in] data Memory buffer with scene data.
         * @param[in] size Size in bytes of the scene data within data buffer
         * @param[in] localOnly Mark for local only optimization
         * @return The loaded ramses Scene or nullptr on error
         *
         */
        static Scene* LoadSceneFromMemory(RamsesClient& client, std::unique_ptr<unsigned char[]> data, size_t size, bool localOnly); // NOLINT(modernize-avoid-c-arrays)

        /**
         * @brief Dumps all objects of a scene which do not contribute to the visual appearance of the scene on screen.
         * This includes disabled RenderPass-es, invisible MeshNode-s, client resources which are not used by the scene, and so on.
         * The output is in text form, starts with a list of all unrequired objects and their names and concludes with a
         * statistic (number of unrequired objects out of all objects of that type)
         *
         * @param[in] scene the source scene
         */
        static void DumpUnrequiredSceneObjects(const Scene& scene);

        /**
         * @brief As #DumpUnrequiredSceneObjects but write to given stream.
         *
         * @param[in] scene the source scene
         * @param[out] out stream to write to
         */
        static void DumpUnrequiredSceneObjectsToFile(const Scene& scene, std::ostream& out);
    };

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    inline Scene* RamsesUtils::LoadSceneFromMemory(RamsesClient& client, std::unique_ptr<unsigned char[]> data, size_t size, bool localOnly)
    {
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        std::unique_ptr<unsigned char[], void(*)(const unsigned char*)> dataWithDeleter(data.release(), [](const unsigned char* ptr) { delete[] ptr; });
        return client.loadSceneFromMemory(std::move(dataWithDeleter), size, localOnly);
    }
}

#endif