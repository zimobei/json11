#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <initializer_list>

namespace json11 {

class Json;
class JsonValue;

// 表示Json的六种数据类型
enum JsonType {
	NUL, NUMBER, BOOL, STRING, ARRAY, OBJECT
};
// 此结构体用于帮助构建NUL数据类型，除此以外没有什么其他用处
struct NullStruct {
	bool operator== (NullStruct) const { return true; }
	bool operator<	(NullStruct) const { return false; }
};
// JsonArray用于表示数组结构
using JsonArray = std::vector<std::shared_ptr<JsonValue>>;
// JsonObject用于表示对象结构
using JsonObject = std::map<std::string, std::shared_ptr<JsonValue>>;
// shape的作用是什么暂不清楚
using shape = std::initializer_list<std::pair<std::string, json11::JsonType>>;

};