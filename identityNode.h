#ifndef IDENTITYNODE_H
#define IDNETITYNODE_H
#include "ICalculateElement.h"

//! модуль для определения узла
class IndentityNode:public ICalculateElement
{
public:
    IndentityNode(uint32_t idClass);
    virtual bool bind(){return true;}
    virtual void init();
    virtual void calculate(){}
    virtual void finite(){}
    std::string getNodeID(void);
private:
    //! список обнаруженных MAC адресов
    std::list<std::string> getListMac();
};

#endif // LAYERNODE_H
