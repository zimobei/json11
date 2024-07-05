#pragma once
#include "json11_namespace.h"
#include <memory>

namespace json11 {

/* JsonValue 接口
 * 
 * Json库中数据的实际存储有JsonValue的实现类完成
 * 此接口声明了多个数据访问函数，用于帮助Json类的接口实现
 */
class JsonValue {
public:
	// type() 函数用于返回当前JsonValue接口所指向类中数据的类型
	virtual json11::JsonType type() const = 0;
	// equals() 函数用于判断两个JsonValue接口所指向类中数据是否相同
	virtual bool equals(const JsonValue* other) const = 0;
	// less() 函数用于判断当前JsonValue接口所指向类中数据是否小于参数other所指向类中数据
	virtual bool less(const JsonValue* other) const = 0;
	// dump() 函数用于将当前JsonValue接口所指向类中数据以string的形式保存在参数out的末尾
	virtual void dump(std::string& out) const = 0;
	// bool_value() 函数用于返回当前JsonValue接口所指向类中BOOL类型数据的值
	virtual bool bool_value() const;
	// int_value() 函数用于返回当前JsonValue接口所指向类中NUMBER类型数据的int值
	virtual int int_value() const;
	// number_value() 函数用于返回当前JsonValue接口所指向类中NUMBER类型数据的double值
	virtual double number_value() const;
	// strig_value() 函数用于返回当前JsonValue接口所指向类中STRING类型数据的值
	virtual const std::string& string_value() const;
	// array_items() 函数用于返回当前JsonValue接口所指向类中JsonArray类型数据的值
	// （注意：此处返回的值为shared_ptr<JsonValue> 智能指针类型）
	virtual const json11::JsonArray& array_items() const;
	// object_items() 函数用于返回当前JsonValue接口所指向类中Object类型数据的值
	// （注意：此处返回的值为shared_ptr<JsonValue> 智能指针类型）
	virtual const json11::JsonObject& object_items() const;
	// opertor[size_t i] 操作运算符用于返回当前JsonValue接口所指向类中JsonArray类型数据i索引处的值
	virtual const std::shared_ptr<JsonValue>& operator[](size_t i) const;
	// operator[string& key] 操作运算符用于返回当前JsonValue接口所指向类中JsonObject类型数据key键处的值
	virtual const std::shared_ptr<JsonValue>& operator[](const std::string& key) const;
	
	virtual ~JsonValue() {}
};

/* Value<json11::JsonType, typename>
 * 
 * JsonValue接口实现类的基类
 * 通过模板声明有效地将JsonValue的实现类限制在了json11::JsonType范围内 
 * m_value属性负责存储数据
 */
template<json11::JsonType tag, typename T>
class Value : public JsonValue {
protected:
	const T m_value;
public:
	explicit Value(const T& value) : m_value(value) {}
	explicit Value(T&& value) : m_value(std::move(value)) {}
	// 实现少部分JsonValue的接口，子类直接继承这些函数，无需重复实现
	json11::JsonType type() const override { return tag; }
	bool equals(const JsonValue* other) const override {
		return m_value == static_cast<const Value<tag, T>*>(other)->m_value;
	}
	bool less(const JsonValue* other) const override {
		return m_value < static_cast<const Value<tag, T>*>(other)->m_value;
	}
};

/* NullValue 派生类
 * 
 * 负责存储Json中NUL数据
 * 虽然NUL本身不表示任何信息，但此派生类仍然需要实现dump()函数 
 */
class NullValue final : public Value<json11::JsonType::NUL, json11::NullStruct> {
public:
	NullValue() : Value({}) {}
	void dump(std::string& out) const override;
};

/* BooleanValue 派生类
 * 
 * 负责存储Json中BOOL数据
 */
class BooleanValue final : public Value<json11::JsonType::BOOL, bool> {
public:
	explicit BooleanValue(bool value) : Value(value) {}
	bool bool_value() const override;
	void dump(std::string& out) const override;
};

/* NumberValue 派生类
 * 
 * 负责存储Json中的NUMBER数据
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

/* StringValue 派生类
 * 
 * 负责存储Json中的STRING数据
 */
class StringValue final : public Value<json11::JsonType::STRING, std::string> {
public:
	explicit StringValue(const std::string& value) : Value(value) {}
	explicit StringValue(std::string&& value) : Value(std::move(value)) {}
	const std::string& string_value() const override;
	void dump(std::string& out) const override;
};

/* ArrayValue 派生类
 * 
 * 负责存储Json中的JsonArray数据
 */
class ArrayValue final : public Value<json11::JsonType::ARRAY, json11::JsonArray> {
public:
	explicit ArrayValue(const json11::JsonArray& value) : Value(value) {}
	explicit ArrayValue(json11::JsonArray&& value) : Value(std::move(value)) {}
	const std::shared_ptr<JsonValue>& operator[](size_t i) const override;
	const json11::JsonArray& array_items() const override;
	void dump(std::string& out) const override;
};

/* ObjectValue 派生类
 * 
 * 负责存储Json中的JsonObject数据
 */
class ObjectValue final : public Value<json11::JsonType::OBJECT, json11::JsonObject> {
public:
	explicit ObjectValue(const json11::JsonObject& value) : Value(value) {}
	explicit ObjectValue(json11::JsonObject&& value) : Value(std::move(value)) {}
	const std::shared_ptr<JsonValue>& operator[](const std::string& key) const override;
	const json11::JsonObject& object_items() const override;
	void dump(std::string& out) const override;
};

/* static 初始化空成员
 * 
 * 空对象虽然无法存储数据，但是仍然存在价值
 * 对于JsonValue接口而言，由于调用时使用者并不能直接知道该接口所指向派生类的数据类型，所以难免存在调用type_value函数调用错误的情况
 * 下述的静态初始化空成员在此时可以有效地用于预防这种错误
 * 当然，这些初始化空成员的作用还远不止如此，后续的作用可以查阅其他代码
 */
static const std::shared_ptr<JsonValue> default_null = std::make_shared<NullValue>();
static const std::shared_ptr<JsonValue> default_true = std::make_shared<BooleanValue>(true);
static const std::shared_ptr<JsonValue> default_false = std::make_shared<BooleanValue>(false);
static const std::string default_string;
static const json11::JsonArray default_array;
static const json11::JsonObject default_object;
};