class Solution {
public:
    inline int bit(int x) {
        return 1 << x;
    }
    
    int lengthOfLongestSubstring(string s) {
        int szBits[4] = {0};
        int index = 0;
        size_t nMaxLen = 0;
        size_t pos = 0;
        for (size_t i = 0; i < s.length(); i++)
        {
            index = s.at(i)/32;
            if ((szBits[index] & bit(s.at(i)%32)))
            {
                if (s.at(i) != s.at(pos))
                {
                    for (size_t j = pos; j < s.length(); j++)
                    {
                        szBits[s.at(j)/32] &= ~(bit(s.at(j)%32));
                        if (s.at(j) == s.at(i))
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
            szBits[s.at(i)/32] |= bit(s.at(i)%32);
            size_t len = __builtin_popcount(szBits[0])
                + __builtin_popcount(szBits[1])
                + __builtin_popcount(szBits[2])
                + __builtin_popcount(szBits[3]);
            if (!len) pos = i;
            if (nMaxLen < len) nMaxLen = len;
            //if (setC.size() + s.length() - i - 1 <= nMaxLen) break;
        }
        return (int) nMaxLen;
    }
};
