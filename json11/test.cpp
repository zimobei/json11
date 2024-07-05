#include "Json11.h"
#include <iostream>

using namespace std;
using namespace json11;

void fun1() {
	const vector<string> strs = {
		R"(null)",
		R"(true)",
		R"(false)",
		R"(123)",
		R"("this is a test")",
		R"([ null, 1, 2, true, [false] ])",
		R"({ 
			// 此为单行注释
			"key1" : "value1",
			/* 
			 *  此为多行注释
			 */
			"key2" : null,
			"key3" : 456,
			"key4" : false,
			"key5" : [ 3, 4, "this is value5" ]
			 })"
	};

	string err;
	for (const string& str : strs) {
		Json json = Json::parse(str, err);
		cout << "json = " << json.dump() << endl;
		cout << "err = " << err << endl;
		cout << endl;
	}
}

void fun2() {

	const string str2 = R"(

// 单行注释
null // 注意：这里后面不用加逗号
/*  
* 多行注释
*
*/
{ 
			// 此为单行注释
			"key1" : "value1",
			/* 
			 *  此为多行注释
			 */
			"key2" : null,
			"key3" : 456,
			"key4" : false,
			"key5" : [ 3, 4, "this is value5" ]
			 }

	)";

	string err;
	vector<Json> vec_jsons = Json::parse_multi(str2, err);
	for (const Json& json : vec_jsons) {
		cout << "json =  " << json.dump() << endl;
		cout << "err = " << err << endl;
		cout << endl;
	}

	cout << vec_jsons[1]["key5"][2].dump() << endl;

}

void fun3() {
	JsonArray ja{
		make_shared<NullValue>(),
		make_shared<BooleanValue>(true),
		make_shared<NumberValue>(123),
		make_shared<StringValue>("str")
	};
	JsonArray jaa{
		make_shared<ArrayValue>(ja),
		make_shared<ArrayValue>(ja),
	};

	const Json js(jaa);

	cout << js.dump() << endl;
	cout << js[0].dump() << endl;
	cout << js[0][3].dump() << endl;
}

void fun4() {
	JsonObject jo{
		{ "1", make_shared<NullValue>() },
		{ "2", make_shared<BooleanValue>(true) },
		{ "3", make_shared<NumberValue>(123) },
		{ "4", make_shared<StringValue>("str")}
	};
	JsonObject joo{
		{ "1", make_shared<ObjectValue>(jo)},
		{ "2", make_shared<ObjectValue>(jo)},
	};

	const Json js(joo);

	cout << js.dump() << endl;
	cout << js["1"].dump() << endl;
	cout << js["1"]["3"].dump() << endl;

}

void fun5() {
	const string str = R"([null, 1, true, ["string", {"key1" : "value1", "key2" : [false]}]])";
	string err;
	const Json js1 = Json::parse(str, err);

	cout << js1.type() << endl;
	cout << js1.dump() << endl;
	cout << js1[3][1]["key2"][0].dump() << endl;

}

void fun6() {
	const Json js1;
	cout << js1.type() << "  " << js1.dump() << endl;

	const Json js2(1);
	cout << js2.type() << "  " << js2.dump() << "  " << js2.int_value() << endl;

	const Json js3(3.1415926);
	cout << js3.type() << "  " << js3.dump() << "  " << js3.number_value() << endl;

	const Json js4(true);
	cout << js4.type() << "  " << js4.dump() << "  " << js4.bool_value() << endl;

	const Json js5("this is a string");
	cout << js5.type() << "  " << js5.dump() << "  " << js5.string_value() << endl;

	const JsonArray ja{
		make_shared<NullValue>(),
		make_shared<BooleanValue>(false),
		make_shared<NumberValue>(100),
		make_shared<StringValue>("this is a array string")
	};
	const Json js6(ja);
	cout << js6.type() << "  " << js6.dump() << endl;

	const JsonObject jb{
		{ "key1" , make_shared<StringValue>("this is a object string") },
		{ "key2" , make_shared<ArrayValue>(ja) },
		{ "key3" , make_shared<NullValue>() }
	};
	const Json js7(jb);
	cout << js7.type() << "  " << js7.dump() << endl;
}

int main() {

	fun6();

	return 0;
}