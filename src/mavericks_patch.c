/*
 * Copyright (c) 2013, Null <foo.null@yahoo.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 // To compile:
 // clang mavericks_patch.c -framework ApplicationServices -dynamiclib -m32 -mmacosx-version-min=10.5 -o mavericks_patch.dylib

 #include <sys/mman.h>
 #include <unistd.h>
 #include <ApplicationServices/ApplicationServices.h>
 #include <mach/mach_init.h>
 #include <mach/mach_vm.h>
 #include <mach/task.h>
 #include <mach/mach_port.h>

 #define NOP_BYTE 0x90

 static void writeProtectedBuffer(void *buffer, size_t size, uintptr_t address)
 {
 	mach_vm_protect(mach_task_self(), address, size, 0, VM_PROT_ALL);
 	memcpy((void *)address, buffer, size);
 	mach_vm_protect(mach_task_self(), address, size, 0, PROT_READ | PROT_EXEC);
 }

static void nopInstructions(uintptr_t address, size_t size)
{
	void *nopBuffer = malloc(size);
	memset(nopBuffer, NOP_BYTE, size);
	writeProtectedBuffer(nopBuffer, size, address);
	free(nopBuffer);
}

static void changeGamespyAddress(uintptr_t pointer, const char *newAddress)
{
	strcpy((char *)pointer, newAddress);
}

static void patchGameVersionInstructions(char *gameVersionPatchBuffer, uintptr_t address, uintptr_t offset1, uintptr_t offset2)
{
	writeProtectedBuffer(gameVersionPatchBuffer, 2, address + offset1);
	writeProtectedBuffer(gameVersionPatchBuffer, 1, address + offset2);
}

static __attribute__((constructor)) void init()
{
	// Reserve memory halo wants before halo initiates, should help fix a bug in 10.9 where GPU drivers may have been loaded here
	mmap((void *)0x40000000, 0x1b40000, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_ANON | MAP_PRIVATE, -1, 0);

	// nop two jmps which Halo checks if system has AGL_ACCELERATED
	// this prevents Halo from running in certain display setups and it can do just fine if we skip these checks..
	nopInstructions(0x2B46CB, 2);
	nopInstructions(0x2B482E, 2);

	// Use the new HPC game browser since Gamespy died
	changeGamespyAddress(0x3867A6, "natneg1.hosthpc.com");
	changeGamespyAddress(0x386B2C, "s1.master.hosthpc.com");
	changeGamespyAddress(0x38BF5B, "s1.ms01.hosthpc.com");
	changeGamespyAddress(0x3867C0, "natneg2.hosthpc.com");

	// Change 01.00.09.0620 to 01.00.10.0621; affects hosting server
	// This is not stored in memory as a simple string, very convoluted/strange
	char gameVersionPatchBuffer[] = {0x31, 0x30};
	writeProtectedBuffer(gameVersionPatchBuffer, 2, 0x114da9);
	writeProtectedBuffer(gameVersionPatchBuffer, 1, 0x114dbc);

	// Change 4 locations that check against game version
	patchGameVersionInstructions(gameVersionPatchBuffer, 0x114C7C, 11, 24);
	patchGameVersionInstructions(gameVersionPatchBuffer, 0x114d97, 18, 37);
	patchGameVersionInstructions(gameVersionPatchBuffer, 0x114fef, 18, 37);
	patchGameVersionInstructions(gameVersionPatchBuffer, 0x237e17, 15, 28);

	// Change 1.08 string (01.00.08.0616) to 1.09 (01.00.09.0620)
	// The lobby will show both 1.10 and 1.09 servers
	char oldVersionString[] = "01.00.09.0620";
	writeProtectedBuffer(oldVersionString, sizeof(oldVersionString) - 1, 0x323B14);

	// Make sure we're frontmost app
	ProcessSerialNumber serialNumber;
	if (GetProcessForPID(getpid(), &serialNumber) == noErr)
	{
		SetFrontProcess(&serialNumber);
	}
}
