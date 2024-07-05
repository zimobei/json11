#pragma once
#include "JsonParser.cpp"

namespace json11 {
class Json final {
public:
	std::shared_ptr<JsonValue> m_ptr;
public:
	/*
	 * Json的构造函数
	 */
	Json() noexcept;
	Json(std::nullptr_t) noexcept;
	Json(bool value);
	Json(int value);
	Json(double value);
	Json(const std::string& value);
	Json(std::string&& value);
	Json(const char* value);
	Json(const JsonArray& value);
	Json(JsonArray&& value);
	Json(const JsonObject& value);
	Json(JsonObject&& value);

	Json(void*) = delete;

	/*
	 *  Json的操作符重载
	 */
	bool operator== (const Json& rhs) const;
	bool operator<  (const Json& rhs) const;
	bool operator!= (const Json& rhs) const;
	bool operator<= (const Json& rhs) const;
	bool operator>  (const Json& rhs) const;
	bool operator>= (const Json& rhs) const;

	/*
	 *  以下为一些主要的接口
	 */
	static Json parse(const std::string& in, std::string& err);
	static Json parse(const char* in, std::string& err);

	static std::vector<Json> parse_multi(const std::string& in, std::string& err);

	void dump(std::string& out) const;
	std::string dump() const;

	/*
	 * 以下为一些访问接口
	 */
	json11::JsonType type() const;

	bool is_null()   const;
	bool is_bool()   const;
	bool is_number() const;
	bool is_string() const;
	bool is_array()  const;
	bool is_object() const;
	bool has_shape(const shape& types, std::string& err) const;

	bool bool_value() const;
	int int_value() const;
	double number_value() const;
	const std::string& string_value() const;
	const JsonArray& array_items() const;
	const JsonObject& object_items() const;
	const Json& operator[](size_t t) const;
	const Json& operator[](const std::string& key) const;

}; // Json


}; // namespace json11