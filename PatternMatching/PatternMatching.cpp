#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;
const string USAGE = "Arguments\n\n-f file                            | file to match in\n-c file                            | load pattern config file\n-p matchpattern:replacepattern     | match and replace pattern\n-o                                 | (Optional)Match Patterns with overlap\n\nExample Usage:\nPatternMatching.exe -f owo.txt -c config.txt\nPatternMatching.exe -f owo.txt -p \"01 02 03:02 03 04\"";

void fail(string reason="")
{
    if (reason == "") cout << USAGE << endl;
    else cout << reason << endl << USAGE << endl;
    exit(0);
}

bool file_exists(const std::string& name) 
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

vector<char> HexToBytes(const std::string& hex) {
    std::vector<char> bytes;

    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        char byte = (char)strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(byte);
    }

    return bytes;
}

void removeWhitespace(string& string)
{
    string.erase(std::remove_if(string.begin(), string.end(), std::isspace), string.end());
}

pair<string, string> processPattern(string str)
{
    removeWhitespace(str);
    for (long i = 0; i < str.size(); i++) 
    {
        if (str[i] != ':') continue;
        return pair<string, string>{str.substr(0, i), str.substr(i + 1, str.length() - i + 1)};
    }

    return pair<string, string>{};
}

string readfile(std::string file) {
    int length{};
    char* array{};
    std::string s;

    std::ifstream stream;
    stream.open(file, std::ios_base::binary);
    if (stream.good()) {
        length = stream.rdbuf()->pubseekoff(0, std::ios_base::end);
        array = new char[length];
        stream.rdbuf()->pubseekoff(0, std::ios_base::beg);
        stream.read(array, length);
        stream.close();
    }

    char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    for (int i = 0; i < length; ++i)
    {
        s += hex_chars[(array[i] & 0xF0) >> 4];
        s += hex_chars[(array[i] & 0x0F) >> 0];
    }

    return s;
}

bool is_hex_notation(std::string const& s)
{
    return s.size() > 2 && s.find_first_not_of("0123456789abcdefABCDEF", 0) == std::string::npos;
}

bool is_hex_notation_w(std::string const& s)
{
    return s.size() > 2 && s.find_first_not_of("0123456789abcdefABCDEF?", 0) == std::string::npos;
}

vector<size_t> bmh(const string& text, const string& pattern)
{
    size_t m = pattern.size();
    size_t n = text.size();
    size_t i;
    size_t bad[16]{}; // 16 = hex alphabet

    for (size_t i = 0; i < 16; ++i)
        bad[i] = m;

    size_t lastPatternByte = m - 1;

    for (i = 0; i < lastPatternByte; ++i)
        bad[pattern[i]] = lastPatternByte - i;

    // Beginning

    size_t index = 0;
    vector<size_t> ret;

    while (index <= (n - m))
    {
        for (i = lastPatternByte; text[(index + i)] == pattern[i]; --i)
        {
            if (i == 0)
            {
                ret.push_back(index);
                break;
            }
        }

        index += bad[text[(index + lastPatternByte)]];
    }
    if (ret.empty()) return vector<size_t>(0);
    return ret;
}

vector<size_t> bmh_w_fast(const std::string& text, const std::string& pattern, char wildcard='?') {
    // Does not account for overlaps
    int m = pattern.length();
    int n = text.length();

    vector<int> right(16, -1); // 16 = hex alphabet
    for (int j = 0; j < m; j++) {
        if (pattern[j] != wildcard) {
            int a = (pattern[j] >= '0' && pattern[j] <= '9') ? (pattern[j] - '0') : (pattern[j] - 'A' + 10);
            right[a] = j;
        }
    }

    vector<size_t> positions;
    int skip;
    for (int i = 0; i <= n - m; i += skip) {
        skip = 0;
        for (int j = m - 1; j >= 0; j--) {
            if (pattern[j] != wildcard && pattern[j] != text[i + j]) {
                int a = (text[i + j] >= '0' && text[i + j] <= '9') ? (text[i + j] - '0') : (text[i + j] - 'A' + 10);
                skip = max(1, j - right[a]);
                break;
            }
        }
        if (skip == 0) {
            positions.push_back(i);
            skip = 1;
        }
    }
    if (positions.empty()) return vector<size_t>(0);
    return positions;
}

