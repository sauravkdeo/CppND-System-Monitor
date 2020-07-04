#include <dirent.h>
#include <unistd.h>
#include <iomanip>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  std::string line, key;
  float memTotal, memFree, value;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal") {
          memTotal = value;
        }
        if (key == "MemFree") {
          memFree = value;
          break;
        }
      }
    }
  }
  return (float)(memTotal - memFree) / (float)memTotal;
}

long LinuxParser::UpTime() {
  long seg;
  std::string line;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    // line.replace(line.begin(),line.end(),'.',' ');
    std::istringstream linestream(line);
    linestream >> seg;
  }
  return seg;
}

long LinuxParser::Jiffies() {
  std::vector<std::string> cpuUtilization = LinuxParser::CpuUtilization();
  long jiffies{0};
  for (int i = kUser_; i <= kSteal_; i++) {
    jiffies += std::stol(cpuUtilization[i]);
  }
  return jiffies;
}

long LinuxParser::ActiveJiffies(int pid) {
  std::vector<std::string> buffer;
  std::string line;
  long totalTime{0};
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::stringstream sstream(line);
    while (std::getline(sstream, line, ' ')) {
      buffer.push_back(line);  // std::cout << line << std::endl;
    }
    totalTime = std::stol(buffer[13]) + std::stol(buffer[14]) +
                std::stol(buffer[15]) + std::stol(buffer[16]);
  }
  return totalTime;
}

long LinuxParser::ActiveJiffies() {
  return LinuxParser::Jiffies() - LinuxParser::IdleJiffies();
}

long LinuxParser::IdleJiffies() {
  std::vector<std::string> cpuUtilization = LinuxParser::CpuUtilization();
  long idleJiffies{0};
  for (int i = kIdle_; i <= kIOwait_; i++) {
    idleJiffies += std::stol(cpuUtilization[i]);
  }
  return idleJiffies;
}

vector<string> LinuxParser::CpuUtilization() {
  std::vector<std::string> buffer;
  std::string line;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::stringstream sstream(line);
    while (std::getline(sstream, line, ' ')) {
      if (line == "cpu" || line == "") {
      } else {
        buffer.push_back(line);  // std::cout << line << std::endl;
      }
    }
  }
  return buffer;
}

int LinuxParser::TotalProcesses() {
  int value = 0;
  std::string line, key;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          return value;
        }
      }
    }
  }
  return value;
}

int LinuxParser::RunningProcesses() {
  int value = 0;
  std::string line, key;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "procs_running") {
        return value;
      }
    }
  }
  return value;
}

string LinuxParser::Command(int pid) {
  std::string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    if (line == "") {
      return "None";
    }
    return line;
  }
  return std::string();
}

string LinuxParser::Ram(int pid) {
  std::string line, key, value;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize") {
          std::stringstream ram;
          ram << std::fixed << std::setprecision(3) << stof(value) / 1000;
          return ram.str();
        }
      }
    }
  }
  return std::to_string(0);
}

string LinuxParser::Uid(int pid) {
  std::string line, key, value;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid") {
          return value;
        }
      }
    }
  }
  return string("");
}

string LinuxParser::User(int pid) {
  std::string line, key, value, uid;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value >> uid) {
        if (uid == LinuxParser::Uid(pid)) {
          return key;
          break;
        }
      }
    }
  }
  return string();
}

long LinuxParser::UpTime(int pid) {
  std::string line, value;
  std::vector<std::string> buffer;
  long time = 0;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::stringstream sstream(line);
    while (std::getline(sstream, line, ' ')) {
      buffer.push_back(line);  // std::cout << line << std::endl;
    }
    if (buffer.size() > 0) {
      time = std::stol(buffer[21]) / sysconf(_SC_CLK_TCK);
      return time;
    }
  }
  return 0l;
}