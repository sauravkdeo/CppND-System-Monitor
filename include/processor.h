#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "linux_parser.h"

// float prevJiffies = (float)LinuxParser::Jiffies();
// float prevActiveJiffies = (float)LinuxParser::ActiveJiffies();

class Processor {
 public:
  float Utilization();  

 private:
  long jiffies{0};
  long activeJiffies{0};
};

#endif