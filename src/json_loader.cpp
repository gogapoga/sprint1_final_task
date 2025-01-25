#include "json_loader.h"
#include <fstream>

#include <iostream>

//#define BOOST_JSON_NO_LIB
//#define BOOST_CONTAINER_NO_LIB
#include <boost/json/src.hpp>
#include <boost/json/parse.hpp>

using namespace std::literals;

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path) {
    // Загрузить содержимое файла json_path, например, в виде строки
    std::ifstream f_input(json_path);
    if(!f_input.is_open()){
        std::cerr << "Error open <game-config-json>"sv << std::endl;
    }
    std::string json_str(std::istreambuf_iterator<char>(f_input), {});
    f_input.close();
    // Распарсить строку как JSON, используя boost::json::parse
    auto parse = boost::json::parse(json_str);
    // Загрузить модель игры из файла
    model::Game game;
    auto maps = parse.as_object().at("maps").as_array();
    for(const auto& m : maps)
    {
        auto map_object = m.as_object();
        model::Map map{ model::Map::Id{map_object.at("id").as_string().c_str()}, static_cast<std::string>(map_object.at("name").as_string()) };
        auto roads = map_object.at("roads").as_array();
        for (const auto& r : roads) {
            const auto& map_road = r.as_object();
            if (map_road.count("x1") > 0) {
                model::Road road{ model::Road::HORIZONTAL, model::Point { static_cast<int>(map_road.at("x0").as_int64()),  static_cast<int>(map_road.at("y0").as_int64())},
                    static_cast<int>(map_road.at("x1").as_int64()) };
                map.AddRoad(road);
            }
            else {
                model::Road road{ model::Road::VERTICAL,  model::Point { static_cast<int>(map_road.at("x0").as_int64()),  static_cast<int>(map_road.at("y0").as_int64())},
                    static_cast<int>(map_road.at("y1").as_int64()) };
                map.AddRoad(road);
            }
        }
        auto buildings = map_object.at("buildings").as_array();
        for (const auto& b : buildings) {
            const auto& map_building = b.as_object();
            model::Building building{ model::Rectangle { model::Point { static_cast<int>(map_building.at("x").as_int64()),  static_cast<int>(map_building.at("y").as_int64())},
                model::Size { static_cast<int>(map_building.at("w").as_int64()),  static_cast<int>(map_building.at("h").as_int64())}} };
            map.AddBuilding(building);
        }
        auto offices = map_object.at("offices").as_array();
        for (const auto& o : offices) {
            const auto& map_office = o.as_object();
            model::Office office{ model::Office::Id{map_office.at("id").as_string().c_str()},  model::Point{ static_cast<int>(map_office.at("x").as_int64()),  static_cast<int>(map_office.at("y").as_int64())},  model::Offset { static_cast<int>(map_office.at("offsetX").as_int64()),  static_cast<int>(map_office.at("offsetY").as_int64())} };
            map.AddOffice(office);
        }
        game.AddMap(map);
    }
    return game;
}

}  // namespace json_loader
