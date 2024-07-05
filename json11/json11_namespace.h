#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <initializer_list>

namespace json11 {

class Json;
class JsonValue;

// ��ʾJson��������������
enum JsonType {
	NUL, NUMBER, BOOL, STRING, ARRAY, OBJECT
};
// �˽ṹ�����ڰ�������NUL�������ͣ���������û��ʲô�����ô�
struct NullStruct {
	bool operator== (NullStruct) const { return true; }
	bool operator<	(NullStruct) const { return false; }
};
// JsonArray���ڱ�ʾ����ṹ
using JsonArray = std::vector<std::shared_ptr<JsonValue>>;
// JsonObject���ڱ�ʾ����ṹ
using JsonObject = std::map<std::string, std::shared_ptr<JsonValue>>;
// shape��������ʲô�ݲ����
using shape = std::initializer_list<std::pair<std::string, json11::JsonType>>;

};