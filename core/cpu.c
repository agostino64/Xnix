/*
 * Copyright (c) 2006-2007 -  http://brynet.biz.tm - <brynet@gmail.com>
 * Copyright (c) 2024 Agustin Gutierrez
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <xnix/common.h>
#include <xnix/vga.h>
#include <xnix/cpu.h>

#define cpuid(in, a, b, c, d) __asm__("cpuid": "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in));

/* Simply call this function detect_cpu(); */
int detect_cpu(void) /* or main() if your trying to port this as an independant application */
{
	unsigned long ebx, unused;
	cpuid(0, unused, ebx, unused, unused);
	switch(ebx)
	{
		case 0x756e6547: /* Intel Magic Code */
		do_intel();
		break;
		case 0x68747541: /* AMD Magic Code */
		do_amd();
		break;
		default:
		printk("Unknown x86 CPU Detected\n");
		break;
	}
	return 0;
}

/* Intel Specific brand list */
char *Intel[] = {
	"Brand ID Not Supported.", 
	"Intel(R) Celeron(R) processor", 
	"Intel(R) Pentium(R) III processor", 
	"Intel(R) Pentium(R) III Xeon(R) processor", 
	"Intel(R) Pentium(R) III processor", 
	"Reserved", 
	"Mobile Intel(R) Pentium(R) III processor-M", 
	"Mobile Intel(R) Celeron(R) processor", 
	"Intel(R) Pentium(R) 4 processor", 
	"Intel(R) Pentium(R) 4 processor", 
	"Intel(R) Celeron(R) processor", 
	"Intel(R) Xeon(R) Processor", 
	"Intel(R) Xeon(R) processor MP", 
	"Reserved", 
	"Mobile Intel(R) Pentium(R) 4 processor-M", 
	"Mobile Intel(R) Pentium(R) Celeron(R) processor", 
	"Reserved", 
	"Mobile Genuine Intel(R) processor", 
	"Intel(R) Celeron(R) M processor", 
	"Mobile Intel(R) Celeron(R) processor", 
	"Intel(R) Celeron(R) processor", 
	"Mobile Geniune Intel(R) processor", 
	"Intel(R) Pentium(R) M processor", 
	"Mobile Intel(R) Celeron(R) processor"
};

/* This table is for those brand strings that have two values depending on the processor signature. It should have the same number of entries as the above table. */
char *Intel_Other[] = {
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Intel(R) Celeron(R) processor", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Intel(R) Xeon(R) processor MP", 
	"Reserved", 
	"Reserved", 
	"Intel(R) Xeon(R) processor", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved"
};

