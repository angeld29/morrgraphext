#include "Breakpoint.h"

static void* Overwritten;
static Context context;

struct BreakpointData {
	DWORD addr;
	DWORD len;
	BYTE* overwritten;
	BreakpointFunc* func;
	bool active;

	BreakpointData(DWORD _addr, DWORD _len, const BYTE* _overwritten) {
		addr=_addr;
		len=_len;
		overwritten=new BYTE[len+7];
		CopyMemory(overwritten, _overwritten, len);
		overwritten[len+0]=0xff;	//Push eip
		overwritten[len+1]=0x35;
		*(DWORD*)&overwritten[len+2]=(DWORD)&context.Eip;
		overwritten[len+6]=0xc3;	//Return

		func=0;
		active=false;
	}
};

static const BYTE BP0[9]  = { 0x51, 0x53, 0x8b, 0xd9, 0xb9, 0x78, 0xb4, 0x7c, 0x00 };
static const BYTE BP1[8]  = { 0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf8, 0x6a, 0xff };
static const BYTE BP2[10] = { 0x33, 0xf6, 0x83, 0xc5, 0x02, 0x3d, 0x26, 0x01, 0x00, 0x00 };
static const BYTE BP3[6]  = { 0x85, 0xd2, 0x7d, 0x22, 0x8b, 0x54 };
static const BYTE BP4[10] = { 0x8b, 0xd0, 0x3b, 0xd5, 0x0f, 0x85, 0x52, 0x04, 0x00, 0x00 };
static const BYTE BP5[8]  = { 0x8b, 0x50, 0x14, 0xe9, 0x84, 0x05, 0x00, 0x00 };
static const BYTE BP6[7]  = { 0x8b, 0xd0, 0xe9, 0x6b, 0x05, 0x00, 0x00 };
static const BYTE BP7[10] = { 0x8b, 0x44, 0x24, 0x1c, 0x8b, 0x0d, 0xb4, 0xeb, 0x7c, 0x00 };
static const BYTE BP8[7]  = { 0xa0, 0xdc, 0xeb, 0x7c, 0x00, 0x84, 0xc0 };

static BreakpointData Breakpoints[] = {
	BreakpointData(0x4fdb00, 9, BP0),
	BreakpointData(0x5028a0, 8, BP1),
	BreakpointData(0x500540, 10, BP2),
	BreakpointData(0x50D5A8, 6, BP3),
	BreakpointData(0x4fe195, 10, BP4),
	BreakpointData(0x4fe065, 8, BP5),
	BreakpointData(0x4fe07f, 7, BP6),
	BreakpointData(0x509a94, 10, BP7),
	BreakpointData(0x507c54, 7, BP8)
};

static DWORD _stdcall InterruptExtern2(DWORD ID) {
	if(ID==3) {
		Overwritten = (void*)0x0050D5D6;
	} else {
		Overwritten = Breakpoints[ID].overwritten;

		if(ID==5) *(DWORD*)&Overwritten+=8;
		if(ID==6) *(DWORD*)&Overwritten+=7;
	}
	
	DWORD d=context.Eip;
	Breakpoints[ID].func(&context);
	if(context.Eip==d) {
		if(ID!=5&&ID!=6&&ID!=8) context.Eip+=Breakpoints[ID].len-8;
		return 0;
	} else return 1;
}

static __declspec(naked) void InterruptExtern() {
	_asm {
		//Save state
		mov context.Eax, eax
		pushfd
		pop eax
		mov context.flags, eax
		mov context.Ebx, ebx
		mov context.Ecx, ecx
		mov context.Edx, edx
		mov context.Edi, edi
		mov context.Esi, esi
		mov context.Ebp, ebp
		pop eax
		mov context.Eip, eax
		pop eax
		mov context.Esp, esp
		push eax

		//Carry out the interrupt
		call InterruptExtern2

		//restore state apart from the eax register and the flags
		mov ebx, context.Ebx
		mov ecx, context.Ecx
		mov edx, context.Edx
		mov edi, context.Edi
		mov esi, context.Esi
		mov ebp, context.Ebp
		mov esp, context.Esp

		//check result
		test eax, eax
		jz Continue

		//Jump to some other point in morrowind's code
		mov eax, context.Eax
		push context.flags
		popfd
		push context.Eip
		ret
Continue:
		//continue execution where it left off
		mov eax, context.Eax
		push context.flags
		popfd
		jmp Overwritten
	}
}

//This one conflicts with MWE, so move it backward a bit
static __declspec(naked) void Interrupt3() {
	_asm {
		//cmp edx, edx //Surely edx will _always_ be equal to edx??? Why does morrowind need to do this?
		//jge 0x0050D5CE
		cmp edx, 0x1bd
		jl 0x0050D5CE
		_emit 0x6a
		_emit 0x03
		call InterruptExtern
	}
}

