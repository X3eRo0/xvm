#include "memory.h"

int main(int argc, char* argv[]) {

	// bug if the very first section allocated is deleted then that creates a bug
	// the memory pointed by the head of maps gets deleted.

	m_map* mmap = allocate_map(NULL, (char*)".text", argv[0], 0x1000, 0x1337, 0x41414000, PROT_READ | PROT_EXEC, 0x00);
	allocate_map(mmap, (char*)".data", argv[0], 0x1000, 0x0337, 0x41415000, PROT_READ | PROT_WRITE, 0x00);
	allocate_map(mmap, (char*)"stack", argv[0], 0x4000, 0x0000, 0xf00db000, PROT_READ | PROT_WRITE, 0x00);
	vmmap(mmap);
	fini_m_map(mmap);
	return 0;
}