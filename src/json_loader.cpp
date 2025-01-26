#include "json_loader.h"
#include <fstream>
#include <iostream>
#include <boost/json/src.hpp>
#include <boost/json/parse.hpp>

using namespace std::literals;

namespace json_loader {

    model::Game LoadGame(const std::filesystem::path &json_path) {
        // Загрузить содержимое файла json_path, например, в виде строки
        std::ifstream f_input(json_path);
        if (!f_input.is_open()) {
            std::cerr << "Error open <game-config-json>"sv << std::endl;
        }
        std::string json_str(std::istreambuf_iterator<char>(f_input), {});
        f_input.close();

        // Распарсить строку как JSON, используя boost::json::parse
        auto parse = boost::json::parse(json_str);

        // Загрузить модель игры из файла
        model::Game game;
        auto maps = parse.as_object().at(model::ConstJsonKey::TEXT_MAPS).as_array();

        for (const auto &m: maps) {
            boost::json::object map_json_object = m.as_object();
            model::Map map{model::Map::Id{map_json_object.at(model::ConstJsonKey::TEXT_ID).as_string().c_str()},
                           static_cast<std::string>(map_json_object.at(model::ConstJsonKey::TEXT_NAME).as_string())};

            LoadRoadsMap(map_json_object, map);

            LoadBuildingsMap(map_json_object, map);

            LoadOfficesMap(map_json_object, map);

            game.AddMap(map);
        }
        return game;
    }

    void LoadRoadsMap(const boost::json::object &map_json_object, model::Map &map) {
        auto roads = map_json_object.at(model::ConstJsonKey::TEXT_ROADS).as_array();
        for (const auto &r: roads) {
            const auto &map_road = r.as_object();
            if (map_road.count(model::ConstJsonKey::TEXT_X1) > 0) {
                model::Road road{model::Road::HORIZONTAL, model::Point{static_cast<int>(map_road.at(model::ConstJsonKey::TEXT_X0).as_int64()),
                                                                       static_cast<int>(map_road.at(model::ConstJsonKey::TEXT_Y0).as_int64())},
                                 static_cast<int>(map_road.at(model::ConstJsonKey::TEXT_X1).as_int64())};
                map.AddRoad(road);
            } else {
                model::Road road{model::Road::VERTICAL, model::Point{static_cast<int>(map_road.at(model::ConstJsonKey::TEXT_X0).as_int64()),
                                                                     static_cast<int>(map_road.at(model::ConstJsonKey::TEXT_Y0).as_int64())},
                                 static_cast<int>(map_road.at(model::ConstJsonKey::TEXT_Y1).as_int64())};
                map.AddRoad(road);
            }
        }
    }

    void LoadBuildingsMap(const boost::json::object &map_json_object, model::Map &map) {
        auto buildings = map_json_object.at(model::ConstJsonKey::TEXT_BUILDINGS).as_array();
        for (const auto &b: buildings) {
            const auto &map_building = b.as_object();
            model::Building building{model::Rectangle{model::Point{static_cast<int>(map_building.at(model::ConstJsonKey::TEXT_X).as_int64()),
                                                                   static_cast<int>(map_building.at(model::ConstJsonKey::TEXT_Y).as_int64())},
                                                      model::Size{static_cast<int>(map_building.at(model::ConstJsonKey::TEXT_W).as_int64()),
                                                                  static_cast<int>(map_building.at(model::ConstJsonKey::TEXT_H).as_int64())}}};
            map.AddBuilding(building);
        }
    }

    void LoadOfficesMap(const boost::json::object &map_json_object, model::Map &map) {
        auto offices = map_json_object.at(model::ConstJsonKey::TEXT_OFFICES).as_array();
        for (const auto &o: offices) {
            const auto &map_office = o.as_object();
            model::Office office{model::Office::Id{map_office.at(model::ConstJsonKey::TEXT_ID).as_string().c_str()},
                                 model::Point{static_cast<int>(map_office.at(model::ConstJsonKey::TEXT_X).as_int64()),
                                              static_cast<int>(map_office.at(model::ConstJsonKey::TEXT_Y).as_int64())},
                                 model::Offset{static_cast<int>(map_office.at(model::ConstJsonKey::TEXT_OFFSET_X).as_int64()),
                                               static_cast<int>(map_office.at(model::ConstJsonKey::TEXT_OFFSET_Y).as_int64())}};
            map.AddOffice(office);
        }
    }

}  // namespace json_loader
