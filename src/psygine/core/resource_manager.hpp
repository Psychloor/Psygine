//  SPDX-FileCopyrightText: 2025 Kevin Blomqvist
//  SPDX-License-Identifier: MIT

#ifndef PSYGINE_RESOURCE_MANAGER_HPP
#define PSYGINE_RESOURCE_MANAGER_HPP

#include <memory>
#include <string>
#include <unordered_map>

namespace psygine::core
{

    /**
     * @brief Abstract class for managing shared resources with caching and loading capabilities.
     *
     * This class serves as a resource manager template, allowing storage, retrieval,
     * and cleanup of shared resources. It uses an internal cache to store resources
     * by their unique paths, ensuring efficient reuse and validity checks.
     *
     * @tparam T The type of resource to be managed.
     */
    template <typename T>
    class ResourceManager
    {
    public:
        virtual ~ResourceManager() = default;

        /**
         * @brief Retrieves a shared resource by its path, loading it if not already cached.
         *
         * This method attempts to retrieve a resource from the internal cache. If the resource
         * is not present or no longer valid, it will be loaded and added to the cache before
         * being returned.
         *
         * @param path The file path or identifier of the resource to retrieve.
         * @return A `std::shared_ptr<T>` pointing to the retrieved or loaded resource.
         */
        [[nodiscard]] std::shared_ptr<T> get(const std::string& path)
        {
            if (auto it = cache_.find(path);
                it != cache_.end())
            {
                if (auto resource = it->second.lock())
                {
                    return resource;
                }

                // Expired entry: remove it to keep the cache tidy.
                cache_.erase(it);
            }

            if (auto resource = load(path))
            {
                cache_.emplace(path, resource);
                return resource;
            }

            return nullptr;
        }

        /**
         * @brief Cleans up the internal resource cache by removing expired entries.
         *
         * This method iterates through the internal cache and removes any cached
         * resource whose `std::weak_ptr` has expired. It helps to reclaim memory
         * and maintain the integrity of the resource cache by purging stale or
         * invalid references.
         */
        void cleanup() noexcept
        {
            std::erase_if(cache_, [](auto& pair)
            {
                return pair.second.expired();
            });
        }

        ResourceManager(const ResourceManager& other) = delete;
        ResourceManager(ResourceManager&& other) noexcept = delete;
        ResourceManager& operator=(const ResourceManager& other) = delete;
        ResourceManager& operator=(ResourceManager&& other) noexcept = delete;

    protected:
        /**
         * @brief Loads a resource from the given path.
         *
         * This pure virtual method must be implemented by derived classes to define
         * the specific mechanism for loading a resource. It is invoked when a resource
         * is not found in the cache and needs to be loaded.
         *
         * @param path The file path or identifier of the resource to load.
         * @return A `std::shared_ptr<T>` pointing to the loaded resource.
         */
        [[nodiscard]] virtual std::shared_ptr<T> load(const std::string& path) = 0;

        std::unordered_map<std::string, std::weak_ptr<T>> cache_;
    };
}

#endif //PSYGINE_RESOURCE_MANAGER_HPP