//Not enough room to inject code for these, so move the injection site back a little and add the displaced code here
//Since one of the instructions was a relative jump, also need to replace it with an absolute jump here
static const DWORD InterruptEnd = 0x004fe5f1;
static __declspec(naked) void Interrupt5() {
	_asm {
		mov edx, [eax+0x14]
		_emit 0x6a
		_emit 0x05
		call InterruptExtern
		jmp InterruptEnd
	}
}

static __declspec(naked) void Interrupt6() {
	_asm {
		mov edx, eax
		_emit 0x6a
		_emit 0x06
		call InterruptExtern
		jmp InterruptEnd
	}
}

static const DWORD Interrupt8End = 0x507c5b;
static __declspec(naked) void Interrupt8() {
	_asm {
		_emit 0x6a
		_emit 0x08
		Call InterruptExtern
		jmp Interrupt8End
	}
}

static const DWORD InterruptExternAddr = (DWORD)&InterruptExtern;
static const DWORD Interrupt3Addr = (DWORD)&Interrupt3;
static const DWORD Interrupt5Addr = (DWORD)&Interrupt5;
static const DWORD Interrupt6Addr = (DWORD)&Interrupt6;
static const DWORD Interrupt8Addr = (DWORD)&Interrupt8;

void AddBreakpoint(BYTE ID, BreakpointFunc func) {
	if(ID>8) return;
	if(Breakpoints[ID].active) return;

	Breakpoints[ID].func=func;
	Breakpoints[ID].active=true;
	//DWORD OldProtect;

	//VirtualProtect((DWORD*)Breakpoints[ID].addr, Breakpoints[ID].len, PAGE_READWRITE, &OldProtect);
	BYTE* data = (BYTE*)Breakpoints[ID].addr;
	
	if(ID==3) {
		//MWE conflict, so needs a special case.
		//Move the injection site backward a bit.
		data[0]=0xff; //jmp Interrupt3
		data[1]=0x25;
		*(DWORD*)&data[2]=(DWORD)&Interrupt3Addr;
	} else if(ID==5||ID==6||ID==8) {
		//Not enough room at the injection site for these, so they need special treatment
		data[0]=0xff; //jmp Interrupt5/6/whatever
		data[1]=0x25;
		switch(ID) {
			case 5: *(DWORD*)&data[2]=(DWORD)&Interrupt5Addr; break;
			case 6: *(DWORD*)&data[2]=(DWORD)&Interrupt6Addr; break;
			case 8: *(DWORD*)&data[2]=(DWORD)&Interrupt8Addr; break;
		}
		for(DWORD i=6;i<Breakpoints[ID].len;i++) data[i]=0x90;
	} else {
		data[0]=0x6a; //push one immediate byte to stack
		data[1]=ID;
		data[2]=0xff; //call InterruptExtern
		data[3]=0x15;
		*(DWORD*)&data[4]=(DWORD)&InterruptExternAddr;
		for(DWORD i=8;i<Breakpoints[ID].len;i++) data[i]=0x90;
	}
	//VirtualProtect((DWORD*)Breakpoints[ID].addr, Breakpoints[ID].len, OldProtect, &OldProtect);
}

void RemoveBreakpoint(BYTE ID) {
	if(ID>8) return;
	if(!Breakpoints[ID].active) return;

	//DWORD OldProtect;
	const BYTE* source;
	switch(ID) {
		case 0: source=(BYTE*)&BP0; break;
		case 1: source=(BYTE*)&BP1; break;
		case 2: source=(BYTE*)&BP2; break;
		case 3: source=(BYTE*)&BP3; break;
		case 4: source=(BYTE*)&BP4; break;
		case 5: source=(BYTE*)&BP5; break;
		case 6: source=(BYTE*)&BP6; break;
		case 7: source=(BYTE*)&BP7; break;
		case 8: source=(BYTE*)&BP8; break;
	}
	//VirtualProtect((DWORD*)Breakpoints[ID].addr, Breakpoints[ID].len, PAGE_READWRITE, &OldProtect);
	CopyMemory((DWORD*)Breakpoints[ID].addr, source, Breakpoints[ID].len);
	//VirtualProtect((DWORD*)Breakpoints[ID].addr, Breakpoints[ID].len, OldProtect, &OldProtect);

	Breakpoints[ID].active=false;
}

void SetupPageProtection() {
	DWORD OldProtect;
	for(int ID=0;ID<9;ID++) {
		//Setup code protection here to avoid having to do it every time the breakpoints are set
		VirtualProtect((DWORD*)Breakpoints[ID].addr, Breakpoints[ID].len, PAGE_EXECUTE_READWRITE, &OldProtect);
		//Mark the overwritten buffer as executable to avoid DEP issues
		//Fails to initialize if PAGE_EXECUTE_READ is used?
		VirtualProtect((DWORD*)Breakpoints[ID].overwritten, Breakpoints[ID].len+7, PAGE_EXECUTE_READWRITE, &OldProtect);
	}
}