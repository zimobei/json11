#include "JsonValue.h"
#include <cassert>
#include <iostream>

namespace json11 {
/* JsonParser 
 * 
 * 负责实现将string解析成JsonValue的工具类
 */
class JsonParser final {
private:
	const std::string& str; // 表示需要解析的string
	size_t i; // 表示str的索引指针，初始时为0
	std::string& err; // 用于记录string解析过程中发生的错误
	bool has_fail; // 记录当前JsonParser对象在解析过程中是否发生了错误，初始时为false
	const int max_depth; // JsonObject中的JsonValue存在嵌套的情况，若嵌套层次过多，则对性能要求过高，max_depth用于限制这种嵌套层次
public:
	JsonParser(const std::string& str_v, size_t i_v, std::string& err_v) 
		: str(str_v), i(i_v), err(err_v), has_fail(false), max_depth(200){}
	JsonParser(const std::string& str_v, std::string& err_v) 
		: JsonParser(str_v, 0, err_v){}
private:
	/* FormatChar()
	*
	* 将char参数c格式化为string的格式输出
	* 为什么要这么做？
	* 在下述代码中，对err的更新主要是通过 err += "报错信息" + char来表示的（其中char即为解析过程出错时指针i所指向的字符）
	* 但是string本身并不支持这种语句，所以在此需要将cahr转化为string的形式（后续会进行优化）
	*/
	inline std::string FormatChar(char c) {
		char buf[12];
		if (static_cast<uint8_t>(c) >= 0x20 && static_cast<uint8_t>(c) <= 0x7f) {
			snprintf(buf, sizeof buf, "'%c' (%d)", c, c);
		} else {
			snprintf(buf, sizeof buf, "(%d)", c);
		}
		return std::string(buf);
	}

	/* in_range()
	*
	* 判断第一个参数x是否处于第二个参数lower和第三参数upper之间
	* 处于则返回true，否则返回false
	*/
	inline bool in_range(long x, long lower, long upper) {
		return (x >= lower && x <= upper);
	}

	/* encode_utf8()
	*
	* 将pt所对应的utf8字码添加到out尾部
	*/
	void encode_utf8(long pt, std::string& out) {
		if (pt < 0) return;
		if (pt < 0x80) {
			out += static_cast<char>(pt);
		} else if (pt < 0x800) {
			out += static_cast<char>((pt >> 6) | 0xC0);
			out += static_cast<char>((pt & 0x3F) | 0x80);
		} else if (pt < 0x10000) {
			out += static_cast<char>((pt >> 12) | 0xE0);
			out += static_cast<char>(((pt >> 6) & 0x3F) | 0x80);
			out += static_cast<char>((pt & 0x3F) | 0x80);
		} else {
			out += static_cast<char>((pt >> 18) | 0xF0);
			out += static_cast<char>(((pt >> 12) & 0x3F) | 0x80);
			out += static_cast<char>(((pt >> 6) & 0x3F) | 0x80);
			out += static_cast<char>((pt & 0x3F) | 0x80);
		}
	}

	/* fail
	 * 
	 * 标记当前JsonParser为fail状态，同时更新err 为对应的报错信息
	 */
	template <typename T>
	T fail(std::string&& msg, const T err_ret) {
		if (!has_fail) {
			err += "报错信息：" + std::move(msg) + ", 错误位置: " + str.substr(i, 6);
		}
		has_fail = true;
		return err_ret;
	}

	std::shared_ptr<JsonValue> fail(std::string&& msg) {
		return fail(std::move(msg), json11::default_null);
	}

	/* consume_whitespace()
	 *
	 * 跳过字符串的空白部分
	 */
	void consume_whitespace() {
		while (str[i] == ' ' || str[i] == '\r' || str[i] == '\n' || str[i] == '\t')
			++i;
	}

	/* consume_comment()
	 *
	 * 确认当前字符串是否为注释
	 * 如果是注释，则跳过，并返回true；否则返回false，并更新err
	 */
	bool consume_comment() {
		bool comment_found = false;
		if (str[i] == '/') {
			++i;
			// 只有一个'/'错误
			if (i == str.size())
				return fail(R"(只有一个/，无法判断是否为注释)", false);
			// 单行注释
			if (str[i] == '/') {
				++i;
				while (i < str.size() && str[i] != '\n') ++i;
				comment_found = true;
			}
			// 多行注释
			else if (str[i] == '*') {
				++i;
				// 由于多行注释一定为 "/* ... */"的格式
				// 所以注释长度至少为4
				// 即当前检索到的'*'的下标 + 2  <= str.size() 
				if (i > str.size() - 2)
					return fail("多行注释存在错误", false);
				// 一直检索，直到找到"*/"为止
				while (!(str[i] == '*' && str[i + 1] == '/')) {
					++i;
					if (i > str.size() - 2)
						return fail("多行注释存在错误", false);
				}
				i += 2;
				comment_found = true;
			}
			// 既非单行注释，又非多行注释
			else {
				return fail(R"(注释存在错误)", false);
			}
		}
		return comment_found;
	}

