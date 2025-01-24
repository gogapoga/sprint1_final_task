#pragma once
#include "http_server.h"
#include "model.h"
#include <iostream>
#include <boost/json/object.hpp>
#include <boost/json/serialize.hpp>

#define BOOST_BEAST_USE_STD_STRING_VIEW



namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;

using namespace std::literals;



class RequestHandler {
public:
    explicit RequestHandler(model::Game& game)
        : game_{game} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        // Обработать запрос request и отправить ответ, используя send
        const auto text_response = [&req](http::status status, std::string_view body) {
            return MakeStringResponse(status, body, req.version(), req.keep_alive());
        };
        auto method = req.base().method();
        if ((method == http::verb::get) || (method == http::verb::head)) {
            std::vector<std::string_view> param = Split(req.base().target(), '/');
            if (param.empty()) {
                send(text_response(static_cast<http::status>(404),  ""));
                return;
            }
            if (!(param.at(0) == "api")) {
                send(text_response(static_cast<http::status>(404), ""));
                return;
            }
            if ((param.size() < 3) || (param.size() > 4) || !(param.at(1) == "v1") || !(param.at(2) == "maps")) {
                boost::json::object json_bad_request;
                json_bad_request["code"] = "badRequest";
                json_bad_request["message"] = "Bad request";
                std::string str = boost::json::serialize(json_bad_request);
                std::ostringstream oss;
                oss << json_bad_request;
                send(text_response(static_cast<http::status>(400), oss.str()));
                return;
            }
            if(param.size() == 4) {
                auto body = GetMapJson(std::string(param.at(3)));
                if(body.empty()){
                    boost::json::object json_map_not_found;
                    json_map_not_found["code"] = "mapNotFound";
                    json_map_not_found["message"] = "Map not found";
                    std::ostringstream oss;
                    oss << json_map_not_found;
                    send(text_response(static_cast<http::status>(404), oss.str()));
                } else{
                    std::ostringstream oss;
                    oss << body;
                    send(text_response(http::status::ok, oss.str()));
                }
            }else{
                boost::json::array body = GetMapsJson();
                std::ostringstream oss;
                oss <<  body;
                send(text_response(http::status::ok, oss.str()));
            }
        }else {
            send(text_response(http::status::method_not_allowed, "Invalid method"sv));
        }
    }

private:
    using StringResponse = http::response<http::string_body>;
    static StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version, bool keep_alive, std::string_view content_type =  ContentType::APPLICATION_JSON) {
        StringResponse response(status, http_version);
        response.set(http::field::content_type, content_type);
        response.body() =  body;
        response.content_length(body.size());
        response.keep_alive(keep_alive);
        return response;
    }
    struct ContentType {
        ContentType() = delete;
        constexpr static std::string_view TEXT_HTML = "text/html"sv;
        constexpr static std::string_view APPLICATION_JSON = "application/json"sv;
    };
    static std::vector<std::string_view> Split(std::string_view string, char delim) {
        std::vector<std::string_view> result;
        size_t pos = 0;
        while (pos < string.length()) {
            auto delim_pos = string.find(delim, pos);
            if (delim_pos == std::string_view::npos) {
                delim_pos = string.size();
            }
            if (auto substr = string.substr(pos, delim_pos - pos); !substr.empty()) {
                result.push_back(substr);
            }
            pos = delim_pos + 1;
        }
        return result;
    }
    boost::json::array GetMapsJson(){
        boost::json::array json_result;
        const auto vector_maps = game_.GetMaps();
        for(const auto& map : vector_maps){
            boost::json::object json_map;
            json_map["id"] = *map.GetId();
            json_map["name"] = map.GetName();
            json_result.emplace_back(json_map);
        }
        return json_result;
    }
    boost::json::object GetMapJson(const std::string& id){
        boost::json::object json_result;
        const auto ptr_map = game_.FindMap(model::Map::Id{id});
        if(ptr_map == nullptr) return json_result;
        json_result["id"] = *(ptr_map->GetId());
        json_result["name"] = ptr_map->GetName();
        boost::json::array roads = GetMapRoadsJson(ptr_map->GetRoads());
        json_result["roads"] = roads;
        boost::json::array buildings = GetMapBuildingsJson(ptr_map->GetBuildings());
        json_result["buildings"] = buildings;
        boost::json::array offices = GetMapOfficesJson(ptr_map->GetOffices());
        json_result["offices"] = offices;
        return json_result;
    }
    static boost::json::array GetMapRoadsJson(const model::Map::Roads& roads) {
        boost::json::array json_result;
        for(const auto& road : roads){
            boost::json::object json_road;
            json_road["x0"] = road.GetStart().x;
            json_road["y0"] = road.GetStart().y;
            if(road.IsHorizontal()){
                json_road["x1"] = road.GetEnd().x;
            }else{
                json_road["y1"] = road.GetEnd().y;
            }
            json_result.emplace_back(json_road);
        }
        return json_result;
    }
    static boost::json::array GetMapBuildingsJson(const model::Map::Buildings& buildings) {
        boost::json::array json_result;
        for(const auto& building : buildings){
            boost::json::object json_building;
            json_building["x"] = building.GetBounds().position.x;
            json_building["y"] = building.GetBounds().position.y;
            json_building["w"] = building.GetBounds().size.width;
            json_building["h"] = building.GetBounds().size.height;
            json_result.emplace_back(json_building);
        }
        return json_result;
    }
    static boost::json::array GetMapOfficesJson(const model::Map::Offices& offices) {
        boost::json::array json_result;
        for(const auto& office : offices){
            boost::json::object json_office;
            json_office["id"] = *office.GetId();
            json_office["x"] = office.GetPosition().x;
            json_office["y"] = office.GetPosition().y;
            json_office["offsetX"] = office.GetOffset().dx;
            json_office["offsetY"] = office.GetOffset().dy;
            json_result.emplace_back(json_office);
        }
        return json_result;
    }
    model::Game& game_;
};

}  // namespace http_handler