bool IsMatch(const string& array, int& position, const string& pattern, char wildcard='?')
{
    if (pattern.size() > (array.size() - position))
        return false;
    for (int i = 0; i < pattern.size(); i++)
        if (array[position + i] != pattern[i] && pattern[i] != wildcard)
            return false;
    return true;
}

vector<size_t> naivematch(const std::string& text, const std::string& pattern, char wildcard='?')
{
    vector<size_t> res;
    int m = text.size();
    for (int i = 0; i < m; i++) {
        if (!IsMatch(text, i, pattern)) continue;
        res.push_back(i);
        //if (i + pattern.size() > m) i += pattern.size();
    }
    if (res.empty()) return vector<size_t>(0);
    return res;
}

int main(short argc, char **argv)
{
    if (argc < 5 || argc > 6) fail("Argument Error");
    short c = 0; short p = 0; short f = 0; short o = 0;
    for (short i = 0; i < argc; i++)
    {
        if ((string)argv[i] == "-c") c = i;
        else if ((string)argv[i] == "-p") p = i;
        else if ((string)argv[i] == "-f") f = i;
        else if ((string)argv[i] == "-no") o = i;
    }
    vector<pair<string, string>> patterns;
    if (f == 0 || !file_exists(argv[f + 1])) fail("No Source File");
    if (p != 0) patterns.push_back(processPattern(argv[p + 1]));
    else if (c != 0)
    {
        if (!file_exists(argv[c + 1])) fail("Config file does not exist");
        ifstream infile(argv[c + 1]);
        for (string str; getline(infile, str);)
        {
            removeWhitespace(str);
            if (str[0] == '#') continue;
            patterns.push_back(processPattern(str));
        }
    }
    else fail("Argument Error");
    if (patterns.size() <= 0) fail("Pattern Parsing Error");

    const string hexdata = readfile(argv[f + 1]);
    vector<pair<string, vector<size_t>>> matches;
    for (pair<string, string> pattern : patterns) 
    {
        int size = matches.size();
        if (pattern.first.size() != pattern.second.size()) fail("Patterns not equal in size");
        if (!is_hex_notation(pattern.second) && !is_hex_notation_w(pattern.second)) fail("Replace Pattern Fail");
        if (is_hex_notation(pattern.first)) 
            matches.push_back(pair<string, vector<size_t>>{pattern.second, bmh(hexdata, pattern.first)});
        else if (is_hex_notation_w(pattern.first))
            if (o == 0)  matches.push_back(pair<string, vector<size_t>>{pattern.second, bmh_w_fast(hexdata, pattern.first)});
            else matches.push_back(pair<string, vector<size_t>>{pattern.second, naivematch(hexdata, pattern.first)});
        else fail("Match Pattern Fail");
        if (matches.size() == size) cout << "No matches for pattern " << pattern.first << endl;
    }
    
    auto is_empty = [](pair<string, vector<size_t>> &m)
    {
        if (m.second.empty()) return true;
        return false;
    };

    matches.erase(remove_if(begin(matches), end(matches), is_empty), end(matches));

    if (matches.size() <= 0) { cout << "No matches found" << endl; return 0; }

    ofstream fout;
    fout.open(argv[f + 1], ios::binary|ios::out|ios::in);

    for (pair<string, vector<size_t>> match : matches) 
    {
        vector<char> t;
        bool flag = false;
        if (is_hex_notation(match.first))
            t = HexToBytes(match.first);
        else flag = true;
        for (size_t pos : match.second) 
        {
            fout.seekp(pos/2);
            if (flag)
            {
                string nmatch = match.first;
                for (long i = 0; i < match.first.size(); i++)
                {
                    if (nmatch[i] != '?') continue;
                    nmatch[i] = hexdata[pos + i];
                }
                t = HexToBytes(nmatch);
            }
            copy(t.begin(), t.end(), ostreambuf_iterator<char>(fout));
            fout.flush();
        }
    }

    fout.close();

    return 0;
}
