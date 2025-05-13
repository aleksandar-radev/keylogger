#include "CleanupManager.h"
#include <filesystem>
#include <chrono>
#include <ctime>
#include "FileOutput.h"

void CleanupManager::DeleteOldImages(int days) {
    int deletedCount = 0;
    std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    if (std::filesystem::exists("./images") && std::filesystem::is_directory("./images")) {
        for (const auto &entry : std::filesystem::directory_iterator("./images")) {
            const auto modifiedTime = std::filesystem::last_write_time(entry);
            const auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                modifiedTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
            const auto modifiedTime_t = std::chrono::system_clock::to_time_t(sctp);
            double ageInDays = difftime(currentTime, modifiedTime_t) / (60 * 60 * 24);
            if (ageInDays > days) {
                std::filesystem::remove(entry);
                ++deletedCount;
            }
        }
    }
    FileOutput::lastDeletedImages = deletedCount;
}

void CleanupManager::DeleteOldLogs(int days) {
    FileOutput logger;
    int deleted = logger.DeleteOldLogs(days);
    FileOutput::lastDeletedLogs = deleted;
}
