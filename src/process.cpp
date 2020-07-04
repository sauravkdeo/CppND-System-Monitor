
#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) {
    this->pid_ = pid;
    this->command_ = LinuxParser::Command(this->pid_);
}

int Process::Pid() { return this->pid_; }

float Process::CpuUtilization() {

    float activeJiffie = (float)LinuxParser::ActiveJiffies();
    float upTime = (float)LinuxParser::UpTime();
    float upTimePid = (float)LinuxParser::UpTime(this->pid_);
    float seconds = (upTime - upTimePid) / (float)sysconf(_SC_CLK_TCK);
    
    return (activeJiffie / (float)sysconf(_SC_CLK_TCK) / seconds);
}

string Process::Command() { return this->command_; }

string Process::Ram() { return LinuxParser::Ram(this->pid_); }

string Process::User() { return LinuxParser::User(this->pid_); }

long int Process::UpTime() { return LinuxParser::UpTime(this->pid_); }

bool Process::operator<(Process const& a) const { 
    long myRam = std::stol(LinuxParser::Ram(this->pid_));
    long otherRam = std::stol(LinuxParser::Ram(a.pid_));
    return myRam < otherRam;
}