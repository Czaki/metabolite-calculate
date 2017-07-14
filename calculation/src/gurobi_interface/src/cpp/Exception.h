// Copyright (C) 2016, Gurobi Optimization, Inc.
// All Rights Reserved
#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <exception>

class GRBException :public std::exception
{
  private:

    std::string msg;
    int error;

  public:

    GRBException(int errcode = 0);
    GRBException(std::string errmsg, int errcode = 0);

    const std::string getMessage() const;
    int getErrorCode() const;
    const char * what() const noexcept { return this->getMessage().c_str();}
};
#endif
