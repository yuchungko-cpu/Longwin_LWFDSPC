#include "s_flash_eeprom.h"
#include "s_hal_eeprom.h"
#include <stddef.h>
#include <stdbool.h>

// ============================================================================
// Flash 模擬 EEPROM 使用範例
// ============================================================================

/**
 * @brief 範例：初始化 Flash 模擬 EEPROM
 * @details 展示如何初始化 Flash 模擬 EEPROM 模組
 * @return bool - true 表示初始化成功, false 表示失敗
 */
bool flashEeprom_example_init(void) {
    // ? 初始化 Flash 模擬 EEPROM 模組
    if (!flashEeprom_init()) {
        // ? 初始化失敗，可以嘗試格式化
        if (!flashEeprom_format()) {
            return false; // 格式化也失敗
        }
        
        // ? 重新嘗試初始化
        if (!flashEeprom_init()) {
            return false;
        }
    }
    
    // ? 檢查記憶體狀態
    if (!flashEeprom_checkMemoryStatus()) {
        return false;
    }
    
    return true;
}

/**
 * @brief 範例：使用 Flash 模擬 EEPROM 儲存參數
 * @details 展示如何將 EEPROM 參數儲存到 Flash 中
 * @param pstParams 要儲存的參數結構指標
 * @return bool - true 表示儲存成功, false 表示失敗
 */
bool flashEeprom_example_saveParams(const S_EEPROM_PARAM *pstParams) {
    if (pstParams == NULL) {
        return false;
    }
    
    // ? 檢查 Flash 模擬 EEPROM 是否已初始化
    if (flashEeprom_isBusy()) {
        return false; // Flash 忙碌中
    }
    
    // ? 計算參數的校驗和
    uint16_t u16Checksum = halEeprom_calculateChecksum(pstParams);
    
    // ? 建立包含校驗和的完整資料結構
    uint8_t au8ParamBuffer[sizeof(S_EEPROM_PARAM)];
    const uint8_t* pu8Source = (const uint8_t*)pstParams;
    
    // ? 複製參數資料
    for (size_t i = 0; i < sizeof(S_EEPROM_PARAM); i++) {
        au8ParamBuffer[i] = pu8Source[i];
    }
    
    // ? 寫入主要參數區域
    uint32_t u32ParamAddress = 0x800000; // Flash 資料記憶體起始位址
    
    if (!flashEeprom_write(u32ParamAddress, au8ParamBuffer, sizeof(S_EEPROM_PARAM))) {
        return false;
    }
    
    // ? 建立備份
    if (!flashEeprom_createBackup(au8ParamBuffer, sizeof(S_EEPROM_PARAM))) {
        return false; // 備份失敗，但主要資料已寫入
    }
    
    return true;
}

/**
 * @brief 範例：從 Flash 模擬 EEPROM 讀取參數
 * @details 展示如何從 Flash 中讀取 EEPROM 參數
 * @param pstParams 參數結構指標，用於存放讀取的資料
 * @return bool - true 表示讀取成功, false 表示失敗
 */
bool flashEeprom_example_loadParams(S_EEPROM_PARAM *pstParams) {
    if (pstParams == NULL) {
        return false;
    }
    
    // ? 檢查 Flash 模擬 EEPROM 是否已初始化
    if (flashEeprom_isBusy()) {
        return false; // Flash 忙碌中
    }
    
    // ? 從主要參數區域讀取
    uint32_t u32ParamAddress = 0x800000; // Flash 資料記憶體起始位址
    uint8_t au8ParamBuffer[sizeof(S_EEPROM_PARAM)];
    
    if (!flashEeprom_read(u32ParamAddress, au8ParamBuffer, sizeof(S_EEPROM_PARAM))) {
        // ? 主要區域讀取失敗，嘗試從備份恢復
        if (!flashEeprom_restoreFromBackup(au8ParamBuffer, sizeof(S_EEPROM_PARAM))) {
            return false; // 備份也失敗
        }
    }
    
    // ? 將讀取的資料複製到參數結構
    uint8_t* pu8Dest = (uint8_t*)pstParams;
    for (size_t i = 0; i < sizeof(S_EEPROM_PARAM); i++) {
        pu8Dest[i] = au8ParamBuffer[i];
    }
    
    // ? 驗證參數有效性
    if (!halEeprom_validateData(pstParams)) {
        return false; // 參數驗證失敗
    }
    
    // ? 驗證校驗和
    if (!halEeprom_verifyChecksum(pstParams)) {
        return false; // 校驗和驗證失敗
    }
    
    return true;
}

