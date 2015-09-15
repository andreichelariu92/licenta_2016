#ifndef OPERATION_H
#define OPERATION_H

#include<iostream>
#include<string>
#include<sstream>

class Operation
{
public:
    Operation(){}
    void virtual exec()=0;
    std::string virtual getResult()=0;
    bool virtual requestStop()=0;
    virtual ~Operation(){}
};
class AddOperation : public Operation
{
public:
    AddOperation(int nr1, int nr2)
       :m_nr1(nr1), m_nr2(nr2), m_result(0), m_stringStream()
    {}
    void exec()
    {
       m_result=m_nr1+m_nr2;
    }
    std::string getResult()
    {
        m_stringStream<<m_result;
        m_stringStream<<"\r\n";
        std::string res=m_stringStream.str();
        return res;
    }
    bool requestStop()
    {
        return true;
    }
    ~AddOperation(){}
private:
    int m_nr1;
    int m_nr2;
    int m_result;
    std::ostringstream m_stringStream;
};

#endif // OPERATION_H
