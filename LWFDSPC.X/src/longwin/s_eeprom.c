#include "s_eeprom.h"
#include <stddef.h> // For NULL

// 手動記憶體複製函式
static void _copyMemory(void *pDest, const void *pSrc, size_t uSize)
{
    uint8_t *pu8Dest = (uint8_t *)pDest;
    const uint8_t *pu8Src = (const uint8_t *)pSrc;
    for (size_t i = 0; i < uSize; i++)
    {
        pu8Dest[i] = pu8Src[i];
    }
}

// 預設 EEPROM 參數值
const S_EEPROM_PARAM g_stEepromDefaultParams = {
    .u8AssistLevel = 5,           // 輔助段數 0-15，預設為 5
    .u16ForwardSpeedLimit = 25,   // 正轉限速 (km/h)，預設為 25
    .u16ReverseSpeedLimit = 10,   // 逆轉限速 (km/h)，預設為 10
    .u8SystemVoltage = 48,        // 控制系統電壓 24V/36V/48V，預設為 48V
    .u16MaxCurrent = 15,          // 最大電流，預設為 15A
    .u8AccelCurveGroup = 0,       // 加速曲線參數組 0-5，預設為 0 (開放自定)
    .u8MotorHallSequence = 1,     // 馬達Hall相序表 1-12，預設為 1
    .u8MotorDirectionReverse = 0, // 馬達方向反轉 0/1，預設為 0 (不變動)
    .u16WheelPulsePerRev = 24,    // 輪徑一圈幾pulse，預設為 24
    .u32TotalOdometer = 0,        // 總行程ODO，預設為 0
    .u16Checksum = 0              // 校驗和，初始化時設為 0
};

// 靜態變數：當前載入的 EEPROM 參數
static S_EEPROM_PARAM s_stCurrentEepromParams;

/**
 * @brief 從 EEPROM 讀取參數
 * @details 此函式會讀取 EEPROM 中儲存的參數資料
 * @param pDest (輸出) 指向要存放讀取資料的結構指標
 * @return bool - true 表示讀取成功, false 表示失敗
 */
bool s_eeprom_read(S_EEPROM_PARAM *pDest)
{
    if (pDest == NULL)
    {
        return false;
    }

    // ? 這裡需要實作實際的 EEPROM 硬體讀取邏輯
    // ? 目前先回傳當前載入的參數作為模擬
    _copyMemory(pDest, &s_stCurrentEepromParams, sizeof(S_EEPROM_PARAM));

    // ? 實際實作時應該從硬體 EEPROM 讀取
    // ? 並進行資料驗證
    return true;
}

/**
 * @brief 將參數寫入到 EEPROM
 * @details 此函式會將參數結構寫入 EEPROM 儲存
 * @param pSrc (輸入) 指向要寫入的參數結構的指標
 * @return bool - true 表示寫入成功, false 表示失敗
 */
bool s_eeprom_write(const S_EEPROM_PARAM *pSrc)
{
    if (pSrc == NULL)
    {
        return false;
    }

    // ? 這裡需要實作實際的 EEPROM 硬體寫入邏輯
    // ? 目前先更新內部變數作為模擬

    // ? 將傳入的參數複製到模組內部儲存區
    _copyMemory(&s_stCurrentEepromParams, pSrc, sizeof(S_EEPROM_PARAM));

    // ? 更新校驗和
    s_eeprom_updateChecksum(&s_stCurrentEepromParams);

    // ? 實際實作時應該寫入硬體 EEPROM
    // ? 並進行寫入驗證

    return true;
}

/**
 * @brief 將預設參數載入到指定的結構中
 * @details 當 EEPROM 讀取失敗或需要恢復預設值時使用
 * @param pDest (輸出) 指向要存放預設資料的結構指標
 */
