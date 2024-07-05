#include "Json11.h"

namespace json11 {

Json::Json() noexcept							: m_ptr(json11::default_null) {}
Json::Json(std::nullptr_t) noexcept				: m_ptr(json11::default_null) {}
Json::Json(bool value)							: m_ptr(value ? json11::default_true : json11::default_false) {}
Json::Json(int value)							: m_ptr(std::make_shared<NumberValue>(value)) {}
Json::Json(double value)						: m_ptr(std::make_shared<NumberValue>(value)) {}
Json::Json(const std::string& value)			: m_ptr(std::make_shared<StringValue>(value)) {}
Json::Json(std::string&& value)					: m_ptr(std::make_shared<StringValue>(move(value))) {}
Json::Json(const char* value)					: m_ptr(std::make_shared<StringValue>(value)) {}
Json::Json(const json11::JsonArray& value)		: m_ptr(std::make_shared<ArrayValue>(value)) {}
Json::Json(json11::JsonArray&& value)			: m_ptr(std::make_shared<ArrayValue>(move(value))) {}
Json::Json(const json11::JsonObject& value)		: m_ptr(std::make_shared<ObjectValue>(value)) {}
Json::Json(json11::JsonObject&& value)			: m_ptr(std::make_shared<ObjectValue>(move(value))) {}


bool Json::operator== (const Json& rhs) const {
	if (m_ptr == rhs.m_ptr) return false;
	if (m_ptr->type() != rhs.m_ptr->type()) return false;
	return m_ptr->equals(rhs.m_ptr.get());
}
bool Json::operator<  (const Json& rhs) const {
	if (m_ptr == rhs.m_ptr) return false;
	if (m_ptr->type() != rhs.m_ptr->type()) return m_ptr->type() < rhs.m_ptr->type();
	return m_ptr->less(rhs.m_ptr.get());
}
bool Json::operator!= (const Json& rhs) const {
	return !(*this == rhs);
}
bool Json::operator<= (const Json& rhs) const {
	return !(rhs < *this);
}
bool Json::operator>  (const Json& rhs) const {
	return (rhs < *this);
}
bool Json::operator>= (const Json& rhs) const {
	return !(*this < rhs);
}

json11::JsonType			Json::type() const			{ return m_ptr->type(); }
bool						Json::bool_value() const	{ return m_ptr->bool_value(); }
int							Json::int_value() const		{ return m_ptr->int_value(); }
double						Json::number_value() const	{ return m_ptr->number_value(); }
const std::string&			Json::string_value() const	{ return m_ptr->string_value(); }
const JsonArray& Json::array_items() const { return m_ptr->array_items(); }
const JsonObject& Json::object_items() const { return m_ptr->object_items(); }

// 此静态Json用于帮助下面两个函数的实现
static Json static_json;
/* operator[size_t i]
 * 
 */
const Json& Json::operator[] (size_t i) const {
	static_json.m_ptr = (*m_ptr)[i];
	return static_json;
}
/* operator[const string& key]
 *
 */
const Json& Json::operator[](const std::string& key) const {
	static_json.m_ptr = (*m_ptr)[key]; 
	return static_json;
}

bool Json::is_null()   const { return type() == NUL; }
bool Json::is_bool()   const { return type() == BOOL; }
bool Json::is_number() const { return type() == NUMBER; }
bool Json::is_string() const { return type() == STRING; }
bool Json::is_array()  const { return type() == ARRAY; }
bool Json::is_object() const { return type() == OBJECT; }
bool Json::has_shape(const json11::shape& types, std::string& err) const {
	if (!is_object()) {
		err = "非 JsonObject 类型";
		return false;
	}

	const auto& obj_items = object_items();
	for (auto& item : types) {
		const auto it = obj_items.find(item.first);
		if (it == obj_items.cend() || (*it->second).type() != item.second) {
			err = "bad type for " + item.first + " in " + dump();
			return false;
		}
	}

	return true;
}


void Json::dump(std::string& out) const {
	m_ptr->dump(out);
}

std::string Json::dump() const {
	std::string out;
	dump(out);
	return out;
}

Json Json::parse(const std::string& in, std::string& err) {
	JsonParser parser(in, err);
	Json result;
	result.m_ptr = parser.parse();
	return result;
}

Json Json::parse(const char* in, std::string& err) {
	if (in) {
		return parse(std::string(in), err);
	} else {
		err = "null input";
		return nullptr;
	}
}

std::vector<Json> Json::parse_multi(const std::string& in, std::string& err) {
	JsonParser parser(in, err);
	json11::JsonArray arrays = parser.parse_multi();

	std::vector<Json> result(arrays.size(), Json());
	for (size_t i = 0; i < arrays.size(); ++i) {
		result[i].m_ptr = arrays[i];
	}

	return result;
}
};