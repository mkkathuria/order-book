#pragma once
// ---------------------------------------------------------------------------
// Minimal nlohmann::json-compatible shim.
// Covers: parse, operator[], .contains(), .is_*(), .get<string/double>(),
//         iteration over arrays and objects.
//
// In a real build, replace this file with the official single_include header:
//   ./third_party/fetch_deps.sh
// ---------------------------------------------------------------------------
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <stdexcept>

namespace nlohmann {

class json {
public:
    enum Type { Null, Bool, Number, String, Array, Object };

    using array_t  = std::vector<json>;
    using object_t = std::unordered_map<std::string, json>;
    using value_t  = std::variant<std::nullptr_t, bool, double, std::string, array_t, object_t>;

    value_t data_;
    Type    type_ = Null;

    // --- constructors ---
    json()                     : data_(nullptr),         type_(Null)   {}
    json(std::nullptr_t)       : data_(nullptr),         type_(Null)   {}
    json(bool b)               : data_(b),               type_(Bool)   {}
    json(double d)             : data_(d),               type_(Number) {}
    json(int i)                : data_(static_cast<double>(i)), type_(Number) {}
    json(const std::string& s) : data_(s),               type_(String) {}
    json(const char* s)        : data_(std::string(s)),  type_(String) {}
    json(const array_t& a)     : data_(a),               type_(Array)  {}
    json(const object_t& o)    : data_(o),               type_(Object) {}

    // --- type checks ---
    bool is_null()         const { return type_ == Null;   }
    bool is_number_float() const { return type_ == Number; }
    bool is_string()       const { return type_ == String; }
    bool is_array()        const { return type_ == Array;  }
    bool is_object()       const { return type_ == Object; }

    // --- accessors ---
    template<typename T> T get() const {
        if constexpr (std::is_same_v<T, std::string>) return std::get<std::string>(data_);
        else if constexpr (std::is_same_v<T, double>) return std::get<double>(data_);
        else if constexpr (std::is_same_v<T, bool>)   return std::get<bool>(data_);
        else throw std::runtime_error("Unsupported get<T> type");
    }

    const json& operator[](const std::string& key) const {
        return std::get<object_t>(data_).at(key);
    }
    const json& operator[](size_t idx) const {
        return std::get<array_t>(data_).at(idx);
    }
    bool contains(const std::string& key) const {
        if (type_ != Object) return false;
        return std::get<object_t>(data_).count(key) > 0;
    }
    size_t size() const {
        if (type_ == Array)  return std::get<array_t>(data_).size();
        if (type_ == Object) return std::get<object_t>(data_).size();
        return 0;
    }

    const array_t&  array()  const { return std::get<array_t>(data_);  }
    const object_t& object() const { return std::get<object_t>(data_); }

    // --- static parse ---
    static json parse(const std::string& input);

private:
    struct Parser {
        const char* s;
        size_t len, pos;
        Parser(const std::string& in) : s(in.data()), len(in.size()), pos(0) {}

        void skip_ws() {
            while (pos < len && (s[pos]==' '||s[pos]=='\t'||s[pos]=='\n'||s[pos]=='\r')) ++pos;
        }
        char peek() {
            if (pos >= len) throw std::runtime_error("Unexpected end of JSON");
            return s[pos];
        }
        char next() { char c = peek(); ++pos; return c; }
        void expect(char c) { skip_ws(); if (next() != c) throw std::runtime_error(std::string("Expected ") + c); }

        json parse_value() {
            skip_ws();
            switch (peek()) {
                case '"': return parse_string();
                case '{': return parse_object();
                case '[': return parse_array();
                case 't': pos += 4; return json(true);
                case 'f': pos += 5; return json(false);
                case 'n': pos += 4; return json(nullptr);
                default:  return parse_number();
            }
        }

        json parse_string() {
            expect('"');
            std::string r;
            while (true) {
                char c = next();
                if (c == '"') break;
                if (c == '\\') {
                    char e = next();
                    switch (e) {
                        case '"': r += '"';  break;
                        case '\\': r += '\\'; break;
                        case '/': r += '/';  break;
                        case 'b': r += '\b'; break;
                        case 'f': r += '\f'; break;
                        case 'n': r += '\n'; break;
                        case 'r': r += '\r'; break;
                        case 't': r += '\t'; break;
                        case 'u': {
                            std::string hex(s + pos, 4);
                            pos += 4;
                            unsigned int cp = std::stoul(hex, nullptr, 16);
                            if (cp < 0x80)
                                r += static_cast<char>(cp);
                            else if (cp < 0x800) {
                                r += static_cast<char>(0xC0 | (cp >> 6));
                                r += static_cast<char>(0x80 | (cp & 0x3F));
                            } else {
                                r += static_cast<char>(0xE0 | (cp >> 12));
                                r += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                                r += static_cast<char>(0x80 | (cp & 0x3F));
                            }
                            break;
                        }
                        default: throw std::runtime_error("Bad JSON escape");
                    }
                } else {
                    r += c;
                }
            }
            return json(r);
        }

        json parse_number() {
            size_t start = pos;
            if (peek() == '-') ++pos;
            while (pos < len && s[pos] >= '0' && s[pos] <= '9') ++pos;
            if (pos < len && s[pos] == '.') {
                ++pos;
                while (pos < len && s[pos] >= '0' && s[pos] <= '9') ++pos;
            }
            if (pos < len && (s[pos] == 'e' || s[pos] == 'E')) {
                ++pos;
                if (pos < len && (s[pos] == '+' || s[pos] == '-')) ++pos;
                while (pos < len && s[pos] >= '0' && s[pos] <= '9') ++pos;
            }
            return json(std::stod(std::string(s + start, pos - start)));
        }

        json parse_array() {
            expect('[');
            array_t arr;
            skip_ws();
            if (peek() == ']') { ++pos; return json(arr); }
            while (true) {
                arr.push_back(parse_value());
                skip_ws();
                if (peek() == ']') { ++pos; break; }
                expect(',');
            }
            return json(arr);
        }

        json parse_object() {
            expect('{');
            object_t obj;
            skip_ws();
            if (peek() == '}') { ++pos; return json(obj); }
            while (true) {
                skip_ws();
                std::string k = parse_string().get<std::string>();
                expect(':');
                obj[k] = parse_value();
                skip_ws();
                if (peek() == '}') { ++pos; break; }
                expect(',');
            }
            return json(obj);
        }
    };
};

inline json json::parse(const std::string& input) {
    Parser p(input);
    return p.parse_value();
}

} // namespace nlohmann
