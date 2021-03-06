//InstCall.cpp

#include "INSTCALL.h"

INSTCALL::INSTCALL(VIRTUALMACHINE& vm) : machine(vm)
{
}

int INSTCALL::GetOperands(OPCODE opcode, VPVOID operanddata)
{
	int result= -1;
	
	if(machine.ReadMem(operanddata,&function,sizeof(function)))
		result= sizeof(function);
	else
		result= -1;
		
	return result;
}

bool INSTCALL::execute(void)
{
	VMREGTYPE ip= 0;
	bool result= machine.GetRegister(IP,ip)
		&& machine.push(ip)
		&& machine.SetRegister(IP,(VMREGTYPE)function);

	return result;
}

INSTCALLSHORT::INSTCALLSHORT(VIRTUALMACHINE& vm) : machine(vm)
{
}

int INSTCALLSHORT::GetOperands(OPCODE opcode, VPVOID operanddata)
{
	int result= -1;
	VMSHORT func= 0;
	
	if(machine.ReadMem(operanddata,&func,sizeof(func)))
	{
		result= sizeof(func);
		// 2005-06-29  CDC  kill the warning about pointer conversion
		function= (VPVOID)((int)func);
	}
	else
		result= -1;
		
	return result;
}

bool INSTCALLSHORT::execute(void)
{
	VMREGTYPE ip= 0;
	bool result= machine.GetRegister(IP,ip)
		&& machine.push(ip)
		&& machine.SetRegister(IP,(VMREGTYPE)function);

	return result;
}

INSTRETURN::INSTRETURN(VIRTUALMACHINE& vm) : machine(vm)
{
}

int INSTRETURN::GetOperands(OPCODE opcode, VPVOID operanddata)
{
	return 0;
}

bool INSTRETURN::execute(void)
{
	VMREGTYPE ip= 0;
	bool result= machine.pop(ip)
		&& machine.SetRegister(IP,ip);
		
	return result;
}

INSTRETURNP::INSTRETURNP(VIRTUALMACHINE& vm) : machine(vm)
{
}

int INSTRETURNP::GetOperands(OPCODE opcode, VPVOID operanddata)
{
	int result= -1;
	
	if(machine.ReadMem(operanddata,&paramsize,sizeof(paramsize)))
		result= sizeof(paramsize);
	else
		result= -1;
		
	return result;
}

bool INSTRETURNP::execute(void)
{
	bool result= false;
	
	VMREGTYPE ip= 0;
	VMREGTYPE sp= 0;
	if(machine.pop(ip) && machine.GetRegister(SP,sp))
	{
		sp+= paramsize;
		result= machine.SetRegister(SP,sp) && machine.SetRegister(IP,ip);
	}
	
	return result;
}

INSTRETURNVP::INSTRETURNVP(VIRTUALMACHINE& vm) : machine(vm)
{
}

int INSTRETURNVP::GetOperands(OPCODE opcode, VPVOID operanddata)
{
	int result= -1;
	
	if(machine.ReadMem(operanddata,&paramsize,sizeof(paramsize)))
		result= sizeof(paramsize);
	else
		result= -1;
		
	return result;
}

bool INSTRETURNVP::execute(void)
{
	bool result= false;
	
	VMREGTYPE funcresult= 0;
	VMREGTYPE ip= 0;
	VMREGTYPE sp= 0;
	if(machine.pop(funcresult) && machine.pop(ip) && machine.GetRegister(SP,sp))
	{
		sp+= paramsize;
		result= machine.SetRegister(SP,sp) && machine.push(funcresult) && machine.SetRegister(IP,ip);
	}
	
	return result;
}
