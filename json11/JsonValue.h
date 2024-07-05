#pragma once
#include "json11_namespace.h"
#include <memory>

namespace json11 {

/* JsonValue �ӿ�
 * 
 * Json�������ݵ�ʵ�ʴ洢��JsonValue��ʵ�������
 * �˽ӿ������˶�����ݷ��ʺ��������ڰ���Json��Ľӿ�ʵ��
 */
class JsonValue {
public:
	// type() �������ڷ��ص�ǰJsonValue�ӿ���ָ���������ݵ�����
	virtual json11::JsonType type() const = 0;
	// equals() ���������ж�����JsonValue�ӿ���ָ�����������Ƿ���ͬ
	virtual bool equals(const JsonValue* other) const = 0;
	// less() ���������жϵ�ǰJsonValue�ӿ���ָ�����������Ƿ�С�ڲ���other��ָ����������
	virtual bool less(const JsonValue* other) const = 0;
	// dump() �������ڽ���ǰJsonValue�ӿ���ָ������������string����ʽ�����ڲ���out��ĩβ
	virtual void dump(std::string& out) const = 0;
	// bool_value() �������ڷ��ص�ǰJsonValue�ӿ���ָ������BOOL�������ݵ�ֵ
	virtual bool bool_value() const;
	// int_value() �������ڷ��ص�ǰJsonValue�ӿ���ָ������NUMBER�������ݵ�intֵ
	virtual int int_value() const;
	// number_value() �������ڷ��ص�ǰJsonValue�ӿ���ָ������NUMBER�������ݵ�doubleֵ
	virtual double number_value() const;
	// strig_value() �������ڷ��ص�ǰJsonValue�ӿ���ָ������STRING�������ݵ�ֵ
	virtual const std::string& string_value() const;
	// array_items() �������ڷ��ص�ǰJsonValue�ӿ���ָ������JsonArray�������ݵ�ֵ
	// ��ע�⣺�˴����ص�ֵΪshared_ptr<JsonValue> ����ָ�����ͣ�
	virtual const json11::JsonArray& array_items() const;
	// object_items() �������ڷ��ص�ǰJsonValue�ӿ���ָ������Object�������ݵ�ֵ
	// ��ע�⣺�˴����ص�ֵΪshared_ptr<JsonValue> ����ָ�����ͣ�
	virtual const json11::JsonObject& object_items() const;
	// opertor[size_t i] ������������ڷ��ص�ǰJsonValue�ӿ���ָ������JsonArray��������i��������ֵ
	virtual const std::shared_ptr<JsonValue>& operator[](size_t i) const;
	// operator[string& key] ������������ڷ��ص�ǰJsonValue�ӿ���ָ������JsonObject��������key������ֵ
	virtual const std::shared_ptr<JsonValue>& operator[](const std::string& key) const;
	
	virtual ~JsonValue() {}
};

/* Value<json11::JsonType, typename>
 * 
 * JsonValue�ӿ�ʵ����Ļ���
 * ͨ��ģ��������Ч�ؽ�JsonValue��ʵ������������json11::JsonType��Χ�� 
 * m_value���Ը���洢����
 */
template<json11::JsonType tag, typename T>
class Value : public JsonValue {
protected:
	const T m_value;
public:
	explicit Value(const T& value) : m_value(value) {}
	explicit Value(T&& value) : m_value(std::move(value)) {}
	// ʵ���ٲ���JsonValue�Ľӿڣ�����ֱ�Ӽ̳���Щ�����������ظ�ʵ��
	json11::JsonType type() const override { return tag; }
	bool equals(const JsonValue* other) const override {
		return m_value == static_cast<const Value<tag, T>*>(other)->m_value;
	}
	bool less(const JsonValue* other) const override {
		return m_value < static_cast<const Value<tag, T>*>(other)->m_value;
	}
};

/* NullValue ������
 * 
 * ����洢Json��NUL����
 * ��ȻNUL������ʾ�κ���Ϣ��������������Ȼ��Ҫʵ��dump()���� 
 */
class NullValue final : public Value<json11::JsonType::NUL, json11::NullStruct> {
public:
	NullValue() : Value({}) {}
	void dump(std::string& out) const override;
};

/* BooleanValue ������
 * 
 * ����洢Json��BOOL����
 */
class BooleanValue final : public Value<json11::JsonType::BOOL, bool> {
public:
	explicit BooleanValue(bool value) : Value(value) {}
	bool bool_value() const override;
	void dump(std::string& out) const override;
};

/* NumberValue ������
 * 
 * ����洢Json�е�NUMBER����
 */
class NumberValue final : public Value<json11::JsonType::NUMBER, double> {
public:
	explicit NumberValue(int value) : Value(value) {}
	explicit NumberValue(double value) : Value(value) {}
	double number_value() const override;
	int int_value() const override;
	bool equals(const JsonValue* other) const override;
	bool less(const JsonValue* other) const override;
	void dump(std::string& out) const override;
};

/* StringValue ������
 * 
 * ����洢Json�е�STRING����
 */
class StringValue final : public Value<json11::JsonType::STRING, std::string> {
public:
	explicit StringValue(const std::string& value) : Value(value) {}
	explicit StringValue(std::string&& value) : Value(std::move(value)) {}
	const std::string& string_value() const override;
	void dump(std::string& out) const override;
};

/* ArrayValue ������
 * 
 * ����洢Json�е�JsonArray����
 */
class ArrayValue final : public Value<json11::JsonType::ARRAY, json11::JsonArray> {
public:
	explicit ArrayValue(const json11::JsonArray& value) : Value(value) {}
	explicit ArrayValue(json11::JsonArray&& value) : Value(std::move(value)) {}
	const std::shared_ptr<JsonValue>& operator[](size_t i) const override;
	const json11::JsonArray& array_items() const override;
	void dump(std::string& out) const override;
};

/* ObjectValue ������
 * 
 * ����洢Json�е�JsonObject����
 */
class ObjectValue final : public Value<json11::JsonType::OBJECT, json11::JsonObject> {
public:
	explicit ObjectValue(const json11::JsonObject& value) : Value(value) {}
	explicit ObjectValue(json11::JsonObject&& value) : Value(std::move(value)) {}
	const std::shared_ptr<JsonValue>& operator[](const std::string& key) const override;
	const json11::JsonObject& object_items() const override;
	void dump(std::string& out) const override;
};

/* static ��ʼ���ճ�Ա
 * 
 * �ն�����Ȼ�޷��洢���ݣ�������Ȼ���ڼ�ֵ
 * ����JsonValue�ӿڶ��ԣ����ڵ���ʱʹ���߲�����ֱ��֪���ýӿ���ָ����������������ͣ�����������ڵ���type_value�������ô�������
 * �����ľ�̬��ʼ���ճ�Ա�ڴ�ʱ������Ч������Ԥ�����ִ���
 * ��Ȼ����Щ��ʼ���ճ�Ա�����û�Զ��ֹ��ˣ����������ÿ��Բ�����������
 */
static const std::shared_ptr<JsonValue> default_null = std::make_shared<NullValue>();
static const std::shared_ptr<JsonValue> default_true = std::make_shared<BooleanValue>(true);
static const std::shared_ptr<JsonValue> default_false = std::make_shared<BooleanValue>(false);
static const std::string default_string;
static const json11::JsonArray default_array;
static const json11::JsonObject default_object;
};