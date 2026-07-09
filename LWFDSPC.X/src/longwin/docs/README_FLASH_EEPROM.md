# Flash 模擬 EEPROM 模組使用說明

## 概述

這個模組提供了使用 dsPIC33CK256MP506 的 Flash 資料記憶體來模擬 EEPROM 功能。由於 dsPIC33CK 沒有內建 EEPROM，我們使用 8KB 的 Flash 資料記憶體來實現類似的功能。

## 檔案結構

```
src/longwin/
├── s_flash_eeprom.h          # Flash 模擬 EEPROM 標頭文件
├── s_flash_eeprom.c          # Flash 模擬 EEPROM 實作文件
├── s_flash_eeprom_example.c  # 使用範例和測試
└── README_FLASH_EEPROM.md    # 本說明文件
```

## 主要特性

### 1. **Flash 記憶體管理**
- 使用 8KB Flash 資料記憶體
- 512 位元組頁面大小
- 自動頁面擦除和寫入管理

### 2. **資料完整性保護**
- 校驗和驗證
- 參數備份機制
- 中繼資料管理

### 3. **磨損平衡**
- 自動選擇磨損最少的頁面
- 延長 Flash 記憶體壽命
- 磨損統計追蹤

### 4. **錯誤處理和恢復**
- 詳細的錯誤狀態碼
- 自動錯誤恢復機制
- 備份資料恢復

## 硬體配置

### Flash 記憶體配置
```c
#define FLASH_EEPROM_START_ADDRESS     0x800000    // Flash 資料記憶體起始位址
#define FLASH_EEPROM_SIZE             8192        // 8KB Flash 資料記憶體
#define FLASH_EEPROM_PAGE_SIZE        512         // Flash 頁面大小
#define FLASH_EEPROM_TOTAL_PAGES      16          // 總頁面數
```

### 記憶體佈局
```
0x800000 - 0x8001FF: 參數儲存區域 (512 bytes)
0x800200 - 0x8003FF: 參數備份區域 (512 bytes)
0x800400 - 0x8005FF: 中繼資料區域 (512 bytes)
0x800600 - 0x801FFF: 可用空間 (6.5KB)
```

## 基本使用方法

### 1. **初始化模組**
```c
#include "s_flash_eeprom.h"

// 初始化 Flash 模擬 EEPROM
if (!flashEeprom_init()) {
    // 處理初始化失敗
    if (!flashEeprom_format()) {
        // 格式化也失敗，需要檢查硬體
        return false;
    }
    // 重新嘗試初始化
    flashEeprom_init();
}
```

### 2. **讀取資料**
```c
uint8_t au8Buffer[32];
uint32_t u32Address = 0x800000;

if (flashEeprom_read(u32Address, au8Buffer, 32)) {
    // 讀取成功
} else {
    // 讀取失敗，檢查錯誤狀態
    const S_FLASH_EEPROM_RESULT* pstResult = flashEeprom_getLastResult();
    // 根據錯誤狀態進行處理
}
```

### 3. **寫入資料**
```c
uint8_t au8Data[32] = {0x01, 0x02, 0x03, ...};
uint32_t u32Address = 0x800000;

if (flashEeprom_write(u32Address, au8Data, 32)) {
    // 寫入成功
} else {
    // 寫入失敗，檢查錯誤狀態
}
```

### 4. **檢查狀態**
```c
// 檢查記憶體狀態
if (!flashEeprom_checkMemoryStatus()) {
    // 記憶體有問題，可能需要格式化
    flashEeprom_format();
}

// 取得空間資訊
uint32_t u32Total, u32Used, u32Free;
flashEeprom_getSpaceInfo(&u32Total, &u32Used, &u32Free);
```

## 進階功能

### 1. **參數備份和恢復**
```c
// 建立備份
if (flashEeprom_createBackup(au8Data, 32)) {
    // 備份成功
}

// 從備份恢復
if (flashEeprom_restoreFromBackup(au8Data, 32)) {
    // 恢復成功
}
```

### 2. **磨損平衡管理**
```c
// 執行磨損平衡
if (flashEeprom_wearLeveling()) {
    // 磨損平衡成功
}

// 取得磨損統計
uint16_t au16WearCounts[16];
flashEeprom_getWearStatistics(au16WearCounts);
```

### 3. **等待操作完成**
```c
// 等待 Flash 操作完成，最多等待 5 秒
if (flashEeprom_waitForCompletion(5000)) {
    // 操作完成
} else {
    // 超時
}
```

## 與現有 EEPROM 模組整合

### 1. **修改 s_hal_eeprom.c**
在現有的 EEPROM 模組中，您可以呼叫 Flash 模擬 EEPROM 函式：

