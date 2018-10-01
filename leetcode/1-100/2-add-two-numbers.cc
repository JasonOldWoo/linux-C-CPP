/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode(int x) : val(x), next(NULL) {}
 * };
 */
class Solution {
public:
        ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
        ListNode* ln1 = l1; ListNode* ln2 = l2;
        ListNode* p = NULL; ListNode* pTmp = NULL;
        int cflag = 0;
        for (; ln2 != NULL || ln1 != NULL; )
        {
            int add = 0;
            if (ln1 && ln2)
            {
                add = ln1->val + ln2->val + cflag;
                ln1 = ln1->next;
                ln2 = ln2->next;
            }
            else if (ln1)
            {
                add = ln1->val + cflag;
                ln1 = ln1->next;
            }
            else if (ln2)
            {
                add = ln2->val + cflag;
                ln2 = ln2->next;
            }
            cflag = add / 10;
            if (cflag) add %= 10;
            ListNode* pNode = createNode(add);
            //cout << "add=" << pNode->val << '\n';
            if (p == NULL)
            {
                p = pNode;
                pTmp = p;
            }
            else
            {
                pTmp = addToList(pTmp, pNode);
            }
        }
        if (cflag)
        {
            ListNode* pNode = createNode(cflag);
            addToList(pTmp, pNode);
        }
        return p;
    }
    ListNode* createNode(int val)
    {
        ListNode* pNode = new ListNode(val);
        pNode->next = NULL;
        return pNode;
    }
    ListNode* addToList(ListNode* pTmp, ListNode* pNode)
    {
            pTmp->next = pNode;
            return pNode;
    }

};
