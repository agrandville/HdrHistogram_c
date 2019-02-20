/**
 * hdr_atomic.h
 * Written by Philip Orwig and released to the public domain,
 * as explained at http://creativecommons.org/publicdomain/zero/1.0/
 */

#ifndef HDR_ATOMIC_H__
#define HDR_ATOMIC_H__


#if defined(_MSC_VER)

#ifdef _M_X64
#pragma intrinsic(_InterlockedExchange64,_InterlockedExchangeAdd64)
#else
#pragma intrinsic(_InterlockedExchangeAdd64)
#endif

#include <stdint.h>
#include <windows.h>
#include <winnt.h>
#include <intrin.h>

static void __inline * hdr_atomic_load_pointer(volatile void** pointer)
{
#ifdef _M_IX86
	__asm {
		mov eax, dword ptr[pointer]
		mov ecx, dword ptr[eax]
		lock cmpxchg8b qword ptr[ecx];
 		mov dword ptr pointer, ecx;
 		mov eax, dword ptr[pointer]
}
#else
	return *pointer; 
#endif
}
// Intel 64 and IA-32 Architectures Software Developer's Manual, Volume 3A, 8.1.1. Guaranteed Atomic Operations:
static __inline void hdr_atomic_store_pointer(volatile void** pointer, volatile void* value)
{
	*pointer = value;
}

static int64_t __inline hdr_atomic_load_64(volatile int64_t* field)
{ 
#ifdef _M_IX86
	__asm {
		mov edi, dword ptr[field]
		xor eax, eax
		xor ebx, ebx
		xor ecx, ecx
		xor edx, edx
		lock cmpxchg8b qword ptr[edi]
	}
#else
	return *field;
#endif
}

static void __inline hdr_atomic_store_64(volatile int64_t* field, int64_t value)
{
#ifdef _M_IX86
	__asm {
		mov edi, field
		mov eax, dword ptr[edi]
		mov edx, dword ptr[edi + 4]
		mov ebx, dword ptr[value]
		mov ecx, dword ptr[value + 4]
		lock cmpxchg8b qword ptr[edi]
	}
#else
	_InterlockedExchange64(field, value);
#endif
}

static int64_t __inline hdr_atomic_exchange_64(volatile int64_t* field, int64_t initial)
{
#ifdef _M_IX86
	__asm {
		mov edi, field
		mov eax, dword ptr[edi]
		mov edx, dword ptr[edi + 4]
		mov ebx, dword ptr[initial]
		mov ecx, dword ptr[initial + 4]
		lock cmpxchg8b qword ptr[edi]
	}
#else
	return _InterlockedExchange64(field, initial);
#endif
}

static int64_t __inline hdr_atomic_add_fetch_64(volatile int64_t* field, int64_t value)
{
#ifdef _M_IX86
	return InterlockedExchangeAdd64(field, value) + value;
#else
	return _InterlockedExchangeAdd64(field, value) + value;
#endif
}

#elif defined(__ATOMIC_SEQ_CST)

#define hdr_atomic_load_pointer(x) __atomic_load_n(x, __ATOMIC_SEQ_CST)
#define hdr_atomic_store_pointer(f,v) __atomic_store_n(f,v, __ATOMIC_SEQ_CST)
#define hdr_atomic_load_64(x) __atomic_load_n(x, __ATOMIC_SEQ_CST)
#define hdr_atomic_store_64(f,v) __atomic_store_n(f,v, __ATOMIC_SEQ_CST)
#define hdr_atomic_exchange_64(f,i) __atomic_exchange_n(f,i, __ATOMIC_SEQ_CST)
#define hdr_atomic_add_fetch_64(field, value) __atomic_add_fetch(field, value, __ATOMIC_SEQ_CST)

#elif defined(__x86_64__)

#include <stdint.h>

static inline void* hdr_atomic_load_pointer(void** pointer)
{
   void* p =  *pointer;
	asm volatile ("" ::: "memory");
	return p;
}

static inline void hdr_atomic_store_pointer(volatile void** pointer, volatile void* value)
{
    asm volatile ("lock; xchgq %0, %1" : "+q" (value), "+m" (*pointer));
}

static inline int64_t hdr_atomic_load_64(int64_t* field)
{
    int64_t i = *field;
	asm volatile ("" ::: "memory");
	return i;
}

static inline void hdr_atomic_store_64(int64_t* field, int64_t value)
{
    asm volatile ("lock; xchgq %0, %1" : "+q" (value), "+m" (*field));
}

static inline int64_t hdr_atomic_exchange_64(volatile int64_t* field, int64_t value)
{
    int64_t result = 0;
    asm volatile ("lock; xchgq %1, %2" : "=r" (result), "+q" (value), "+m" (*field));
    return result;
}

static inline int64_t hdr_atomic_add_fetch_64(volatile int64_t* field, int64_t value)
{
    return __sync_add_and_fetch(field, value);
}

#else

#error "Unable to determine atomic operations for your platform"

#endif

#endif /* HDR_ATOMIC_H__ */
