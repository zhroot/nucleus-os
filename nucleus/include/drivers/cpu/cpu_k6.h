#ifndef _CPU_K6_
#define _CPU_K6_

void AMD_K6_writeback(int family, int model, int stepping);
void AMD_K6_write_msr(unsigned long msr, unsigned long v1, unsigned long v2, struct regs *amd_regs);
void AMD_K6_read_msr(unsigned long msr, struct regs *amd_regs);

#endif
