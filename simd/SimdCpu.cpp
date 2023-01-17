//{{{
/*
* Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2022 Yermalayeu Ihar,
*               2022-2022 Souriya Trinh,
*               2022-2022 Fabien Spindler.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
//}}}
//{{{  includes
#include "SimdCpu.h"
#include "SimdMath.h"

#include <vector>
#include <sstream>
#include <iostream>
#include <thread>

#if defined(_MSC_VER)
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif

  #include <windows.h>
  #include <intrin.h>

#elif defined(__GNUC__)
  #include <unistd.h>
  #include <stdbool.h>
  #include <stdlib.h>

  #if defined(SIMD_X86_ENABLE) || defined(SIMD_X64_ENABLE)
    #include <cpuid.h>
  #endif

  #if defined(SIMD_PPC_ENABLE) || defined(SIMD_PPC64_ENABLE) || defined(SIMD_ARM_ENABLE) || defined(SIMD_ARM64_ENABLE)
    #include <fcntl.h>
    #if !defined(__APPLE__)
      #include <sys/auxv.h>
      #if (defined(SIMD_ARM_ENABLE) || defined(SIMD_ARM64_ENABLE)) && !defined(__FreeBSD__)
        #include <asm/hwcap.h>
      #endif
    #endif
  #endif
#else
  #error Do not know how to detect CPU info
#endif
//}}}

namespace Simd { 
  namespace Base {
    #if defined(SIMD_X86_ENABLE) || defined(SIMD_X64_ENABLE)
      //{{{
      SIMD_INLINE bool CpuId (int eax, int ecx, unsigned int *registers)
      {
         #if defined(_MSC_VER)
           __cpuidex ((int*)registers, eax, ecx);

         #elif (defined __GNUC__)
           if (__get_cpuid_max (0, NULL) < eax)
             return false;
           __cpuid_count (eax, ecx, registers[Cpuid::Eax],
                                    registers[Cpuid::Ebx],
                                    registers[Cpuid::Ecx],
                                    registers[Cpuid::Edx]);

         #else
           #error Do not know how to detect CPU info!
         #endif

        return true;
        }
      //}}}
      //{{{
      bool CheckBit (int eax, int ecx, Cpuid::Register index, Cpuid::Bit bit) {

        unsigned int registers[4] = { 0, 0, 0, 0 };
        if (!CpuId (eax, ecx, registers))
          return false;
        return (registers[index] & bit) == bit;
        }
      //}}}
      //{{{
      const char* VendorId() {

        unsigned int regs[4] = { 0, 0, 0, 0};
        CpuId (0, 0, regs);
        static unsigned int vendorId[4] = { regs[1], regs[3], regs[2], 0 };
        return (char*)vendorId;
        }
      //}}}
    #endif

    #if defined(__GNUC__) && (defined(SIMD_PPC_ENABLE) || defined(SIMD_PPC64_ENABLE) || \
                              defined(SIMD_ARM_ENABLE) || defined(SIMD_ARM64_ENABLE)) && !defined(__APPLE__)
      //{{{
      bool CheckBit (int at, int bit) {

        bool result = false;
        int file = ::open("/proc/self/auxv", O_RDONLY);
        if (file < 0)
          return false;

        const ssize_t size = 64;
        unsigned long buffer[size];
        for (ssize_t count = size; count == size;) {
          count = ::read(file, buffer, sizeof(buffer)) / sizeof(unsigned long);
          for (int i = 0; i < count; i += 2) {
            if (buffer[i] == (unsigned)at) {
              result = !!(buffer[i + 1] & bit);
              count = 0;
              }
            if (buffer[i] == AT_NULL)
              count = 0;
            }
          }

        ::close(file);
        return result;
        }
      //}}}
    #endif

    #if defined(_MSC_VER)
      //{{{  windows proc info
      typedef SYSTEM_LOGICAL_PROCESSOR_INFORMATION Info;
      //{{{
      void GetLogicalProcessorInformation (std::vector<Info> & info)
      {
          DWORD size = 0;
          ::GetLogicalProcessorInformation(0, &size);
          info.resize(size / sizeof(Info));
          ::GetLogicalProcessorInformation(info.data(), &size);
      }
      //}}}

      //{{{
      size_t CpuSocketNumber()
      {
          std::vector<Info> info;
          GetLogicalProcessorInformation(info);
          size_t number = 0;
          for (size_t i = 0; i < info.size(); ++i)
              if (info[i].Relationship == ::RelationNumaNode)
                  number++;
          return number;
      }
      //}}}
      //{{{
      size_t CpuCoreNumber()
      {
          std::vector<Info> info;
          GetLogicalProcessorInformation(info);
          size_t number = 0;
          for (size_t i = 0; i < info.size(); ++i)
              if (info[i].Relationship == ::RelationProcessorCore)
                  number++;
          return number;
      }
      //}}}
      //{{{
      size_t CpuCacheSize (size_t level)
      {
          std::vector<Info> info;
          GetLogicalProcessorInformation(info);
          for (size_t i = 0; i < info.size(); ++i)
              if (info[i].Relationship == ::RelationCache && info[i].Cache.Level == level && (info[i].Cache.Type == ::CacheData || info[i].Cache.Type == CacheUnified))
                  return info[i].Cache.Size;
          return 0;
      }
      //}}}
      //}}}
    #elif defined(__GNUC__)
      //{{{  gnu proc info
      //{{{
      size_t CpuSocketNumber()
      {
          uint32_t number = 0;
          ::FILE * p = ::popen("lscpu -b -p=Socket 2>/dev/null | grep -v '^#' | sort -u 2>/dev/null | wc -l 2>/dev/null", "r");
          if (p)
          {
              char buffer[PATH_MAX];
              while (::fgets(buffer, PATH_MAX, p));
              number = ::atoi(buffer);
              ::pclose(p);
          }
          return number;
      }
      //}}}
      //{{{
      size_t CpuCoreNumber()
      {
          uint32_t number = 0;
          ::FILE * p = ::popen("lscpu -b -p=Core 2>/dev/null | grep -v '^#' | sort -u 2>/dev/null | wc -l 2>/dev/null", "r");
          if (p)
          {
              char buffer[PATH_MAX];
              while (::fgets(buffer, PATH_MAX, p));
              number = ::atoi(buffer);
              ::pclose(p);
          }
          return number;
      }
      //}}}
      //{{{
      SIMD_INLINE size_t CorrectIfZero (size_t value, size_t otherwise)
      {
          return value ? value : otherwise;
      }
      //}}}
      #if defined(_SC_LEVEL1_DCACHE_SIZE) && defined(_SC_LEVEL2_CACHE_SIZE) && defined(_SC_LEVEL3_CACHE_SIZE)
        //{{{
        size_t CpuCacheSize (size_t level) {

          switch (level) {
            case 1:
              {
              const size_t sz = ::sysconf(_SC_LEVEL1_DCACHE_SIZE) < 0 ? 0 : ::sysconf(_SC_LEVEL1_DCACHE_SIZE);
              return CorrectIfZero(sz, 32 * 1024);
              }

            case 2:
              {
              const size_t sz = ::sysconf(_SC_LEVEL2_CACHE_SIZE) < 0 ? 0 : ::sysconf(_SC_LEVEL2_CACHE_SIZE);
              return CorrectIfZero(sz, 256 * 1024);
              }

            case 3:
              {
              const size_t sz = ::sysconf(_SC_LEVEL3_CACHE_SIZE) < 0 ? 0 : ::sysconf(_SC_LEVEL3_CACHE_SIZE);
              return CorrectIfZero(sz, 2048 * 1024);
              }

            default:
              return 0;
            }
          }
        //}}}
      #else
         //{{{
         size_t CpuCacheSize (size_t level) {

            switch (level) {
             case 1: return 32 * 1024;
             case 2: return 256 * 1024;
             case 3: return 2048 * 1024;
             default: return 0;
             }
           }
         //}}}
      //}}}
      #endif
    #endif

    size_t CpuThreadNumber() { return std::thread::hardware_concurrency(); }

    size_t g_threadNumber = 1;
    size_t GetThreadNumber() { return g_threadNumber; }
    //{{{
    void SetThreadNumber (size_t threadNumber) {
      g_threadNumber = Simd::RestrictRange<size_t>(threadNumber, 1, std::thread::hardware_concurrency());
      }
    //}}}
    }

  #ifdef SIMD_SSE41_ENABLE
    //{{{
    namespace Sse41 {
      //{{{
      SIMD_INLINE bool SupportedByCPU() {

        return Base::CheckBit(1, 0, Cpuid::Edx, Cpuid::SSE) &&
               Base::CheckBit(1, 0, Cpuid::Edx, Cpuid::SSE2) &&
               Base::CheckBit(1, 0, Cpuid::Ecx, Cpuid::SSE3) &&
               Base::CheckBit(1, 0, Cpuid::Ecx, Cpuid::SSSE3) &&
               Base::CheckBit(1, 0, Cpuid::Ecx, Cpuid::SSE41) &&
               Base::CheckBit(1, 0, Cpuid::Ecx, Cpuid::SSE42);
        }
      //}}}
      //{{{
      SIMD_INLINE bool SupportedByOS() {

        #if defined(_MSC_VER)
          __try {
            int value = _mm_testz_si128(_mm_set1_epi8(0), _mm_set1_epi8(-1)); // try to execute of SSE41 instructions;
            uint32_t crc = _mm_crc32_u8(0, 1); // try to execute of SSE42 instructions;
            }
          __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
            }
        #endif

        return true;
        }
      //}}}
      bool GetEnable() { return SupportedByCPU() && SupportedByOS(); }
      }
    //}}}
  #endif

  #ifdef SIMD_AVX_ENABLE
    //{{{
    namespace Avx {
      //{{{
      SIMD_INLINE bool SupportedByCPU() {

        return Base::CheckBit(1, 0, Cpuid::Ecx, Cpuid::OSXSAVE) &&
               Base::CheckBit(1, 0, Cpuid::Ecx, Cpuid::AVX);
        }
      //}}}
      //{{{
      SIMD_INLINE bool SupportedByOS() {

        #if defined(_MSC_VER)
          __try {
            __m256d value = _mm256_set1_pd(1.0);// try to execute of AVX instructions;
            }
          __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
            }
        #endif

        return true;
        }
      //}}}
      bool GetEnable() { return SupportedByCPU() && SupportedByOS(); }
      }
    //}}}
  #endif

  #ifdef SIMD_AVX2_ENABLE
    //{{{
    namespace Avx2 {
      //{{{
      SIMD_INLINE bool SupportedByCPU() {

        return Base::CheckBit(7, 0, Cpuid::Ebx, Cpuid::AVX2) &&
               Base::CheckBit(7, 0, Cpuid::Ebx, Cpuid::BMI1) &&
               Base::CheckBit(7, 0, Cpuid::Ebx, Cpuid::BMI2) &&
               Base::CheckBit(1, 0, Cpuid::Ecx, Cpuid::OSXSAVE) &&
               Base::CheckBit(1, 0, Cpuid::Ecx, Cpuid::FMA) &&
               Base::CheckBit(1, 0, Cpuid::Ecx, Cpuid::F16C);
        }
      //}}}
      //{{{
      SIMD_INLINE bool SupportedByOS() {

        #if defined(_MSC_VER)
          __try {
            __m256i value = _mm256_abs_epi8(_mm256_set1_epi8(1));// try to execute of AVX2 instructions;
            }
          __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
            }
        #endif

        return true;
        }
      //}}}
      bool GetEnable() { return SupportedByCPU() && SupportedByOS(); }
      //{{{
      bool GetSlowGather() {

        const char* vendorId = Base::VendorId();

        if (memcmp (vendorId, "GenuineIntel", 12) == 0)
          return false;
        else if (memcmp (vendorId, "AuthenticAMD", 12) == 0)
          return true;
        else
          return true;
        }
      //}}}
      }
    //}}}
  #endif

  #ifdef SIMD_AVX512BW_ENABLE
    //{{{
    namespace Avx512bw {
      //{{{
      SIMD_INLINE bool SupportedByCPU() {

        return Base::CheckBit(7, 0, Cpuid::Ebx, Cpuid::AVX512_F) &&
               Base::CheckBit(7, 0, Cpuid::Ebx, Cpuid::AVX512_CD) &&
               Base::CheckBit(7, 0, Cpuid::Ebx, Cpuid::AVX512_DQ) &&
               Base::CheckBit(7, 0, Cpuid::Ebx, Cpuid::AVX512_BW) &&
               Base::CheckBit(7, 0, Cpuid::Ebx, Cpuid::AVX512_VL);
        }
      //}}}
      //{{{
      SIMD_INLINE bool SupportedByOS() {

        #if defined(_MSC_VER)
          __try {
            __m512i value = _mm512_abs_epi8(_mm512_set1_epi8(1));// try to execute of AVX-512BW instructions;
            }
          __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
            }
        #endif

        return true;
        }
      //}}}
      bool GetEnable() { return SupportedByCPU() && SupportedByOS(); }
      }
    //}}}
  #endif

  namespace Cpu {
    const size_t SOCKET_NUMBER = Base::CpuSocketNumber();
    const size_t CORE_NUMBER = Base::CpuCoreNumber();
    const size_t THREAD_NUMBER = Base::CpuThreadNumber();
    const size_t L1_CACHE_SIZE = Base::CpuCacheSize(1);
    const size_t L2_CACHE_SIZE = Base::CpuCacheSize(2);
    const size_t L3_CACHE_SIZE = Base::CpuCacheSize(3);
    }
  }
