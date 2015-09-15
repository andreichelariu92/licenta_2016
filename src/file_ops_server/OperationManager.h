#ifndef OPERATIONMANAGER_H
#define OPERATIONMANAGER_H

#include<string>

class Operation;
class OperationManager
{
public:
    OperationManager(){}
    Operation* createOperation(char* buffer, unsigned int bufferSize);
};

#endif // OPERATIONMANAGER_H
