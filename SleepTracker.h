/**
 * @file SleepTracker.h
 * @brief Module theo dõi và phân tích giấc ngủ
 */

#ifndef SLEEPTRACKER_H
#define SLEEPTRACKER_H

#include <Arduino.h>
#include "Globals.h"

/**
 * @brief Cập nhật trạng thái theo dõi giấc ngủ
 */
void SleepTracking_Update();

/**
 * @brief In thống kê giấc ngủ ra Serial
 */
void PrintSleepStatistics();

/**
 * @brief Lưu bản ghi giấc ngủ
 */
void SaveSleepRecord();

/**
 * @brief In lịch sử giấc ngủ
 */
void PrintSleepHistory();

/**
 * @brief Lấy thống kê giấc ngủ hiện tại
 */
void GetSleepStatistics(unsigned long* totalTime, unsigned long* leftTime, 
                        unsigned long* rightTime, unsigned long* centerTime, 
                        int* changes);

/**
 * @brief Lấy bản ghi giấc ngủ theo index
 */
SleepRecord GetSleepRecord(int index);

/**
 * @brief Xóa lịch sử giấc ngủ
 */
void ClearSleepHistory();

/**
 * @brief Gửi thống kê giấc ngủ lên server
 */
void SendSleepStatisticsToServer();

/**
 * @brief Xác định giai đoạn giấc ngủ
 * @return 1: Ngủ sâu đầu, 2: Ngủ sâu giữa, 3: Ngủ sâu cuối, 0: Không ngủ
 */
int GetSleepStage();

/**
 * @brief Gửi dữ liệu giấc ngủ lên server
 */
void SendSleepData();

/**
 * @brief Cập nhật việc gửi dữ liệu giấc ngủ
 */
void SleepDataSend_Update();

/**
 * @brief Lấy timestamp hiện tại từ NTP
 */
unsigned long getCurrentTimestamp();

#endif // SLEEPTRACKER_H