void s_eeprom_loadDefaults(S_EEPROM_PARAM *pDest)
{
    if (pDest == NULL)
    {
        return;
    }

    // ? 將預設參數複製到目標結構
    _copyMemory(pDest, &g_stEepromDefaultParams, sizeof(S_EEPROM_PARAM));

    // ? 更新預設參數的校驗和
    s_eeprom_updateChecksum(pDest);
}

/**
 * @brief 更新單一參數值到 EEPROM
 * @details 此函式會讀取現有資料，更新指定參數後寫入
 * @param u8ParamIndex 要更新的參數索引 (0-9)
 * @param u32NewValue 新的參數值
 * @return bool - true 表示更新成功, false 表示失敗
 */
bool s_eeprom_updateSingleParam(uint8_t u8ParamIndex, uint32_t u32NewValue)
{
    // ? 檢查參數索引範圍
    if (u8ParamIndex >= 10)
    {
        return false;
    }

    // ? 讀取當前 EEPROM 參數
    S_EEPROM_PARAM stTempParams;
    if (!s_eeprom_read(&stTempParams))
    {
        return false;
    }

    // ? 根據索引更新對應的參數
    switch (u8ParamIndex)
    {
    case 0: // 輔助段數
        if (u32NewValue <= 15)
        {
            stTempParams.u8AssistLevel = (uint8_t)u32NewValue;
        }
        else
        {
            return false;
        }
        break;
    case 1: // 正轉限速
        stTempParams.u16ForwardSpeedLimit = (uint16_t)u32NewValue;
        break;
    case 2: // 逆轉限速
        stTempParams.u16ReverseSpeedLimit = (uint16_t)u32NewValue;
        break;
    case 3: // 控制系統電壓
        if (u32NewValue == 24 || u32NewValue == 36 || u32NewValue == 48)
        {
            stTempParams.u8SystemVoltage = (uint8_t)u32NewValue;
        }
        else
        {
            return false;
        }
        break;
    case 4: // 最大電流
        stTempParams.u16MaxCurrent = (uint16_t)u32NewValue;
        break;
    case 5: // 加速曲線參數組
        if (u32NewValue <= 5)
        {
            stTempParams.u8AccelCurveGroup = (uint8_t)u32NewValue;
        }
        else
        {
            return false;
        }
        break;
    case 6: // 馬達Hall相序表
        if (u32NewValue >= 1 && u32NewValue <= 12)
        {
            stTempParams.u8MotorHallSequence = (uint8_t)u32NewValue;
        }
        else
        {
            return false;
        }
        break;
    case 7: // 馬達方向反轉
        if (u32NewValue <= 1)
        {
            stTempParams.u8MotorDirectionReverse = (uint8_t)u32NewValue;
        }
        else
        {
            return false;
        }
        break;
    case 8: // 輪徑一圈幾pulse
        stTempParams.u16WheelPulsePerRev = (uint16_t)u32NewValue;
        break;
    case 9: // 總行程ODO
        stTempParams.u32TotalOdometer = u32NewValue;
        break;
    default:
        return false;
    }

    // ? 更新校驗和
    s_eeprom_updateChecksum(&stTempParams);

    // ? 將更新後的參數寫入 EEPROM
    return s_eeprom_write(&stTempParams);
}

/**
 * @brief 強制重新寫入所有參數到 EEPROM
 * @details 此函式會忽略現有資料，直接將新的參數結構寫入 EEPROM
 * @param pSrc (輸入) 指向要寫入的參數結構的指標
 * @return bool - true 表示寫入成功, false 表示失敗
 */
bool s_eeprom_forceWriteAll(const S_EEPROM_PARAM *pSrc)
{
    if (pSrc == NULL)
    {
        return false;
    }

    // ? 直接寫入新的參數結構，不進行讀取驗證
    return s_eeprom_write(pSrc);
}

/**
 * @brief 重置 EEPROM 到預設值
 * @details 此函式會將 EEPROM 中的所有參數重置為預設值
 * @return bool - true 表示重置成功, false 表示重置失敗
 * @note 此操作會清除所有自定義設定，請謹慎使用
 */
