#define bit(x) (1u<<(x))
int lengthOfLongestSubstring(char* s) {
        unsigned int szBits[4]={0};
        int index=0, numbits=0;
        size_t nMaxLen=0,len=0;
        size_t pos=0;
        size_t i,j;
        size_t strLen=strlen(s);
        const char* cs=s;
        for (i=0; i<strLen; i++)
        {
            index=cs[i]/32;
            numbits=bit(cs[i]%32);
            if ((szBits[index]&numbits))
            {
                if (cs[i]!=cs[pos])
                {
                    for (j=pos; j<strLen; j++)
                    {
                        szBits[cs[j]/32]&= ~(bit(cs[j]%32));
                        if (cs[j] == cs[i])
                        {
                            len -= (j-pos+1);
                            pos=j+1;
                            break;
                        }
                    }
                }
                else
                {
                    pos++;
                }
            }
            if (!(szBits[index]&numbits)) len++;
            szBits[index] |= numbits;
            if (!len) pos=i;
            if (nMaxLen<len) nMaxLen=len;
        }
        return (int)nMaxLen;
}
