/*
 * CPU detection and identification for Xnix.
 *
 * This module queries the CPU vendor and brand via the CPUID instruction,
 * stores the results in string buffers, and provides a function to print
 * the collected information.
 *
 * Author: Agustin Gutierrez
 * Based on work by brynet (brynet@gmail.com)
 *
 * License:
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <xnix/vga.h>

/**
 * CPUID leaf definitions.
 */
#define CPUID_VENDOR_ID      0x00000000
#define CPUID_BRAND_STRING1  0x80000002
#define CPUID_BRAND_STRING2  0x80000003
#define CPUID_BRAND_STRING3  0x80000004

/**
 * I/O ports for system control.
 */
#define KBC_COMMAND_PORT       0x64   /**< Keyboard controller command port */
#define RESET_COMMAND          0xFE   /**< Keyboard controller reset command */
#define ACPI_SHUTDOWN_PORT     0x604  /**< ACPI port for shutdown */
#define ACPI_SHUTDOWN_VALUE    0x2000 /**< Value to trigger ACPI shutdown */

/**
 * Execute the CPUID instruction with the given leaf.
 *
 * @param leaf  CPUID leaf number
 * @param a     EAX register output
 * @param b     EBX register output
 * @param c     ECX register output
 * @param d     EDX register output
 */
#define cpuid(leaf, a, b, c, d) \
    __asm__("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(leaf))

/**
 * Buffers to hold CPU identification strings.
 */
static char cpu_vendor[13] = {0};  /**< Null-terminated CPU vendor string */
static char cpu_brand[49]  = {0};  /**< Null-terminated CPU brand string */

/**
 * Collects the CPU vendor string into cpu_vendor buffer.
 *
 * @param ebx  EBX register returning first part of vendor ID
 * @param ecx  ECX register returning third part of vendor ID
 * @param edx  EDX register returning second part of vendor ID
 */
static void collect_vendor(uint32_t ebx, uint32_t ecx, uint32_t edx)
{
    /* Order: EBX, EDX, ECX */
    *(uint32_t *)&cpu_vendor[0] = ebx;
    *(uint32_t *)&cpu_vendor[4] = edx;
    *(uint32_t *)&cpu_vendor[8] = ecx;
    cpu_vendor[12] = '\0';
}

/**
 * Collects the CPU brand string into cpu_brand buffer.
 * Reads three successive CPUID leaves.
 */
static void collect_brand(void)
{
    uint32_t regs[4];
    char *ptr = cpu_brand;

    for (uint32_t leaf = CPUID_BRAND_STRING1; leaf <= CPUID_BRAND_STRING3; leaf++) {
        cpuid(leaf, regs[0], regs[1], regs[2], regs[3]);
        /* Copy four 32-bit registers into the buffer */
        for (int i = 0; i < 4; i++) {
            *ptr++ = (char)((regs[0] >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < 4; i++) {
            *ptr++ = (char)((regs[1] >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < 4; i++) {
            *ptr++ = (char)((regs[2] >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < 4; i++) {
            *ptr++ = (char)((regs[3] >> (i * 8)) & 0xFF);
        }
    }
    cpu_brand[48] = '\0';
}

/**
 * Prints the collected CPU information via printk.
 */
static void print_cpu_info(void)
{
    printk("%s - %s\n", cpu_vendor, cpu_brand);
}

/**
 * Detects the CPU vendor and brand, stores them in buffers,
 * and prints the full identification.
 *
 * @return 0 on success
 */
int detect_cpu(void)
{
    uint32_t eax, ebx, ecx, edx;

    /* Query vendor ID */
    cpuid(CPUID_VENDOR_ID, eax, ebx, ecx, edx);
    collect_vendor(ebx, ecx, edx);

    /* Query brand string */
    collect_brand();

    /* Output all information at once */
    print_cpu_info();

    return 0;
}

/* System control functions */
void sti(void)     { __asm__ __volatile__("sti"); }
void cli(void)     { __asm__ __volatile__("cli"); }
void halt(void)    { __asm__ __volatile__("hlt"); }
void reboot(void)  { outb(KBC_COMMAND_PORT, RESET_COMMAND); }
void shutdown(void){ outw(ACPI_SHUTDOWN_PORT, ACPI_SHUTDOWN_VALUE); }

