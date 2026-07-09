#ifndef S_EEPROM_H
#define S_EEPROM_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief EEPROM 參數結構體
 * @details 根據 longwinConrtrolFunction_chunchi.md 文件定義的 10 個主要配置參數
 */
typedef struct {
    // 主要配置參數
    uint8_t u8AssistLevel;              // 輔助段數 0-15
    uint16_t u16ForwardSpeedLimit;      // 正轉限速 (km/h)
    uint16_t u16ReverseSpeedLimit;      // 逆轉限速 (km/h)
    uint8_t u8SystemVoltage;            // 控制系統電壓 24V/36V/48V
    uint16_t u16MaxCurrent;             // 最大電流
    uint8_t u8AccelCurveGroup;          // 加速曲線參數組 0-5
    uint8_t u8MotorHallSequence;        // 馬達Hall相序表 1-12
    uint8_t u8MotorDirectionReverse;    // 馬達方向反轉 0/1
    uint16_t u16WheelPulsePerRev;       // 輪徑一圈幾pulse
    uint32_t u32TotalOdometer;          // 總行程ODO
    
    // 校驗和
    uint16_t u16Checksum;               // 資料校驗和
} S_EEPROM_PARAM;

/**
 * @brief 預設 EEPROM 參數值
 * @details 提供系統初始化的預設參數設定
 */
extern const S_EEPROM_PARAM g_stEepromDefaultParams;

/**
 * @brief 從 EEPROM 讀取參數。
 * @details 此函式會讀取 EEPROM 中儲存的參數資料。
 * @param pDest (輸出) 指向要存放讀取資料的結構指標。
 * @return bool - true 表示讀取成功, false 表示失敗。
 */
bool s_eeprom_read(S_EEPROM_PARAM *pDest);

/**
 * @brief 將參數寫入到 EEPROM。
 * @details 此函式會將參數結構寫入 EEPROM 儲存。
 * @param pSrc (輸入) 指向要寫入的參數結構的指標。
 * @return bool - true 表示寫入成功, false 表示失敗。
 */
bool s_eeprom_write(const S_EEPROM_PARAM *pSrc);

/**
 * @brief 將預設參數載入到指定的結構中。
 * @details 當 EEPROM 讀取失敗或需要恢復預設值時使用。
 * @param pDest (輸出) 指向要存放預設資料的結構指標。
 */
void s_eeprom_loadDefaults(S_EEPROM_PARAM *pDest);

/**
 * @brief 更新單一參數值到 EEPROM。
 * @details 此函式會讀取現有資料，更新指定參數後寫入。
 * @param u8ParamIndex 要更新的參數索引 (0-9)
 * @param u32NewValue 新的參數值
 * @return bool - true 表示更新成功, false 表示失敗
 */
bool s_eeprom_updateSingleParam(uint8_t u8ParamIndex, uint32_t u32NewValue);

/**
 * @brief 強制重新寫入所有參數到 EEPROM。
 * @details 此函式會忽略現有資料，直接將新的參數結構寫入 EEPROM。
 * @param pSrc (輸入) 指向要寫入的參數結構的指標
 * @return bool - true 表示寫入成功, false 表示失敗
 */
bool s_eeprom_forceWriteAll(const S_EEPROM_PARAM *pSrc);

/**
 * @brief 重置 EEPROM 到預設值。
 * @details 此函式會將 EEPROM 中的所有參數重置為預設值。
 * @return bool - true 表示重置成功, false 表示重置失敗
 * @note 此操作會清除所有自定義設定，請謹慎使用
 */
bool s_eeprom_resetToDefaults(void);

/**
 * @brief 初始化 EEPROM 模組並進行錯誤檢查
 * @details 開機時檢查 EEPROM 資料完整性，錯誤時自動恢復
 * @return bool - true 表示初始化成功, false 表示需要手動處理
 */
bool s_eeprom_initWithValidation(void);

/**
 * @brief 驗證 EEPROM 資料完整性
 * @details 檢查資料範圍、格式和一致性
 * @param pstParams 指向要驗證的參數結構指標
 * @return bool - true 表示資料有效, false 表示資料損壞
 */
bool s_eeprom_validateData(const S_EEPROM_PARAM *pstParams);

/**
 * @brief 從 EEPROM 讀取並驗證資料
 * @details 讀取後自動驗證資料完整性，失敗時可選擇恢復預設值
 * @param pDest (輸出) 指向要存放讀取資料的結構指標
 * @param bAutoRecovery 是否啟用自動恢復 (true: 驗證失敗時載入預設值)
 * @return bool - true 表示讀取並驗證成功, false 表示失敗
 */
bool s_eeprom_readWithValidation(S_EEPROM_PARAM *pDest, bool bAutoRecovery);

/**
 * @brief 寫入 EEPROM 並驗證寫入結果
 * @details 寫入後讀取驗證，確保資料正確寫入
 * @param pSrc (輸入) 指向要寫入的參數結構的指標
 * @return bool - true 表示寫入並驗證成功, false 表示失敗
 */
bool s_eeprom_writeWithVerification(const S_EEPROM_PARAM *pSrc);

/**
 * @brief 檢查 EEPROM 硬體狀態
 * @details 檢查 EEPROM 硬體是否正常運作
 * @return bool - true 表示硬體正常, false 表示硬體故障
 */
bool s_eeprom_checkHardwareStatus(void);

/**
 * @brief 取得 EEPROM 錯誤狀態
 * @details 回傳詳細的錯誤狀態資訊
 * @return uint8_t - 錯誤狀態碼 (0: 正常, 1: 資料損壞, 2: 硬體故障, 3: 寫入失敗)
 */
uint8_t s_eeprom_getErrorStatus(void);

/**
 * @brief 計算參數結構的校驗和
 * @details 計算除校驗和欄位外的所有欄位的校驗和值
 * @param pstParams 指向要計算校驗和的參數結構指標
 * @return uint16_t - 計算出的校驗和值
 */
uint16_t s_eeprom_calculateChecksum(const S_EEPROM_PARAM *pstParams);

/**
 * @brief 驗證參數結構的校驗和
 * @details 驗證參數結構的校驗和是否正確
 * @param pstParams 指向要驗證的參數結構指標
 * @return bool - true 表示校驗和正確, false 表示校驗和錯誤
 */
bool s_eeprom_verifyChecksum(const S_EEPROM_PARAM *pstParams);

/**
 * @brief 更新參數結構的校驗和
 * @details 計算並更新參數結構的校驗和欄位
 * @param pstParams 指向要更新校驗和的參數結構指標
 */
void s_eeprom_updateChecksum(S_EEPROM_PARAM *pstParams);

#endif // S_HAL_EEPROM_H
