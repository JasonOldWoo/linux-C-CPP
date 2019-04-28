#include <stdio.h>
#include <string.h>
#include <limits.h>

int myAtoi(char* str)
{
    // 1 - remove white space
    char positive = 1;
    char* p = str;
    int len = strlen(str);
    unsigned int tmp = 0;
    const unsigned int iMax = 2147483647;
    const unsigned int iMin = 2147483648;
    for (; p != str + len; p++) {
        if (*p == ' ') continue ;
        else break;
    }
    if (p == str + len) return 0;

    // 2 - parse sign
    if (*p == '-') {
        positive = 0;
        p++;
    } else if (*p == '+') {
        positive = 1;
        p++;
    }
    if (p == str + len) return 0;

    // 3 - validate
    if (*p < '0' || *p > '9') {
        return 0;
    }

    int d = 0;
    while (*p >= '0' && *p <= '9' && (p != str+len)) {
        if (d >= 9) {
            printf("tmp=%d iMax/10=%d iMin/10=%d\n", tmp, iMax/10, iMin/10);
            if (positive && tmp > iMax/10) {
                printf("reach limits ~1 INT_MAX=%d\n", INT_MAX);
                return INT_MAX;
            } else if (positive && tmp == iMax/10 && (*p)-48 > 7) {
                printf("reach limits ~2 INT_MAX=%d\n", INT_MAX);
                return INT_MAX;
            } else if (positive && tmp == iMax/10) {
                return (int) (*p-48+tmp*10);
            } else if (!positive && tmp > iMin/10) {
                printf("reach limits ~1 INT_MIN=%d\n", INT_MIN);
                return INT_MIN;
            } else if (!positive && tmp == iMin/10 && (*p)-48 > 8) {
                printf("reach limits ~2 INT_MIN=%d\n", INT_MIN);
                return INT_MIN;
            } else if (!positive && tmp == iMin/10) {
                return (-*p+48-(int)tmp*10);
            }
        }
        tmp = (*p - 48) + tmp * 10;
        p++;
        d++;
    }
    if (!positive) return (0 - (int) tmp);
    return (int) tmp;
}

int main()
{
    printf("%d\n", myAtoi((char*)"-2147483648"));
}
