#pragma once

class OperateInterface
{
public:
    virtual int Operate() = 0;  
    virtual int Start() = 0;    
    virtual int Stop() = 0;
    virtual ~OperateInterface() = default;     
};
