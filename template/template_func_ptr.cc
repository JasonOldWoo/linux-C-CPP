#include <map>
#include <iostream>

class CSend
{
public:
        void Send(int) { std::cout << "Do!\n"; }
};

template <typename _T_Key, typename _T_Item, typename _T_Compare = std::less<_T_Key> >
class CA
{
typedef  _T_Item _I;
typedef  _T_Key _K;
typedef  _T_Compare _C;
typedef typename std::map<_K, _I, _C> _Map;
public:
typedef void (CSend::*PSend) (_I);
typedef void (CSend::*SSend) (CA* pCA);

public:
        CA(CSend* pS, PSend pFS) :m_pS(pS), m_pFS(pFS){}
        CA() {}
        ~CA() {}

        void Set(SSend pFunc) { m_pSS = pFunc; }

        void Do() {(m_pS->*m_pFS)(m_Item);}

private:
        _Map m_mapPkt;
        _I m_Item;
        CSend* m_pS;
        PSend m_pFS;
        SSend m_pSS;
};

struct IDeriveSendSink
{
        virtual ~IDeriveSendSink() {}
        virtual void OnResultA(int) = 0;
        virtual void OnResultB(char*) = 0;
};

class CTool : public IDeriveSendSink
{
public:
        virtual void OnResultA(int) { std::cout << "result a\n"; }
        virtual void OnResultB(char*) { std::cout << "result b\n"; }
};

class CDeriveSend : public CSend
{
        typedef CA<int, int> CA_Sp;
public:
        CDeriveSend() { m_clsCA.Set((CA_Sp::SSend) &CDeriveSend::Send_A); }
        template <typename _Tp>
        void Send(_Tp*) { return; }
        void Send_A(CA_Sp* pS) { Send(pS); }

        void FuncA(int) {}
        void FuncB(char*) {}
        template <typename _Tp>
        struct FuncCls {
                typedef void (CDeriveSend::*FuncX)(_Tp);
        };
        void DoA(FuncCls<int>::FuncX pF, int x) { (this->*pF)(x); }
        void DoA_Wrapper(int x) { DoA((FuncCls<int>::FuncX) &CDeriveSend::FuncA, x); }
        template <typename _Tp>
        struct SinkFuncPtrWrapper {
                typedef void (IDeriveSendSink::*SinkFuncPtr) (_Tp);
        };
        template <typename _Tp>
        void DoCallBack(typename SinkFuncPtrWrapper<_Tp>::SinkFuncPtr pFunc)
        { _Tp val; (m_clsTool.*pFunc)(val); }
        void DoCallBack_Special_A()
        { DoCallBack<int>((SinkFuncPtrWrapper<int>::SinkFuncPtr) &IDeriveSendSink::OnResultA); }

private:
        CA_Sp m_clsCA;
        CTool m_clsTool;
};

int main()
{
        CSend clsS;
        CA<int, int> clsCa(&clsS, (CA<int, int>::PSend)&CSend::Send);
        clsCa.Do();
        CDeriveSend clsSend;
        clsSend.DoCallBack_Special_A();
        return 0;
}