bool s_eeprom_resetToDefaults(void)
{
    // ? 將預設參數寫入 EEPROM
    return s_eeprom_write(&g_stEepromDefaultParams);
}

// 靜態變數：錯誤狀態
static uint8_t s_u8ErrorStatus = 0; // 0: 正常, 1: 資料損壞, 2: 硬體故障, 3: 寫入失敗

/**
 * @brief 驗證 EEPROM 資料完整性
 * @details 檢查資料範圍、格式和一致性
 * @param pstParams 指向要驗證的參數結構指標
 * @return bool - true 表示資料有效, false 表示資料損壞
 */
bool s_eeprom_validateData(const S_EEPROM_PARAM *pstParams)
{
    if (pstParams == NULL)
    {
        return false;
    }

    // ? 檢查輔助段數範圍 (0-15)
    if (pstParams->u8AssistLevel > 15)
    {
        return false;
    }

    // ? 檢查速度限制合理性 (0-100 km/h)
    if (pstParams->u16ForwardSpeedLimit > 100 || pstParams->u16ReverseSpeedLimit > 100)
    {
        return false;
    }

    // ? 檢查系統電壓 (只允許 24V, 36V, 48V)
    if (pstParams->u8SystemVoltage != 24 && pstParams->u8SystemVoltage != 36 && pstParams->u8SystemVoltage != 48)
    {
        return false;
    }

    // ? 檢查最大電流合理性 (1-100A)
    if (pstParams->u16MaxCurrent < 1 || pstParams->u16MaxCurrent > 100)
    {
        return false;
    }

    // ? 檢查加速曲線參數組 (0-5)
    if (pstParams->u8AccelCurveGroup > 5)
    {
        return false;
    }

    // ? 檢查馬達Hall相序表 (1-12)
    if (pstParams->u8MotorHallSequence < 1 || pstParams->u8MotorHallSequence > 12)
    {
        return false;
    }

    // ? 檢查馬達方向反轉 (0-1)
    if (pstParams->u8MotorDirectionReverse > 1)
    {
        return false;
    }

    // ? 檢查輪徑脈衝數合理性 (1-1000)
    if (pstParams->u16WheelPulsePerRev < 1 || pstParams->u16WheelPulsePerRev > 1000)
    {
        return false;
    }

    // ? 檢查總行程ODO合理性 (0-999999 km)
    if (pstParams->u32TotalOdometer > 999999)
    {
        return false;
    }

    return true;
}

/**
 * @brief 檢查 EEPROM 硬體狀態
 * @details 檢查 EEPROM 硬體是否正常運作
 * @return bool - true 表示硬體正常, false 表示硬體故障
 */
bool s_eeprom_checkHardwareStatus(void)
{
    // ? 這裡需要實作實際的硬體狀態檢查
    // ? 例如：檢查電源電壓、通信狀態等

    // ? 目前先假設硬體正常
    // ? 實際實作時應該檢查：
    // ? 1. EEPROM 電源電壓
    // ? 2. SPI/I2C 通信狀態
    // ? 3. 寫入保護狀態
    // ? 4. 溫度狀態等

    return true;
}

/**
 * @brief 從 EEPROM 讀取並驗證資料
 * @details 讀取後自動驗證資料完整性，失敗時可選擇恢復預設值
 * @param pDest (輸出) 指向要存放讀取資料的結構指標
 * @param bAutoRecovery 是否啟用自動恢復 (true: 驗證失敗時載入預設值)
 * @return bool - true 表示讀取並驗證成功, false 表示失敗
 */