/* Intel-specific information */
int do_intel(void)
{
	unsigned long eax, ebx, ecx, edx, max_eax, signature, unused;
	int model, family, type, brand, stepping, reserved;
	cpuid(0, eax, ebx, ecx, edx);
	printk("Vendor: %c%c%c%c%c%c%c%c%c%c%c%c\n", 
         (char)(ebx), (char)(ebx >> 8), (char)(ebx >> 16), (char)(ebx >> 24),
         (char)(edx), (char)(edx >> 8), (char)(edx >> 16), (char)(edx >> 24),
         (char)(ecx), (char)(ecx >> 8), (char)(ecx >> 16), (char)(ecx >> 24));
	int extended_family = -1;
	cpuid(1, eax, ebx, unused, unused);
	model = (eax >> 4) & 0xf;
	family = (eax >> 8) & 0xf;
	type = (eax >> 12) & 0x3;
	brand = ebx & 0xff;
	stepping = eax & 0xf;
	reserved = eax >> 14;
	signature = eax;
	printk("Type: ");
	switch(type)
	{
		case 0:
		printk("Original OEM");
		break;
		case 1:
		printk("Overdrive");
		break;
		case 2:
		printk("Dual-capable");
		break;
		case 3:
		printk("Reserved");
		break;
	}
	printk("\n");
	printk("Family: ");
	switch(family)
	{
		case 3:
		printk("i386");
		break;
		case 4:
		printk("i486");
		break;
		case 5:
		printk("Pentium");
		break;
		case 6:
		printk("Pentium Pro");
		break;
		case 15:
		printk("Pentium 4");
	}
	printk("\n");
	if(family == 15)
	{
		extended_family = (eax >> 20) & 0xff;
		printk("Extended family %d\n", extended_family);
	}
	printk("Model: ");
	switch(family)
	{
		case 3:
		break;
		case 4:
		switch(model)
		{
			case 0:
			case 1:
			printk("DX");
			break;
			case 2:
			printk("SX");
			break;
			case 3:
			printk("487/DX2");
			break;
			case 4:
			printk("SL");
			break;
			case 5:
			printk("SX2");
			break;
			case 7:
			printk("Write-back enhanced DX2");
			break;
			case 8:
			printk("DX4");
			break;
		}
		break;
		case 5:
		switch(model)
		{
			case 1:
			printk("60/66");
			break;
			case 2:
			printk("75-200");
			break;
			case 3:
			printk("for 486 system");
			break;
			case 4:
			printk("MMX");
			break;
		}
		break;
		case 6:
		switch(model)
		{
			case 1:
			printk("Pentium Pro");
			break;
			case 3:
			printk("Pentium II Model 3");
			break;
			case 5:
			printk("Pentium II Model 5/Xeon/Celeron");
			break;
			case 6:
			printk("Celeron");
			break;
			case 7:
			printk("Pentium III/Pentium III Xeon - external L2 cache");
			break;
			case 8:
			printk("Pentium III/Pentium III Xeon - internal L2 cache");
			break;
		}
		break;
		case 15:
		break;
	}
	printk("\n");
	cpuid(0x80000000, max_eax, unused, unused, unused);
	/* Quok said: If the max extended eax value is high enough to support the processor brand string
	(values 0x80000002 to 0x80000004), then we'll use that information to return the brand information. 
	Otherwise, we'll refer back to the brand tables above for backwards compatibility with older processors. 
	According to the Sept. 2006 Intel Arch Software Developer's Guide, if extended eax values are supported, 
	then all 3 values for the processor brand string are supported, but we'll test just to make sure and be safe. */
	if(max_eax >= 0x80000004)
	{
		printk("Brand: ");
		if(max_eax >= 0x80000002) 
		{
			cpuid(0x80000002, eax, ebx, ecx, edx);
			printregs(eax, ebx, ecx, edx);
		}
		if(max_eax >= 0x80000003)
		{
			cpuid(0x80000003, eax, ebx, ecx, edx);
			printregs(eax, ebx, ecx, edx);
		}
		if(max_eax >= 0x80000004)
		{
			cpuid(0x80000004, eax, ebx, ecx, edx);
			printregs(eax, ebx, ecx, edx);
		}
		printk("\n");
	}
	else if(brand > 0)
	{
		printk("Brand %d - ", brand);
		if(brand < 0x18)
		{
			if(signature == 0x000006B1 || signature == 0x00000F13)
				printk("%s\n", Intel_Other[brand]);
			else
				printk("%s\n", Intel[brand]);
		}
		else
			printk("Reserved\n");
	}
	printk("Stepping: %d\n", stepping);
	printk("Reserved: %d\n", reserved);
	return 0;
}

/* Print Registers */
void printregs(int eax, int ebx, int ecx, int edx)
{
	int j;
	char string[17];
	string[16] = '\0';
	for(j = 0; j < 4; j++)
	{
		string[j] = eax >> (8 * j);
		string[j + 4] = ebx >> (8 * j);
		string[j + 8] = ecx >> (8 * j);
		string[j + 12] = edx >> (8 * j);
	}
	printk("%s", string);
}

