#include "JsonValue.h"
#include <cmath>
using namespace json11;
/*
 * 定义JsonValue接口的默认实现（返回static 初始化空成员） 
 */
bool								JsonValue::bool_value() const					{ return false; }
int									JsonValue::int_value() const					{ return 0; }
double								JsonValue::number_value() const					{ return 0; }
const std::string&					JsonValue::string_value() const					{ return json11::default_string; }
const json11::JsonArray&			JsonValue::array_items() const					{ return json11::default_array; }
const json11::JsonObject&			JsonValue::object_items() const					{ return json11::default_object; }
const std::shared_ptr<JsonValue>&	JsonValue::operator[](size_t) const				{ return json11::default_null; }
const std::shared_ptr<JsonValue>&	JsonValue::operator[](const std::string&) const	{ return json11::default_null; }

/*
 *  定义Value<json11::JsonType, typename>派生类接口的实现 
 */
bool						BooleanValue::bool_value() const					{ return m_value; }
int							NumberValue::int_value() const						{ return static_cast<int>(m_value); }
double						NumberValue::number_value() const					{ return m_value; }
bool						NumberValue::equals(const JsonValue* other) const	{ return m_value == other->number_value(); }
bool						NumberValue::less(const JsonValue* other) const		{ return m_value < other->number_value(); }
const std::string&			StringValue::string_value() const					{ return m_value; }
const json11::JsonArray&	ArrayValue::array_items() const						{ return m_value; }
const json11::JsonObject&	ObjectValue::object_items() const					{ return m_value; }

/* ArrayValue::operator[size_t i]
 * 
 * 若JsonValue类型为JsonArray（vector<shared_ptr<JsonValue> >），则会调用该接口
 * 返回索引位置i处的shared_ptr<JsonValue>；若i不存在，则返回空
 */
const std::shared_ptr<JsonValue>& ArrayValue::operator[](size_t i) const {
	if (i >= m_value.size()) return json11::default_null;
	else return m_value[i];
}

/* ObjectValue::operator[const string& key]
 * 
 * 若JsonValue类型为JsonObject（map<string, shared_ptr<JsonValue> >），则会调用该接口
 * 返回key键所对应的shared_ptr<JsonValue>；若key键不存在，则返回空
 */
const std::shared_ptr<JsonValue>& ObjectValue::operator[](const std::string& key) const {
	auto iter = m_value.find(key);
	if (iter == m_value.end()) {
		return json11::default_null;
	} else {
		return iter->second;
	}
}

/* dump()
 *  
 * 将当前Value中的数据根据其类型JsonType转化为string形式，添加到引用参数out的尾部 
 */
void NullValue::dump(std::string& out) const {
	out += "null";
}
void BooleanValue::dump(std::string& out) const {
	if (m_value) out += "true";
	else out += "false";
}
void NumberValue::dump(std::string& out) const {
	if (std::isfinite(m_value)) {
		char buf[32];
		snprintf(buf, sizeof buf, "%.17g", m_value);
		out += buf;
	} else {
		out += "null";
	}
}
void StringValue::dump(std::string& out) const {
	out += '"';
	for (size_t i = 0; i < m_value.size(); ++i) {
		const char ch = m_value[i];
		if (ch == '\\') {
			out += R"(\\)";
		} else if (ch == '"') {
			out += R"(\")";
		} else if (ch == '\b') {
			out += R"(\b)";
		} else if (ch == '\f') {
			out += R"(\f)";
		} else if (ch == '\n') {
			out += R"(\n)";
		} else if (ch == '\r') {
			out += R"(\r)";
		} else if (ch == '\t') {
			out += R"(\t)";
		} else if (static_cast<uint8_t>(ch) <= 0x1f) {
			char buf[8];
			snprintf(buf, sizeof buf, "\\u%04x", ch);
			out += buf;
		} else if (static_cast<uint8_t>(ch) == 0xe2 && static_cast<uint8_t>(m_value[i + 1]) == 0x80
				   && static_cast<uint8_t>(m_value[i + 2]) == 0xa8) {
			out += "\\u2028";
			i += 2;
		} else if (static_cast<uint8_t>(ch) == 0xe2 && static_cast<uint8_t>(m_value[i + 1]) == 0x80
				   && static_cast<uint8_t>(m_value[i + 2]) == 0xa9) {
			out += "\\u2029";
			i += 2;
		} else {
			out += ch;
		}
	}
	out += '"';
}
void ArrayValue::dump(std::string& out) const {
	bool first = true;
	out += "[";
	for (const std::shared_ptr<JsonValue>& v : m_value) {
		if (!first) out += ", ";
		(*v).dump(out);
		first = false;
	}
	out += "]";
}
void ObjectValue::dump(std::string& out) const {
	bool first = true;
	out += "{";
	for (const auto& kv : m_value) {
		if (!first) out += ", ";
		// 这种方式好处是封装性较好，但性能似乎有所影响
		// 可以向Json11原版一样，将所有的dump(const T&, string&)移出Value<JsonType, T>中，以此提高性能
		const std::shared_ptr<JsonValue> key_value = std::make_shared<StringValue>(kv.first);
		key_value->dump(out);

		out += ": ";

		(*kv.second).dump(out);
		first = false;
	}
	out += "}";
}