bool s_eeprom_readWithValidation(S_EEPROM_PARAM *pDest, bool bAutoRecovery)
{
    if (pDest == NULL)
    {
        s_u8ErrorStatus = 1; // 資料損壞
        return false;
    }

    // ? 檢查硬體狀態
    if (!s_eeprom_checkHardwareStatus())
    {
        s_u8ErrorStatus = 2; // 硬體故障
        return false;
    }

    // ? 從 EEPROM 讀取資料
    if (!s_eeprom_read(pDest))
    {
        s_u8ErrorStatus = 1; // 資料損壞
        if (bAutoRecovery)
        {
            // ? 自動恢復：載入預設值
            s_eeprom_loadDefaults(pDest);
            s_u8ErrorStatus = 0; // 恢復成功，清除錯誤狀態
            return true;
        }
        return false;
    }

    // ? 驗證讀取的資料
    if (!s_eeprom_validateData(pDest))
    {
        s_u8ErrorStatus = 1; // 資料損壞
        if (bAutoRecovery)
        {
            // ? 自動恢復：載入預設值
            s_eeprom_loadDefaults(pDest);
            s_u8ErrorStatus = 0; // 恢復成功，清除錯誤狀態
            return true;
        }
        return false;
    }

    // ? 驗證校驗和
    if (!s_eeprom_verifyChecksum(pDest))
    {
        s_u8ErrorStatus = 1; // 資料損壞 (校驗和錯誤)
        if (bAutoRecovery)
        {
            // ? 自動恢復：載入預設值
            s_eeprom_loadDefaults(pDest);
            s_u8ErrorStatus = 0; // 恢復成功，清除錯誤狀態
            return true;
        }
        return false;
    }

    // ? 資料驗證成功
    s_u8ErrorStatus = 0; // 清除錯誤狀態
    return true;
}

/**
 * @brief 寫入 EEPROM 並驗證寫入結果
 * @details 寫入後讀取驗證，確保資料正確寫入
 * @param pSrc (輸入) 指向要寫入的參數結構的指標
 * @return bool - true 表示寫入並驗證成功, false 表示失敗
 */
bool s_eeprom_writeWithVerification(const S_EEPROM_PARAM *pSrc)
{
    if (pSrc == NULL)
    {
        s_u8ErrorStatus = 1; // 資料損壞
        return false;
    }

    // ? 檢查硬體狀態
    if (!s_eeprom_checkHardwareStatus())
    {
        s_u8ErrorStatus = 2; // 硬體故障
        return false;
    }

    // ? 驗證要寫入的資料
    if (!s_eeprom_validateData(pSrc))
    {
        s_u8ErrorStatus = 1; // 資料損壞
        return false;
    }

    // ? 創建臨時結構並更新校驗和
    S_EEPROM_PARAM stTempParams = *pSrc;
    s_eeprom_updateChecksum(&stTempParams);

    // ? 寫入 EEPROM (使用帶有校驗和的臨時結構)
    if (!s_eeprom_write(&stTempParams))
    {
        s_u8ErrorStatus = 3; // 寫入失敗
        return false;
    }

    // ? 讀取驗證寫入結果
    S_EEPROM_PARAM stReadBack;
    if (!s_eeprom_read(&stReadBack))
    {
        s_u8ErrorStatus = 3; // 寫入失敗
        return false;
    }

    // ? 比較寫入和讀取的資料 (包含校驗和)
    if (stReadBack.u8AssistLevel != stTempParams.u8AssistLevel ||
        stReadBack.u16ForwardSpeedLimit != stTempParams.u16ForwardSpeedLimit ||
        stReadBack.u16ReverseSpeedLimit != stTempParams.u16ReverseSpeedLimit ||
        stReadBack.u8SystemVoltage != stTempParams.u8SystemVoltage ||
        stReadBack.u16MaxCurrent != stTempParams.u16MaxCurrent ||
        stReadBack.u8AccelCurveGroup != stTempParams.u8AccelCurveGroup ||
        stReadBack.u8MotorHallSequence != stTempParams.u8MotorHallSequence ||
        stReadBack.u8MotorDirectionReverse != stTempParams.u8MotorDirectionReverse ||
        stReadBack.u16WheelPulsePerRev != stTempParams.u16WheelPulsePerRev ||
        stReadBack.u32TotalOdometer != stTempParams.u32TotalOdometer ||
        stReadBack.u16Checksum != stTempParams.u16Checksum)
    {

        s_u8ErrorStatus = 3; // 寫入失敗
        return false;
    }

    // ? 寫入驗證成功
    s_u8ErrorStatus = 0; // 清除錯誤狀態
    return true;
}

