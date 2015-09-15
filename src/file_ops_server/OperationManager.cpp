#include"OperationManager.h"
#include"Operation.h"
#include<string>
Operation* OperationManager::createOperation(char* buffer, unsigned int bufferSize)
{
   Operation* retOperation=0;
   using std::string;

   //convert the buffer into a string
   string message(buffer,bufferSize);

   //find the delimeter (column)
   size_t delimeterPos=message.find_first_of(':');

   //if the delimeter is found
   if(delimeterPos != string::npos)
   {
       //create the operation
       string operationName=string(buffer,delimeterPos);

       if(operationName == "Add")
       {
          //move delimeterPos to the first
          //number (i.e. not space)
           delimeterPos=message.find_first_not_of(' ',delimeterPos+1);

           //if there are any numbers
           if(delimeterPos != string::npos)
           {
               //convert the first char to a number
               int a=message[delimeterPos] - '0';
               if(a>=0 && a<=9)
               {
                   //move delimeterPos to the next number
                   delimeterPos=delimeterPos+2;

                   //convert the second number
                   int b=message[delimeterPos] - '0';
                   if(b>=0 && b<=9)
                   {
                       //create the AddOperation
                       retOperation=new AddOperation(a,b);
                   }
               }
           }
       }
   }
   return retOperation;
}
