#include "JsonValue.h"
#include <cassert>
#include <iostream>

namespace json11 {
/* JsonParser 
 * 
 * ����ʵ�ֽ�string������JsonValue�Ĺ�����
 */
class JsonParser final {
private:
	const std::string& str; // ��ʾ��Ҫ������string
	size_t i; // ��ʾstr������ָ�룬��ʼʱΪ0
	std::string& err; // ���ڼ�¼string���������з����Ĵ���
	bool has_fail; // ��¼��ǰJsonParser�����ڽ����������Ƿ����˴��󣬳�ʼʱΪfalse
	const int max_depth; // JsonObject�е�JsonValue����Ƕ�׵��������Ƕ�ײ�ι��࣬�������Ҫ����ߣ�max_depth������������Ƕ�ײ��
public:
	JsonParser(const std::string& str_v, size_t i_v, std::string& err_v) 
		: str(str_v), i(i_v), err(err_v), has_fail(false), max_depth(200){}
	JsonParser(const std::string& str_v, std::string& err_v) 
		: JsonParser(str_v, 0, err_v){}
private:
	/* FormatChar()
	*
	* ��char����c��ʽ��Ϊstring�ĸ�ʽ���
	* ΪʲôҪ��ô����
	* �����������У���err�ĸ�����Ҫ��ͨ�� err += "������Ϣ" + char����ʾ�ģ�����char��Ϊ�������̳���ʱָ��i��ָ����ַ���
	* ����string������֧��������䣬�����ڴ���Ҫ��cahrת��Ϊstring����ʽ������������Ż���
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
	* �жϵ�һ������x�Ƿ��ڵڶ�������lower�͵�������upper֮��
	* �����򷵻�true�����򷵻�false
	*/
	inline bool in_range(long x, long lower, long upper) {
		return (x >= lower && x <= upper);
	}

	/* encode_utf8()
	*
	* ��pt����Ӧ��utf8������ӵ�outβ��
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
	 * ��ǵ�ǰJsonParserΪfail״̬��ͬʱ����err Ϊ��Ӧ�ı�����Ϣ
	 */
	template <typename T>
	T fail(std::string&& msg, const T err_ret) {
		if (!has_fail) {
			err += "������Ϣ��" + std::move(msg) + ", ����λ��: " + str.substr(i, 6);
		}
		has_fail = true;
		return err_ret;
	}

	std::shared_ptr<JsonValue> fail(std::string&& msg) {
		return fail(std::move(msg), json11::default_null);
	}

	/* consume_whitespace()
	 *
	 * �����ַ����Ŀհײ���
	 */
	void consume_whitespace() {
		while (str[i] == ' ' || str[i] == '\r' || str[i] == '\n' || str[i] == '\t')
			++i;
	}

	/* consume_comment()
	 *
	 * ȷ�ϵ�ǰ�ַ����Ƿ�Ϊע��
	 * �����ע�ͣ���������������true�����򷵻�false��������err
	 */
	bool consume_comment() {
		bool comment_found = false;
		if (str[i] == '/') {
			++i;
			// ֻ��һ��'/'����
			if (i == str.size())
				return fail(R"(ֻ��һ��/���޷��ж��Ƿ�Ϊע��)", false);
			// ����ע��
			if (str[i] == '/') {
				++i;
				while (i < str.size() && str[i] != '\n') ++i;
				comment_found = true;
			}
			// ����ע��
			else if (str[i] == '*') {
				++i;
				// ���ڶ���ע��һ��Ϊ "/* ... */"�ĸ�ʽ
				// ����ע�ͳ�������Ϊ4
				// ����ǰ��������'*'���±� + 2  <= str.size() 
				if (i > str.size() - 2)
					return fail("����ע�ʹ��ڴ���", false);
				// һֱ������ֱ���ҵ�"*/"Ϊֹ
				while (!(str[i] == '*' && str[i + 1] == '/')) {
					++i;
					if (i > str.size() - 2)
						return fail("����ע�ʹ��ڴ���", false);
				}
				i += 2;
				comment_found = true;
			}
			// �ȷǵ���ע�ͣ��ַǶ���ע��
			else {
				return fail(R"(ע�ʹ��ڴ���)", false);
			}
		}
		return comment_found;
	}

