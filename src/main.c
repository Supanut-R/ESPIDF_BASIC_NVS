#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#define TAG "NVS"
nvs_handle_t my_handler;

void app_main() {
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) { // ครั้งแรกพยายามเช็คก่อนว่า initial ได้ไหม หรือ เคยมีการใช้ NVS มาก่อนไหม
        ESP_ERROR_CHECK(nvs_flash_erase()); // ถ้า flash เรามีปัญหาจริงๆ จะติดตั้งแต่บรรทัดนี้ละ
        ret = nvs_flash_init();
    }
    // ret = ESP_FAIL
    // ESP_ERROR_CHECK(ret); // สมมติเหตุการณ์ขึ้นมาว่ามีข้อผิดพลาด
    ESP_ERROR_CHECK(ret); // เช็คว่าค่าที่ return มาเป็นค่าที่เป็น error มั๊ย ถ้าเป็น error ทำให้ initialize ไม่ได้ เราจะไม่ยอมให้มันเข้าไปรันใน app เรา โดยให้มัน reboot ตัวมันเอง
                          // คือมันจะเข้า boot loop

    ret = nvs_open("storage", NVS_READWRITE, &my_handler); // เมื่อ initial ผ่าน ก็ต้องมา check อีกว่า เปิดการใช้งาน storage NVS ได้ไหม #ไม่จำเป็นต้องเป็นชื่อ storage ชื่ออื่นก็ได้
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret)); //ถ้าเปิดไม่ติดจะแสดง error ออกมา
    } else {
        ESP_LOGI(TAG, "Done");
        // Write
        int32_t counter = 0;
        int32_t type;
        ret = nvs_find_key(my_handler, "counter", &type);
        ESP_ERROR_CHECK(ret);
        if (ret == ESP_OK) {
            ESP_ERROR_CHECK(nvs_get_i32(my_handler, "counter", &counter));
        }
        while(true) {
            counter++;
            printf("Updating counter in NVS ... ");
            ret = nvs_set_i32(my_handler, "counter", counter); // key = "counter"
            printf((ret != ESP_OK) ? "Failed!\n" : "Done\n");
            if (counter > 10) {
                break;
            }
        
        }
        printf("Committing updates in NVS ... ");
        ret = nvs_commit(my_handler); // commit ปิดท้าย เพื่อ save
        printf((ret != ESP_OK) ? "Failed!\n" : "Done\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // Read
        printf("Reading counter from NVS ... ");
        ret = nvs_get_i32(my_handler, "counter", &counter);
        switch (ret) {
            case ESP_OK:
                printf("Done\n");
                printf("Restart counter = %" PRIu32 "\n", counter);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(ret));
        }
        
    }
    
}