	/* consume_garbage()
	 *
	 * 跳过注释和空白
	 */
	void consume_garbage() {
		consume_whitespace();
		bool comment_found = false;
		do {
			comment_found = consume_comment();
			if (has_fail) return;
			consume_whitespace();
		} while (comment_found);
	}

	/* get_next_token()
	 *
	 * 返回第一个有效字符（忽略注释与空白）
	 * 并自动后移一位
	 */
	char get_next_token() {
		consume_garbage();
		if (has_fail) return static_cast<char>(0);
		if (i == str.size())
			return fail("string末尾处不存在json内容", static_cast<char>(0));
		return str[i++];
	}

	/* expect()
	 *
	 * 从str的索引i开始检索，检索长度为expected的长度，判断这一部分是否与expected相同
	 * 相同则返回res，不同则返回fail错误
	 * 主要用于检索null、true、false等情况
	 */
	std::shared_ptr<JsonValue> expect(const std::string& expected, std::shared_ptr<JsonValue> res) {
		assert(i != 0);
		--i;
		if (str.compare(i, expected.length(), expected) == 0) {
			i += expected.length();
			return res;
		} else {
			return fail("parse error: expected " + expected + ", got " + str.substr(i, expected.length()));
		}
	}

	std::shared_ptr<JsonValue> parse_number() {
		size_t start_pos = i;
	//
		if (str[i] == '-') ++i;
		//
		if (str[i] == '0') {
			++i;
			if (in_range(str[i], '0', '9')) {
				err += "0后面跟其他数字不符合数字规范";
				return json11::default_null;
			}
		} else if (in_range(str[i], '1', '9')) {
			++i;
			while (in_range(str[i], '0', '9')) ++i;
		} else {
			// err += "必须要有一位数字" + FormatChar(str[i]);
			err += "必须要有一位数字";
			err.push_back(str[i]);
			has_fail = true;
			return json11::default_null;
		}
		//
		if (str[i] == '.') {
			++i;
			if (!in_range(str[i], '0', '9')) {
				err += "数字小数点后出错";
				return json11::default_null;
			}
			while (in_range(str[i], '0', '9')) ++i;
		}
		//
		if (str[i] == 'e' || str[i] == 'E') {
			++i;
			if (str[i] == '+' || str[i] == '-') ++i;
			if (!in_range(str[i], '0', '9')) {
				err += "指数负号后至少要有一个是数字";
				return json11::default_null;
			}
			while (in_range(str[i], '0', '9')) ++i;
		}
		double result = std::strtod(str.c_str() + start_pos, nullptr);
		return std::make_shared<NumberValue>(result);
	}