	/* consume_garbage()
	 *
	 * ����ע�ͺͿհ�
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
	 * ���ص�һ����Ч�ַ�������ע����հף�
	 * ���Զ�����һλ
	 */
	char get_next_token() {
		consume_garbage();
		if (has_fail) return static_cast<char>(0);
		if (i == str.size())
			return fail("stringĩβ��������json����", static_cast<char>(0));
		return str[i++];
	}

	/* expect()
	 *
	 * ��str������i��ʼ��������������Ϊexpected�ĳ��ȣ��ж���һ�����Ƿ���expected��ͬ
	 * ��ͬ�򷵻�res����ͬ�򷵻�fail����
	 * ��Ҫ���ڼ���null��true��false�����
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
				err += "0������������ֲ��������ֹ淶";
				return json11::default_null;
			}
		} else if (in_range(str[i], '1', '9')) {
			++i;
			while (in_range(str[i], '0', '9')) ++i;
		} else {
			// err += "����Ҫ��һλ����" + FormatChar(str[i]);
			err += "����Ҫ��һλ����";
			err.push_back(str[i]);
			has_fail = true;
			return json11::default_null;
		}
		//
		if (str[i] == '.') {
			++i;
			if (!in_range(str[i], '0', '9')) {
				err += "����С��������";
				return json11::default_null;
			}
			while (in_range(str[i], '0', '9')) ++i;
		}
		//
		if (str[i] == 'e' || str[i] == 'E') {
			++i;
			if (str[i] == '+' || str[i] == '-') ++i;
			if (!in_range(str[i], '0', '9')) {
				err += "ָ�����ź�����Ҫ��һ��������";
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
				return fail("�ַ����������", "");
			}

			char ch = str[i++];
			// 
			if (ch == '"') {
				encode_utf8(last_escaped_codepoint, out);
				return out;
			}
			//
			if (in_range(ch, 0, 0x1F)) {
				return fail("�ַ����г��������ַ�", FormatChar(ch));
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
					return fail("ת���ַ����ִ���", "");
				}
				ch = str[i++];
				//
				if (ch == 'u') {
					std::string esc = str.substr(i, 4);
					if (esc.size() < 4) {
						return fail(R"(\u����ַ�������)", "");
					}
					for (size_t j = 0; j < 4; ++j) {
						if (!in_range(esc[j], 'a', 'f') && !in_range(esc[j], 'A', 'F') && !in_range(esc[j], '0', '9')) {
							return fail(R"(\u����ַ�������)", "");
						}
					}

					// last_escaped_codepoint��codepoint������һ��ѭ��ʱ��last_escaped_codepoint�����ڴ���unicode�д�����ַ�
					// unicodeͨ��Ϊ16bit����������ַ���Ҫ32bit��������unicode��ͬʹ��
					// ������������ \uD800\uDC00ʱ�������Ӧ���俴��һ���ַ�\uD800DC00
					// ��������Ĵ�����ԸĽ�Ϊ����������һ�����ϴ�����ַ���Χ���ַ�ʱ���������ж���һ���ַ���������������Ч�ʲ��統ǰ����
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
						return fail("���ִ���ת���ַ�" + FormatChar(ch), "");
					}
				}
			}
		}
	}

	std::shared_ptr<JsonValue> parse_json(int depth) {
		if (depth > max_depth) return fail("��ι���");

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
					return  fail("��������ȱ��\',\' " + FormatChar(ch));
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
					return fail("��������ȱ�� '\"' " + FormatChar(ch));
				}
				std::string key = parse_string();
				if (has_fail) {
					return json11::default_null;
				}
				ch = get_next_token();
				if (ch != ':') {
					return fail("��������ȱ�� ';' " + FormatChar(ch));
				}
				data[std::move(key)] = parse_json(depth + 1);
				if (has_fail) {
					return json11::default_null;
				}
				ch = get_next_token();
				if (ch == '}') break;
				if (ch != ',') {
					return fail("��������ȱ�� ',' " + FormatChar(ch));
				}
				ch = get_next_token();
			}
			return std::make_shared<ObjectValue>(data);
		}

		return fail("����δ֪����" + FormatChar(ch));
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