/**
 * @brief 範例：更新單一參數
 * @details 展示如何更新 Flash 中的單一參數
 * @param u8ParamIndex 參數索引 (0-9)
 * @param u32NewValue 新的參數值
 * @return bool - true 表示更新成功, false 表示失敗
 */
bool flashEeprom_example_updateSingleParam(uint8_t u8ParamIndex, uint32_t u32NewValue) {
    // ? 讀取當前參數
    S_EEPROM_PARAM stCurrentParams;
    if (!flashEeprom_example_loadParams(&stCurrentParams)) {
        return false;
    }
    
    // ? 更新指定參數
    if (!halEeprom_updateSingleParam(u8ParamIndex, u32NewValue)) {
        return false;
    }
    
    // ? 將更新後的參數儲存到 Flash
    return flashEeprom_example_saveParams(&stCurrentParams);
}

/**
 * @brief 範例：檢查 Flash 記憶體狀態
 * @details 展示如何檢查 Flash 記憶體的健康狀態
 */
void flashEeprom_example_checkStatus(void) {
    // ? 檢查記憶體狀態
    if (!flashEeprom_checkMemoryStatus()) {
        // ? 記憶體有問題，可以嘗試格式化
        flashEeprom_format();
        return;
    }
    
    // ? 取得空間資訊
    uint32_t u32TotalSize, u32UsedSize, u32FreeSize;
    flashEeprom_getSpaceInfo(&u32TotalSize, &u32UsedSize, &u32FreeSize);
    
    // ? 取得磨損統計
    uint16_t au16WearCounts[16]; // 16 個頁面
    flashEeprom_getWearStatistics(au16WearCounts);
    
    // ? 檢查磨損最嚴重的頁面
    uint16_t u16MaxWear = 0;
    uint8_t u8MaxWearPage = 0;
    
    for (uint8_t i = 0; i < 16; i++) {
        if (au16WearCounts[i] > u16MaxWear) {
            u16MaxWear = au16WearCounts[i];
            u8MaxWearPage = i;
        }
    }
    
    // ? 如果磨損過於嚴重，執行磨損平衡
    if (u16MaxWear > 1000) { // 假設 1000 次為閾值
        flashEeprom_wearLeveling();
    }
}

/**
 * @brief 範例：完整的參數管理流程
 * @details 展示完整的參數讀取、驗證、更新、儲存流程
 * @return bool - true 表示流程成功, false 表示失敗
 */
bool flashEeprom_example_completeWorkflow(void) {
    // ? 1. 初始化
    if (!flashEeprom_example_init()) {
        return false;
    }
    
    // ? 2. 檢查狀態
    flashEeprom_example_checkStatus();
    
    // ? 3. 嘗試讀取現有參數
    S_EEPROM_PARAM stParams;
    if (!flashEeprom_example_loadParams(&stParams)) {
        // ? 讀取失敗，載入預設值
        halEeprom_loadDefaults(&stParams);
        
        // ? 將預設值儲存到 Flash
        if (!flashEeprom_example_saveParams(&stParams)) {
            return false;
        }
    }
    
    // ? 4. 更新一些參數作為範例
    if (!flashEeprom_example_updateSingleParam(0, 8)) { // 設定輔助段數為 8
        return false;
    }
    
    if (!flashEeprom_example_updateSingleParam(1, 30)) { // 設定正轉限速為 30 km/h
        return false;
    }
    
    // ? 5. 驗證最終結果
    S_EEPROM_PARAM stFinalParams;
    if (!flashEeprom_example_loadParams(&stFinalParams)) {
        return false;
    }
    
    // ? 檢查更新是否成功
    if (stFinalParams.u8AssistLevel != 8 || stFinalParams.u16ForwardSpeedLimit != 30) {
        return false;
    }
    
    return true;
}

