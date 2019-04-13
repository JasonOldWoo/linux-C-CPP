#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
1 2 3 4 5 6 7 8 9

1 3 5 7 9
2 4 6 8

1   5   9   D
2 4 6 8 A C E
3   7   B   F

1     7     D
2   6 8   C E
3 5   9 B   F
4     A     10

1       9        11
2     8 A     10 12
3   7   B   F    13
4 6     C E      14
5       D        15
 */

/*
 * p = l * 2 - 2
 * D[0][n] = p * n
 * D[l][n] = (l - 1) + p * n
 * D[m][n]
 *      n is even:
 *      (m) + p * n/2
 *      n is odd:
 *      (m) + p * (n - 1)/2 + (l - m - 1) * 2
 *
 * special case l = 1
 *
 */

char* convert(char* s, int numRows) {
    if (!s || numRows <= 1) {
        return s;
    }

    int tmp;
    int numCols, p, m, n, t = 0, siz, k;
    siz = strlen(s);
    char* array = (char*) malloc(siz + 1);
    array[siz] = '\0';
    // peropd
    p = numRows * 2 - 2;
    // how many columns
    if (numRows >= 3) {
        numCols = 2 * (siz / p);
        tmp = (siz % p);
        if (tmp > numRows) {
            numCols += 2;
        } else if (tmp > 0) {
            numCols += 1;
        }
    } else if (numRows == 2) {
        numCols = siz / 2;
        if (siz % 2) numCols++;
    } else {
        return s;
    }

    //printf("size=%d rows=%d period=%d columns=%d\n",
    //        siz, numRows, p, numCols);

    for (n = 0; n < numCols; n++) {
        k = n * p;
        if (k >= siz) break;
        array[t++] = s[k];
    }

    for (m = 1; m < numRows - 1; m++) {
        for (n = 0; n < numCols; n++) {
            if (t >= siz) break;
            if (n % 2) {    // odd
                k = m + p * (n - 1) / 2 + (numRows - m - 1) * 2;
            } else {        // even
                k = m + p * (n / 2);
            }
            //printf("m=%d n=%d k=%d\n", m, n, k);
            if (k >= siz) continue;
            array[t++] = s[k];
        }
    }

    for (n = 0; n < numCols; n++) {
        k = (numRows - 1) + p * n;
        if (k >= siz) break;
        array[t++] = s[k];
    }
    return array;
}

int main()
{
    char s[100] = "PAYPALISHIRING";
    int numRows = 3;
    printf("result=%s\n", convert(s, numRows));
    bzero(s, sizeof(s));
    strncpy(s, "PAYPALISHIRING", sizeof(s));
    numRows = 4;
    printf("result=%s\n", convert(s, numRows));
    return 0;
}
