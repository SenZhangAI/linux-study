/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_ARM_IRQFLAGS_H
#define __ASM_ARM_IRQFLAGS_H

#ifdef __KERNEL__

#include <asm/ptrace.h>

/*
 * CPU interrupt mask handling.
 */
#ifdef CONFIG_CPU_V7M
#define IRQMASK_REG_NAME_R "primask"
#define IRQMASK_REG_NAME_W "primask"
#define IRQMASK_I_BIT	1
#else
#define IRQMASK_REG_NAME_R "cpsr"
#define IRQMASK_REG_NAME_W "cpsr_c"
#define IRQMASK_I_BIT	PSR_I_BIT
#endif

#if __LINUX_ARM_ARCH__ >= 6

#define arch_local_irq_save arch_local_irq_save
static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags;

	asm volatile(
		"	mrs	%0, " IRQMASK_REG_NAME_R "	@ arch_local_irq_save\n"
		"	cpsid	i"
		: "=r" (flags) : : "memory", "cc");
	return flags;
}

#define arch_local_irq_enable arch_local_irq_enable
static inline void arch_local_irq_enable(void)
{
	asm volatile(
		"	cpsie i			@ arch_local_irq_enable"
		:
		:
		: "memory", "cc");
}

#define arch_local_irq_disable arch_local_irq_disable
static inline void arch_local_irq_disable(void)
{
	asm volatile(
		"	cpsid i			@ arch_local_irq_disable"
		:
		:
		: "memory", "cc");
}

#define local_fiq_enable()  __asm__("cpsie f	@ __stf" : : : "memory", "cc")
#define local_fiq_disable() __asm__("cpsid f	@ __clf" : : : "memory", "cc")

#ifndef CONFIG_CPU_V7M
#define local_abt_enable()  __asm__("cpsie a	@ __sta" : : : "memory", "cc")
#define local_abt_disable() __asm__("cpsid a	@ __cla" : : : "memory", "cc")
#else
#define local_abt_enable()	do { } while (0)
#define local_abt_disable()	do { } while (0)
#endif
#else

/*
 * Save the current interrupt enable state & disable IRQs
 */
#define arch_local_irq_save arch_local_irq_save
static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags, temp;

    //NOTE MRS 指令:  对状态寄存器CPSR和SPSR进行读操作
    //     MSR指令:   对状态寄存器CPSR和SPSR进行写操作。与MRS配合使用，可以实现对CPSR或SPSR寄存器的读-修改-写操作，
    //                可以切换处理器模式、或者允许/禁止IRQ/FIQ中断等。
    //     CPSR 程序状态寄存器(current program status register)
    //     IRQ interrupt request
    //     FIQ 快速中断请求（Fast Interrupt Request), FIQ和IRQ(外部中断模式)之间有很大的区别。
    //         FIQ模式必须尽快处理，处理结束后离开这个模式；IRQ模式可以被FIQ模式中断，但IRQ不能中断FIQ模式
    //     参考 https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
	asm volatile(
		"	mrs	%0, cpsr	@ arch_local_irq_save\n"
		"	orr	%1, %0, #128\n"    //NOTE temp = flags | 128 (0x 1000 0000) IRQ禁止位，使能IRQ禁止
		"	msr	cpsr_c, %1"        //     msr cpsr_c temp 使能cpsr_c的IRQ禁止位
		: "=r" (flags), "=r" (temp)
		:
		: "memory", "cc"); //NOTE The "cc" clobber indicates that the assembler code modifies the flags register.
	return flags;
}

/*
 * Enable IRQs
 */
#define arch_local_irq_enable arch_local_irq_enable
static inline void arch_local_irq_enable(void)
{
	unsigned long temp;
	asm volatile(
		"	mrs	%0, cpsr	@ arch_local_irq_enable\n"
		"	bic	%0, %0, #128\n"
		"	msr	cpsr_c, %0"
		: "=r" (temp)
		:
		: "memory", "cc");
}

/*
 * Disable IRQs
 */
#define arch_local_irq_disable arch_local_irq_disable
static inline void arch_local_irq_disable(void)
{
	unsigned long temp;
	asm volatile(
		"	mrs	%0, cpsr	@ arch_local_irq_disable\n"
		"	orr	%0, %0, #128\n"
		"	msr	cpsr_c, %0"
		: "=r" (temp)
		:
		: "memory", "cc");
}

/*
 * Enable FIQs
 */
#define local_fiq_enable()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ stf\n"		\
"	bic	%0, %0, #64\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory", "cc");					\
	})

/*
 * Disable FIQs
 */
#define local_fiq_disable()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ clf\n"		\
"	orr	%0, %0, #64\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory", "cc");					\
	})

#define local_abt_enable()	do { } while (0)
#define local_abt_disable()	do { } while (0)
#endif

/*
 * Save the current interrupt enable state.
 */
#define arch_local_save_flags arch_local_save_flags
static inline unsigned long arch_local_save_flags(void)
{
	unsigned long flags;
	asm volatile(
		"	mrs	%0, " IRQMASK_REG_NAME_R "	@ local_save_flags"
		: "=r" (flags) : : "memory", "cc");
	return flags;
}

/*
 * restore saved IRQ & FIQ state
 */
#define arch_local_irq_restore arch_local_irq_restore
static inline void arch_local_irq_restore(unsigned long flags)
{
	asm volatile(
		"	msr	" IRQMASK_REG_NAME_W ", %0	@ local_irq_restore"
		:
		: "r" (flags)
		: "memory", "cc");
}

#define arch_irqs_disabled_flags arch_irqs_disabled_flags
static inline int arch_irqs_disabled_flags(unsigned long flags)
{
	return flags & IRQMASK_I_BIT;
}

#include <asm-generic/irqflags.h>

#endif /* ifdef __KERNEL__ */
#endif /* ifndef __ASM_ARM_IRQFLAGS_H */