	std::string parse_string() {
		std::string out;
		long last_escaped_codepoint = -1;
		while (true) {
			if (i == str.length()) {
				return fail("字符串输入错误", "");
			}

			char ch = str[i++];
			// 
			if (ch == '"') {
				encode_utf8(last_escaped_codepoint, out);
				return out;
			}
			//
			if (in_range(ch, 0, 0x1F)) {
				return fail("字符串中出现意外字符", FormatChar(ch));
			}
			//
			if (ch != '\\') {
				encode_utf8(last_escaped_codepoint, out);
				last_escaped_codepoint = -1;
				out += ch;
				continue;
			}
			//
			else {
				if (i == str.length()) {
					return fail("转义字符出现错误", "");
				}
				ch = str[i++];
				//
				if (ch == 'u') {
					std::string esc = str.substr(i, 4);
					if (esc.size() < 4) {
						return fail(R"(\u后的字符串错误)", "");
					}
					for (size_t j = 0; j < 4; ++j) {
						if (!in_range(esc[j], 'a', 'f') && !in_range(esc[j], 'A', 'F') && !in_range(esc[j], '0', '9')) {
							return fail(R"(\u后的字符串错误)", "");
						}
					}

					// last_escaped_codepoint与codepoint（即上一次循环时的last_escaped_codepoint）用于处理unicode中代理对字符
					// unicode通常为16bit，而代理对字符需要32bit，即两个unicode共同使用
					// 举例：当遇到 \uD800\uDC00时，计算机应将其看作一个字符\uD800DC00
					// 不过这里的代码可以改进为：当遇到第一个符合代理对字符范围的字符时，紧接着判断下一个字符，不过这样做的效率不如当前代码
					long codepoint = strtol(esc.data(), nullptr, 16);

					if (in_range(last_escaped_codepoint, 0xD800, 0xDBFF) && in_range(codepoint, 0xDC00, 0xDFFF)) {
						encode_utf8(((last_escaped_codepoint - 0xD800) << 10) | (codepoint - 0xDC00) + 0x10000, out);
						last_escaped_codepoint = -1;
					} else {
						encode_utf8(last_escaped_codepoint, out);
						last_escaped_codepoint = codepoint;
					}

					i += 4;
					continue;
				}
				//
				else {
					encode_utf8(last_escaped_codepoint, out);
					last_escaped_codepoint = -1;

					if (ch == 'b') {
						out += '\b';
					} else if (ch == 'f') {
						out += '\f';
					} else if (ch == 'n') {
						out += '\n';
					} else if (ch == 'r') {
						out += '\r';
					} else if (ch == 't') {
						out += '\t';
					} else if (ch == '"' || ch == '\\' || ch == '/') {
						out += ch;
					} else {
						return fail("发现错误转义字符" + FormatChar(ch), "");
					}
				}
			}
		}
	}

	std::shared_ptr<JsonValue> parse_json(int depth) {
		if (depth > max_depth) return fail("层次过高");

		char ch = get_next_token();
		if (has_fail) return json11::default_null;

		if (ch == 'n') return expect("null", json11::default_null);
		else if (ch == 't') return expect("true", default_true);
		else if (ch == 'f') return expect("false", default_false);
		else if (ch == '-' || (ch >= '0' && ch <= '9')) {
			--i;
			return parse_number();
		}
		else if (ch == '"')  return std::make_shared<StringValue>(parse_string());
		else if (ch == '[') {
			json11::JsonArray data;
			ch = get_next_token();
			//
			if (ch == ']') {
				return std::make_shared<ArrayValue>(data);
			}
			while (true) {
				--i;
				data.push_back(parse_json(depth + 1));
				//
				if (has_fail) {
					return json11::default_null;
				}

				ch = get_next_token();
				if (ch == ']') break;
				if (ch != ',') {
					return  fail("数组数据缺乏\',\' " + FormatChar(ch));
				}
				ch = get_next_token();
			}
			return std::make_shared<ArrayValue>(data);
		}

		else if (ch == '{') {
			json11::JsonObject data;
			ch = get_next_token();
			if (ch == '}') {
				return std::make_shared<ObjectValue>(data);
			}
			while (true) {
				if (ch != '"') {
					return fail("对象类型缺少 '\"' " + FormatChar(ch));
				}
				std::string key = parse_string();
				if (has_fail) {
					return json11::default_null;
				}
				ch = get_next_token();
				if (ch != ':') {
					return fail("对象类型缺少 ';' " + FormatChar(ch));
				}
				data[std::move(key)] = parse_json(depth + 1);
				if (has_fail) {
					return json11::default_null;
				}
				ch = get_next_token();
				if (ch == '}') break;
				if (ch != ',') {
					return fail("对象类型缺少 ',' " + FormatChar(ch));
				}
				ch = get_next_token();
			}
			return std::make_shared<ObjectValue>(data);
		}

		return fail("出现未知错误" + FormatChar(ch));
	}

public:

	std::shared_ptr<JsonValue> parse() {
		std::shared_ptr<JsonValue> result = parse_json(0);
		consume_garbage();
		if (has_fail) return json11::default_null;
		if (i != str.length()) {
			return fail("unexpected trailing " + JsonParser::FormatChar(str[i]));
		}
		return result;
	}

	std::vector<std::shared_ptr<JsonValue>> parse_multi() {
		std::string::size_type parser_stop_pos = 0;
		std::vector<std::shared_ptr<JsonValue>> jsonvalue_vec;
		while (i != str.length() && !has_fail) {
			std::shared_ptr<JsonValue> jv_ptr = parse_json(0);
			jsonvalue_vec.push_back(jv_ptr);
			if (has_fail) break;
			
			consume_garbage();
			if (has_fail) break;

			parser_stop_pos = i;
		}
		return jsonvalue_vec;
	}
};

};