/**
 * @brief 初始化 EEPROM 模組並進行錯誤檢查
 * @details 開機時檢查 EEPROM 資料完整性，錯誤時自動恢復
 * @return bool - true 表示初始化成功, false 表示需要手動處理
 */
bool s_eeprom_initWithValidation(void)
{
    // ? 檢查硬體狀態
    if (!s_eeprom_checkHardwareStatus())
    {
        s_u8ErrorStatus = 2; // 硬體故障
        return false;
    }

    // ? 嘗試讀取並驗證 EEPROM 資料，啟用自動恢復
    if (s_eeprom_readWithValidation(&s_stCurrentEepromParams, true))
    {
        // ? 初始化成功
        s_u8ErrorStatus = 0;
        return true;
    }
    else
    {
        // ? 初始化失敗，需要手動處理
        return false;
    }
}

/**
 * @brief 取得 EEPROM 錯誤狀態
 * @details 回傳詳細的錯誤狀態資訊
 * @return uint8_t - 錯誤狀態碼 (0: 正常, 1: 資料損壞, 2: 硬體故障, 3: 寫入失敗)
 */
uint8_t s_eeprom_getErrorStatus(void)
{
    return s_u8ErrorStatus;
}

/**
 * @brief 計算參數結構的校驗和
 * @details 計算除校驗和欄位外的所有欄位的校驗和值
 * @param pstParams 指向要計算校驗和的參數結構指標
 * @return uint16_t - 計算出的校驗和值
 */
uint16_t s_eeprom_calculateChecksum(const S_EEPROM_PARAM *pstParams)
{
    if (pstParams == NULL)
    {
        return 0;
    }

    uint16_t u16Checksum = 0;
    const uint8_t *pu8Data = (const uint8_t *)pstParams;

    // ? 計算除校驗和欄位外的所有欄位的校驗和
    // ? 校驗和欄位位於結構體的最後 2 個位元組
    for (size_t i = 0; i < (sizeof(S_EEPROM_PARAM) - sizeof(uint16_t)); i++)
    {
        u16Checksum += pu8Data[i];
    }

    // ? 使用簡單的加法校驗和算法
    // ? 可以根據需要改用更複雜的 CRC 算法
    return u16Checksum;
}

/**
 * @brief 驗證參數結構的校驗和
 * @details 驗證參數結構的校驗和是否正確
 * @param pstParams 指向要驗證的參數結構指標
 * @return bool - true 表示校驗和正確, false 表示校驗和錯誤
 */
bool s_eeprom_verifyChecksum(const S_EEPROM_PARAM *pstParams)
{
    if (pstParams == NULL)
    {
        return false;
    }

    // ? 計算當前資料的校驗和
    uint16_t u16CalculatedChecksum = s_eeprom_calculateChecksum(pstParams);

    // ? 比較計算出的校驗和與儲存的校驗和
    return (u16CalculatedChecksum == pstParams->u16Checksum);
}

/**
 * @brief 更新參數結構的校驗和
 * @details 計算並更新參數結構的校驗和欄位
 * @param pstParams 指向要更新校驗和的參數結構指標
 */
void s_eeprom_updateChecksum(S_EEPROM_PARAM *pstParams)
{
    if (pstParams == NULL)
    {
        return;
    }

    // ? 計算新的校驗和
    uint16_t u16NewChecksum = s_eeprom_calculateChecksum(pstParams);

    // ? 更新校驗和欄位
    pstParams->u16Checksum = u16NewChecksum;
}