/**
 * @brief 範例：錯誤處理和恢復
 * @details 展示如何處理 Flash 操作錯誤並進行恢復
 * @return bool - true 表示恢復成功, false 表示恢復失敗
 */
bool flashEeprom_example_errorRecovery(void) {
    // ? 檢查最後一次操作的結果
    const S_FLASH_EEPROM_RESULT* pstLastResult = flashEeprom_getLastResult();
    
    if (pstLastResult == NULL) {
        return false;
    }
    
    // ? 根據錯誤狀態進行相應處理
    switch (pstLastResult->u8Status) {
        case FLASH_EEPROM_STATUS_SUCCESS:
            return true; // 操作成功，無需恢復
            
        case FLASH_EEPROM_STATUS_ERROR:
            // ? 一般錯誤，嘗試重新初始化
            return flashEeprom_example_init();
            
        case FLASH_EEPROM_STATUS_BUSY:
            // ? Flash 忙碌，等待完成
            if (flashEeprom_waitForCompletion(5000)) { // 5 秒超時
                return true;
            }
            return false;
            
        case FLASH_EEPROM_STATUS_INVALID:
            // ? 無效操作，檢查參數
            return false;
            
        case FLASH_EEPROM_STATUS_PROTECTED:
            // ? 寫入保護，嘗試解鎖
            // ? 這裡需要實作特定的解鎖邏輯
            return false;
            
        default:
            return false;
    }
}

// ============================================================================
// 整合測試函式
// ============================================================================

/**
 * @brief 整合測試：測試所有 Flash 模擬 EEPROM 功能
 * @details 執行完整的測試流程，驗證所有功能
 * @return bool - true 表示所有測試通過, false 表示有測試失敗
 */
bool flashEeprom_example_runAllTests(void) {
    bool bAllTestsPassed = true;
    
    // ? 測試 1：初始化
    if (!flashEeprom_example_init()) {
        bAllTestsPassed = false;
    }
    
    // ? 測試 2：基本讀寫
    if (bAllTestsPassed) {
        S_EEPROM_PARAM stTestParams = {0};
        stTestParams.u8AssistLevel = 10;
        stTestParams.u16ForwardSpeedLimit = 40;
        
        if (!flashEeprom_example_saveParams(&stTestParams)) {
            bAllTestsPassed = false;
        }
        
        if (bAllTestsPassed) {
            S_EEPROM_PARAM stReadParams = {0};
            if (!flashEeprom_example_loadParams(&stReadParams)) {
                bAllTestsPassed = false;
            } else if (stReadParams.u8AssistLevel != 10 || stReadParams.u16ForwardSpeedLimit != 40) {
                bAllTestsPassed = false;
            }
        }
    }
    
    // ? 測試 3：單一參數更新
    if (bAllTestsPassed) {
        if (!flashEeprom_example_updateSingleParam(2, 15)) { // 設定逆轉限速為 15
            bAllTestsPassed = false;
        }
    }
    
    // ? 測試 4：狀態檢查
    if (bAllTestsPassed) {
        flashEeprom_example_checkStatus();
    }
    
    // ? 測試 5：完整工作流程
    if (bAllTestsPassed) {
        if (!flashEeprom_example_completeWorkflow()) {
            bAllTestsPassed = false;
        }
    }
    
    return bAllTestsPassed;
}
