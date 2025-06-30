# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/saad/esp/esp-idf/components/bootloader/subproject"
  "D:/github/davetetlow-PowerCutMonitor-fw/Fimware-V1/blink/build/bootloader"
  "D:/github/davetetlow-PowerCutMonitor-fw/Fimware-V1/blink/build/bootloader-prefix"
  "D:/github/davetetlow-PowerCutMonitor-fw/Fimware-V1/blink/build/bootloader-prefix/tmp"
  "D:/github/davetetlow-PowerCutMonitor-fw/Fimware-V1/blink/build/bootloader-prefix/src/bootloader-stamp"
  "D:/github/davetetlow-PowerCutMonitor-fw/Fimware-V1/blink/build/bootloader-prefix/src"
  "D:/github/davetetlow-PowerCutMonitor-fw/Fimware-V1/blink/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/github/davetetlow-PowerCutMonitor-fw/Fimware-V1/blink/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/github/davetetlow-PowerCutMonitor-fw/Fimware-V1/blink/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
