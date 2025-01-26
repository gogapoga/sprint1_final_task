#pragma once

#include <filesystem>
#include <boost/json/object.hpp>

#include "model.h"

namespace json_loader {

    model::Game LoadGame(const std::filesystem::path& json_path);

    void LoadRoadsMap(const boost::json::object& map_json_object, model::Map &map);

    void LoadBuildingsMap(const boost::json::object& map_json_object, model::Map &map);

    void LoadOfficesMap(const boost::json::object& map_json_object, model::Map &map);


}  // namespace json_loader
