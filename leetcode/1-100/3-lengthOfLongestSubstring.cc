// out of mind....
class Solution_Suck {
public:
    int lengthOfLongestSubstring(string s) {
        set<char> setC;
        size_t nMaxLen = 0;
        size_t pos = 0;
        for (size_t i = 0; i < s.length(); i++)
        {
            set<char>::iterator it = setC.find(s.c_str()[i]);
            if (it != setC.end())
            {
                if (*it != s.at(pos))
                {
                    // f**k it
                    for (size_t j = pos; j < s.length(); j++)
                    {
                        setC.erase(s.c_str()[j]);
                        if (s.c_str()[j] == s.c_str()[i])
                        {
                            pos = j+1;
                            break;
                        }
                    }
                }
                else
                {
                    pos++;
                }
            }
            if (setC.empty()) pos = i;
            setC.insert(s.c_str()[i]);
            if (nMaxLen < setC.size()) nMaxLen = setC.size();
            //if (setC.size() + s.length() - i - 1 <= nMaxLen) break;
        }
        return (int) nMaxLen;
    }
};
