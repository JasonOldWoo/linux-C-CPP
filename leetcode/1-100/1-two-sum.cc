class Solution {
public:
    vector<int> twoSum(vector<int>& nums, int target) {
        list<int> lst(nums.begin(), nums.end());
        lst.sort();
        list<int>::iterator iter = lst.begin();
        list<int>::iterator r = lst.end();
        for (; iter != lst.end(); iter++)
        {
            r = iter; r++;
            for (; r != lst.end(); r++)
            {
                //cout << *r << " " << *iter << '\n';
                if (*r + *iter == target) break;
                else if (*r + *iter > target) {
                    r = lst.end(); break;
                }
            }
            if (r != lst.end()) break;
        }
        vector<int> vec(2); int j = 0;
        if (r != lst.end()) ;//cout << *iter << "," << *r << '\n';
        else return vec;
        for (size_t i = 0; i < nums.size(); i++)
        {
            if (*r == nums[i]) vec[j++] = i;
            else if (*iter == nums[i]) vec[j++] = i;
            //cout << *r << " " << *iter << " " << nums[i] << '\n';
            if (j > 1) break;
        }
        return vec;

    }
};