```c
// 在 halEeprom_read 中
bool halEeprom_read(S_EEPROM_PARAM *pDest) {
    if (pDest == NULL) {
        return false;
    }
    
    // 使用 Flash 模擬 EEPROM 讀取
    uint32_t u32Address = 0x800000; // Flash 參數區域
    return flashEeprom_read(u32Address, (uint8_t*)pDest, sizeof(S_EEPROM_PARAM));
}

// 在 halEeprom_write 中
bool halEeprom_write(const S_EEPROM_PARAM *pSrc) {
    if (pSrc == NULL) {
        return false;
    }
    
    // 使用 Flash 模擬 EEPROM 寫入
    uint32_t u32Address = 0x800000; // Flash 參數區域
    return flashEeprom_write(u32Address, (uint8_t*)pSrc, sizeof(S_EEPROM_PARAM));
}
```

### 2. **初始化順序**
```c
void system_init(void) {
    // 1. 初始化 Flash 模擬 EEPROM
    if (!flashEeprom_init()) {
        // 處理初始化失敗
        return;
    }
    
    // 2. 初始化 EEPROM 模組
    if (!halEeprom_initWithValidation()) {
        // 處理初始化失敗
        return;
    }
    
    // 系統初始化完成
}
```

## 錯誤處理

### 錯誤狀態碼
```c
#define FLASH_EEPROM_STATUS_SUCCESS    0    // 操作成功
#define FLASH_EEPROM_STATUS_ERROR     1    // 一般錯誤
#define FLASH_EEPROM_STATUS_BUSY      2    // Flash 忙碌中
#define FLASH_EEPROM_STATUS_INVALID   3    // 無效操作
#define FLASH_EEPROM_STATUS_PROTECTED 4    // 寫入保護
```

### 錯誤處理範例
```c
const S_FLASH_EEPROM_RESULT* pstResult = flashEeprom_getLastResult();

switch (pstResult->u8Status) {
    case FLASH_EEPROM_STATUS_SUCCESS:
        // 操作成功
        break;
        
    case FLASH_EEPROM_STATUS_BUSY:
        // 等待操作完成
        flashEeprom_waitForCompletion(1000);
        break;
        
    case FLASH_EEPROM_STATUS_ERROR:
        // 嘗試重新初始化
        flashEeprom_init();
        break;
        
    case FLASH_EEPROM_STATUS_PROTECTED:
        // 檢查寫入保護設定
        break;
        
    default:
        // 未知錯誤
        break;
}
```

## 注意事項

### 1. **Flash 寫入限制**
- Flash 只能從 1 寫入 0，不能從 0 寫入 1
- 寫入前必須先擦除整個頁面
- 寫入操作需要時間，必須等待完成

### 2. **記憶體壽命**
- Flash 記憶體有寫入次數限制（通常 10,000-100,000 次）
- 使用磨損平衡算法延長壽命
- 避免頻繁寫入相同位置

### 3. **電源管理**
- Flash 操作期間不能斷電
- 建議在穩定的電源條件下進行寫入操作
- 考慮使用 UPS 或備用電源

### 4. **中斷處理**
- Flash 操作期間可能影響中斷響應
- 建議在關鍵操作期間暫時禁用中斷
- 使用中斷驅動的等待機制

## 測試和驗證

### 1. **執行測試**
```c
#include "s_flash_eeprom_example.h"

// 執行所有測試
if (flashEeprom_example_runAllTests()) {
    // 所有測試通過
} else {
    // 有測試失敗，檢查硬體
}
```

### 2. **單元測試**
```c
// 測試基本功能
if (!flashEeprom_example_init()) {
    // 初始化測試失敗
}

// 測試讀寫功能
if (!flashEeprom_example_completeWorkflow()) {
    // 讀寫測試失敗
}
```

## 效能考量

### 1. **讀取效能**
- 讀取操作很快，幾乎沒有延遲
- 適合頻繁讀取的應用

### 2. **寫入效能**
- 寫入操作較慢，需要擦除和寫入時間
- 頁面擦除：約 2-5ms
- 字寫入：約 10-50μs

### 3. **記憶體使用**
- 每個參數結構：32 bytes
- 中繼資料：約 32 bytes
- 總開銷：約 100 bytes

## 未來改進

### 1. **硬體特定優化**
- 實作 dsPIC33CK 特定的 Flash 控制器操作
- 優化時脈設定和暫存器配置
- 加入硬體錯誤檢測

### 2. **進階功能**
- 支援不同資料類型的壓縮
- 實作更複雜的磨損平衡算法
- 加入資料加密功能

### 3. **效能優化**
- 使用 DMA 進行大量資料傳輸
- 實作快取機制減少 Flash 存取
- 優化頁面管理策略

## 技術支援

如果您在使用過程中遇到問題，請檢查：

1. **硬體連接**：確保 Flash 記憶體正常供電
2. **時脈設定**：檢查 Flash 控制器時脈配置
3. **記憶體映射**：確認位址範圍正確
4. **錯誤日誌**：查看詳細的錯誤狀態資訊

## 結語

這個 Flash 模擬 EEPROM 模組提供了完整的解決方案，讓您可以在沒有內建 EEPROM 的 dsPIC33CK 微控制器上實現可靠的參數儲存功能。通過適當的使用和維護，這個模組可以為您的應用提供長期的資料儲存支援。