/* AMD-specific information */
int do_amd(void)
{
	unsigned long extended, eax, ebx, ecx, edx, unused;
	cpuid(0, eax, ebx, ecx, edx);
	printk("Vendor: %c%c%c%c%c%c%c%c%c%c%c%c\n",
         (char)(ebx), (char)(ebx >> 8), (char)(ebx >> 16), (char)(ebx >> 24),
         (char)(edx), (char)(edx >> 8), (char)(edx >> 16), (char)(edx >> 24),
         (char)(ecx), (char)(ecx >> 8), (char)(ecx >> 16), (char)(ecx >> 24));
	int family, model, stepping, reserved;
	cpuid(1, eax, unused, unused, unused);
	model = (eax >> 4) & 0xf;
	family = (eax >> 8) & 0xf;
	stepping = eax & 0xf;
	reserved = eax >> 12;
	printk("Family: ");
	switch(family)
	{
	    case 5:
		printk("AMD K6");
		break;
	    case 6:
	    	printk("AMD K7");
		break;
	    case 15:
	    	printk("K8 / Hammer");
		break;
	    case 16:
	    	printk("K10");
		break;
	    case 17:
	    	printk("AMD K8 & K10 \"hybrid\"");
		break;
	    case 18:
	    	printk("AMD K10 (Llano) / K12 (ARM-based)");
		break;
	    case 10:
	    	printk("AMD Bobcat");
		break;
	    case 21:
	    	printk("AMD Bulldozer / Piledriver / Steamroller / Excavator");
		break;
	    case 22:
	    	printk("AMD Jaguar / Puma");
		break;
	    case 23:
	    	printk("AMD Zen / Zen+ / Zen 2");
		break;
	    case 24:
	    	printk("AMD Hygon Dhyana");
		break;
	    case 25:
	    	printk("AMD Zen 3 / Zen 3+ / Zen 4");
		break;
	    case 26:
	    	printk("AMD Zen 5");
		break;
	    default:
		printk("Unknown Family (%d)", family);
		break;
	}
	printk("\nModel: ");
	switch(model)
	{
	    case 1:
	    case 22:
	    case 23:
		printk("Opteron");
		break;
	    case 2:
		printk("Athlon 64 FX");
		break;
	    case 3:
	    case 10:
	    case 19:
		printk("Athlon 64");
		break;
	    case 4:
	    	printk("Mobile Athlon 64");
		break;
	    case 5:
	    	printk("Athlon XP-M");
		break;
	    case 6:
	    case 20:
	    case 21:
	    	printk("Sempron");
		break;
	    case 7:
	    	printk("Athlon 64");
		break;
	    case 8:
	    	printk("Turion 64");
		break;
	    case 9:
	    	printk("Athlon 64 FX");
		break;
	    case 11:
	    	printk("Sempron");
		break;
	    case 12:
	    case 13:
	    case 15:
	    case 16:
	    case 18:
	    	printk("Athlon 64 X2");
		break;
	    case 17:
	    	printk("Athlon 64 FX");
		break;
	    case 24:
	    	printk("Mobile Sempron");
	    	break;
	    default:
		printk("Unknown Model (%d)", model);
		break;
	}
	printk("\n");
	cpuid(0x80000000, extended, unused, unused, unused);
	if(extended == 0)
		return 0;
	if(extended >= 0x80000002)
	{
		unsigned int j;
		printk("Detected Processor: ");
		for(j = 0x80000002; j <= 0x80000004; j++)
		{
			cpuid(j, eax, ebx, ecx, edx);
			printregs(eax, ebx, ecx, edx);
		}
		printk("\n");
	}
	if(extended >= 0x80000007)
	{
		cpuid(0x80000007, unused, unused, unused, edx);
		if(edx & 1)
		{
			printk("Temperature Sensing Diode Detected!\n");
		}
	}
	printk("Stepping: %d\n", stepping);
	printk("Reserved: %d\n", reserved);	
	return 0;
}

void sti()
{
     __asm__ __volatile__ ("sti");
}

void cli()
{
     __asm__ __volatile__ ("cli");
}

void halt()
{
     __asm__ __volatile__ ("hlt");
}

void reboot(void)
{
  outb(0x64, 0xFE);
}

void shutdown(void)
{
  outw(0x604, 0x2000);
}
