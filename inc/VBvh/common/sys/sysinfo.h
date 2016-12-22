#ifndef SCULPT_SYSINFO_H
#define SCULPT_SYSINFO_H
#define CACHELINE_SIZE 64
#define PAGE_SIZE 4096

#include "common/sys/platform.h"

namespace VBvh
{
  enum CPUModel {
    CPU_UNKNOWN,
    CPU_CORE1,
    CPU_CORE2,
    CPU_CORE_NEHALEM,
    CPU_CORE_SANDYBRIDGE
  };

  /*! get the full path to the running executable */
  std::string getExecutableFileName();

  /*! return platform name */
  std::string getPlatformName();

  /*! get the full name of the compiler */
  std::string getCompilerName();

  /*! return the name of the CPU */
  std::string getCPUVendor();

  /*! get microprocessor model */
  CPUModel getCPUModel(); 

  /*! CPU features */
  static const int CPU_FEATURE_SSE   = 1 << 0;
  static const int CPU_FEATURE_SSE2  = 1 << 1;
  static const int CPU_FEATURE_SSE3  = 1 << 2;
  static const int CPU_FEATURE_SSSE3 = 1 << 3;
  static const int CPU_FEATURE_SSE41 = 1 << 4;
  static const int CPU_FEATURE_SSE42 = 1 << 5; 
  static const int CPU_FEATURE_POPCNT = 1 << 6;
  static const int CPU_FEATURE_AVX   = 1 << 7;
  static const int CPU_FEATURE_F16C  = 1 << 8;
  static const int CPU_FEATURE_RDRAND = 1 << 9;
  static const int CPU_FEATURE_AVX2  = 1 << 10;
  static const int CPU_FEATURE_FMA3  = 1 << 11;
  static const int CPU_FEATURE_LZCNT = 1 << 12;
  static const int CPU_FEATURE_BMI1  = 1 << 13;
  static const int CPU_FEATURE_BMI2  = 1 << 14;
  static const int CPU_FEATURE_KNC   = 1 << 15;

  /*! get CPU features */
  extern int cpu_features;
  int getCPUFeatures();

  /*! convert CPU features into a string */
  std::string stringOfCPUFeatures(int features);

  /*! return the number of logical threads of the system */
  size_t getNumberOfLogicalThreads();
  
  /*! return the number of cores of the system */
  size_t getNumberOfCores();
  
  /*! returns the size of the terminal window in characters */
  int getTerminalWidth();
}
#endif
