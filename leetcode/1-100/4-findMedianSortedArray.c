#define swap(x, y, t)   \
    do {  \
        t = x;  \
        x = y;  \
        y = t;  \
    } while(0)

double median(int* pl, int nl)
{
    double ml;
    if (!(nl%2)) {
        ml = (pl[nl/2-1]+pl[nl/2+1])/2.0;
    } else if (nl) {
        ml = pl[nl/2];
    } else {
        ml = 0; 
    }
    return ml;
}

int binarySearch(int t, int* p, int size)
{
    if (!p || size<=0) return -1;
    int l,r,piv;
    l = 0;
    r = size-1;
    piv = (l+r)/2;
    while (true) {
    }
}

double findMedianSortedArrays(int* nums1, int nums1Size, int* nums2, int nums2Size) {
    int* pl = nums1;
    int* pr = nums2;
    int nl = nums1Size;
    int nr = nums2Size;
    double ml, mr;
    if (!nl) {
        return median(pr, nr);
    } else if (!nr) {
        return median(pl, nl);
    }
    ml = median(pl, nl);
    mr = median(pr, nr);
    if (ml > mr) {
        int* pt;
        swap(pl, pr, pt);
        int nt;
        swap(nl, nr, nt);
        double mt;
        swap(ml, mr, mt);
    } else if (ml == mr) {
        if ((nl%2)==0) {
            return mr;
        } else if ((nr%2)==0) {
            return ml;
        }
    } else {
        // do nothing
    }
    
    // special case
    if (pl[nl-1]<=pr[0]) {
        if (!((nl+nr)%2)) {     // even
            int piv = (nl+nr)/2;
            int idx = piv-nl;
            if (idx>0) {
                return (pr[idx]+pr[idx-1])/2.0;
            } else if (idx){
                return (pl[piv]+pl[piv-1])/2.0;
            } else {
                return 1.0*pl[nl-1];
            }
        } else {        // odd
            int piv = (nl+nr)/2;
            int idx = piv-nl;
            if (idx>0) {
                return pr[idx];
            } else if (idx) {
                return pl[piv];
            } else {
                return 1.0*pr[0];
            }
        }
    }

    // general case - binary search
    if (!((nl+nr)%2)) { // even
        while (true) {
            
        }
    } else {    // odd
        
    }
}
