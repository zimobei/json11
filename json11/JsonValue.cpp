#include "JsonValue.h"
#include <cmath>
using namespace json11;
/*
 * ����JsonValue�ӿڵ�Ĭ��ʵ�֣�����static ��ʼ���ճ�Ա�� 
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
 *  ����Value<json11::JsonType, typename>������ӿڵ�ʵ�� 
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
 * ��JsonValue����ΪJsonArray��vector<shared_ptr<JsonValue> >���������øýӿ�
 * ��������λ��i����shared_ptr<JsonValue>����i�����ڣ��򷵻ؿ�
 */
const std::shared_ptr<JsonValue>& ArrayValue::operator[](size_t i) const {
	if (i >= m_value.size()) return json11::default_null;
	else return m_value[i];
}

/* ObjectValue::operator[const string& key]
 * 
 * ��JsonValue����ΪJsonObject��map<string, shared_ptr<JsonValue> >���������øýӿ�
 * ����key������Ӧ��shared_ptr<JsonValue>����key�������ڣ��򷵻ؿ�
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
 * ����ǰValue�е����ݸ���������JsonTypeת��Ϊstring��ʽ����ӵ����ò���out��β�� 
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
		// ���ַ�ʽ�ô��Ƿ�װ�ԽϺã��������ƺ�����Ӱ��
		// ������Json11ԭ��һ���������е�dump(const T&, string&)�Ƴ�Value<JsonType, T>�У��Դ��������
		const std::shared_ptr<JsonValue> key_value = std::make_shared<StringValue>(kv.first);
		key_value->dump(out);

		out += ": ";

		(*kv.second).dump(out);
		first = false;
	}
	out += "}";
}

