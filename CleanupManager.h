#pragma once
class CleanupManager {
public:
    static void DeleteOldImages(int days);
    static void DeleteOldLogs(int days);
};
