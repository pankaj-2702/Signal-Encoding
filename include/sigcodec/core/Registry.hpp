#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace sigcodec {

/**
 * Generic factory registry.
 *
 * Maps a string name to a function that creates an object.
 *
 * Example:
 *
 *   registry.registerType("NRZ-L", []() {
 *       return std::make_unique<NrzL>();
 *   });
 *
 *   auto coder = registry.create("NRZ-L");
 */
template <typename Interface>
class Registry {
public:

    // Function used to create an implementation object.
    using Factory = std::function<std::unique_ptr<Interface>()>;


    // --------------------------------------------------------
    // Register a new implementation
    // --------------------------------------------------------

    void registerType(const std::string& name, Factory factory)
    {
        factories_[name] = std::move(factory);
    }


    // --------------------------------------------------------
    // Create an implementation by name
    // --------------------------------------------------------

    std::unique_ptr<Interface> create(const std::string& name) const
    {
        auto it = factories_.find(name);

        if (it == factories_.end()) {
            throw std::runtime_error(
                "Unknown registered type: " + name
            );
        }

        return it->second();
    }


    // --------------------------------------------------------
    // Check whether a type exists
    // --------------------------------------------------------

    bool contains(const std::string& name) const
    {
        return factories_.find(name) != factories_.end();
    }


private:

    // Maps:
    //
    // "NRZ-L"      -> factory for NrzL
    // "NRZ-I"      -> factory for NrzI
    // "Manchester" -> factory for Manchester
    //
    std::unordered_map<std::string, Factory> factories_;
};

} // namespace sigcodec