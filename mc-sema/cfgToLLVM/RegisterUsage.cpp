/*
Copyright (c) 2015, Trail of Bits
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  Redistributions in binary form must reproduce the above copyright notice, this  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

  Neither the name of the {organization} nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include "llvm/Support/Debug.h"
#include "X86Subtarget.h"
#include "RegisterUsage.h"
#include "TransExcn.h"

using namespace llvm;

std::map<MCSemaRegs, RegInfo> REG_TO_OFFSET_MAP {
    {EAX, {0, "EAX"}},
    {EBX, {1, "EBX"}},
    {ECX, {2, "ECX"}},
    {EDX, {3, "EDX"}},
    {ESI, {4, "ESI"}},
    {EDI, {5, "EDI"}},
    {ESP, {6, "ESP"}},
    {EBP, {7, "EBP"}},
    {CF, {8, "CF"}},
    {PF, {9, "PF"}},
    {AF, {10, "AF"}},
    {ZF, {11, "ZF"}},
    {SF, {12, "SF"}},
    {OF, {13, "OF"}},
    {DF, {14, "DF"}},
    {ST0, {15, "STi"}}, // NOT A MISTAKE. These},
    {ST1, {15, "STi"}}, // are in a separate structure},
    {ST2, {15, "STi"}},
    {ST3, {15, "STi"}},
    {ST4, {15, "STi"}},
    {ST5, {15, "STi"}},
    {ST6, {15, "STi"}},
    {ST7, {15, "STi"}},
    {FPU_B, {16, "FPU_B"}},
    {FPU_C3, {17, "FPU_C3"}},
    {FPU_TOP, {18, "FPU_TOP"}},
    {FPU_C2, {19, "FPU_C2"}},
    {FPU_C1, {20, "FPU_C1"}},
    {FPU_C0, {21, "FPU_C0"}},
    {FPU_ES, {22, "FPU_ES"}},
    {FPU_SF, {23, "FPU_SF"}},
    {FPU_PE, {24, "FPU_PE"}},
    {FPU_UE, {25, "FPU_UE"}},
    {FPU_OE, {26, "FPU_OE"}},
    {FPU_ZE, {27, "FPU_ZE"}},
    {FPU_DE, {28, "FPU_DE"}},
    {FPU_IE, {29, "FPU_IE"}},
    {FPU_X, {30, "FPU_X"}},
    {FPU_RC, {31, "FPU_RC"}},
    {FPU_PC, {32, "FPU_PC"}},
    {FPU_PM, {33, "FPU_PM"}},
    {FPU_UM, {34, "FPU_UM"}},
    {FPU_OM, {35, "FPU_OM"}},
    {FPU_ZM, {36, "FPU_ZM"}},
    {FPU_DM, {37, "FPU_DM"}},
    {FPU_IM, {38, "FPU_IM"}},
    {FPU_TAG, {39, "FPU_TAG"}},
    {FPU_LASTIP_SEG, {40, "FPU_LASTIP_SEG"}},
    {FPU_LASTIP_OFF, {41, "FPU_LASTIP_OFF"}},
    {FPU_LASTDATA_SEG, {42, "FPU_LASTDATA_SEG"}},
    {FPU_LASTDATA_OFF, {43, "FPU_LASTDATA_OFF"}},
    {FPU_FOPCODE, {44, "FPU_FOPCODE"}},
    {XMM0, {45, "XMM0"}},
    {XMM1, {46, "XMM1"}},
    {XMM2, {47, "XMM2"}},
    {XMM3, {48, "XMM3"}},
    {XMM4, {49, "XMM4"}},
    {XMM5, {50, "XMM5"}},
    {XMM6, {51, "XMM6"}},
    {XMM7, {52, "XMM7"}},
    {STACK_BASE, {53, "STACK_BASE"}},
    {STACK_LIMIT, {54, "STACK_LIMIT"}}
};

StringRef getRegisterName(MCSemaRegs reg) {
    return REG_TO_OFFSET_MAP.at(reg).name; 
}

int getRegisterOffset(MCSemaRegs reg) {
    return REG_TO_OFFSET_MAP.at(reg).position;
}

Value *lookupLocal(Function *F, MCSemaRegs reg) {
    BasicBlock  *entry = &F->getEntryBlock();
    BasicBlock::iterator    it = entry->begin(); 

    std::string localName = std::string(getRegisterName(reg))+"_val";
    while(it != entry->end() ) {
        Value   *v = it;
        
        if( v->getName() == localName ) {
            return v;
        }

        ++it;
    }
    
    throw TErr (__LINE__, __FILE__, "localname: "+localName+" is not found");
    return nullptr;
}
