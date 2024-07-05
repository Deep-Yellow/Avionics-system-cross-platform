#include <string>
#include <vector>

class PerformanceMetrics {
public:
    std::string responseTime;
    std::string processingDelay;
    std::string successRate;
    std::string availability;
    std::string communicationLatency;
};

class Location {
public:
    std::string nodeId;
    std::string physicalLocation;
};

class ResourceUsage {
public:
    std::string CPU;
    std::string memory;
};

class BusinessQuality {
public:
    std::string scanRange;
    std::string scanPrecision;
    bool underwaterDetection;
    std::string antiInterference;
};

class OperationalStatus {
public:
    bool serviceAvailability;
    bool isOccupied;
};

class ServiceInstance {
public:
    std::string serviceId;
    std::string serviceName;
    std::string status;
    PerformanceMetrics performanceMetrics;
    Location location;
    ResourceUsage resourceUsage;
    BusinessQuality businessQuality;
    OperationalStatus operationalStatus;
    std::string lastUpdated;
};

class ServiceList {
public:
    std::string serviceCategory;
    std::string serviceType;
    std::vector<ServiceInstance> instances;
};

