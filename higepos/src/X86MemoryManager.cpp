/*!
    \file  MemoryManager.cpp
    \brief class X86MemoryManager

    class X86MemoryManager

    Copyright (c) 2002 HigePon
    WITHOUT ANY WARRANTY

    \author  HigePon
    \version $Revision$
    \date   create:2002/08/04 update:$Date$
*/
#include<X86MemoryManager.h>


/*!
    \brief get class Name

    get class Name

    \author HigePon
    \date   create:2002/08/04 update:
*/
char* X86MemoryManager::getName() {

    static char buf[] = "X86MemoryManager";
    return buf;
}

/*!
    \brief allocate memory

    allocate memory

    \param size size of memory to allocate
    \return address to allocated memory

    \author HigePon
    \date   create:2002/08/07 update:
*/
unsigned long X86MemoryManager::allocateMemory(unsigned long size) {

    unsigned long oldAddress = current_;
    current_ += size;
    return oldAddress;
}

/*!
    \brief free memory

    free memory

    \param address adrress of memory to be free
    \return result

    \author HigePon
    \date   create:2002/08/07 update:
*/
unsigned long X86MemoryManager::freeMemory(unsigned long address) {

    return 0;
}

/*!
    \brief destructor

    destructor

    \author HigePon
    \date   create:2002/08/08 update:
*/
X86MemoryManager::~X86MemoryManager() {
}

/*!
    \brief private constructor

    constructor

    \author HigePon
    \date   create:2002/08/10 update:2002/08/24
*/
X86MemoryManager::X86MemoryManager():MEMORY_START(0x10000), MEMORY_SIZE(0x15000) {
    current_ = MEMORY_START;
}
