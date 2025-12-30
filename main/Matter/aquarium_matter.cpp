/**
 * @file aquarium_matter.cpp
 * @brief Matter/HomeKit Temperature Sensor Implementation
 * 
 * ESP-Matter compatible temperature sensor for Apple HomeKit
 * Manufacturer: Claude&Silviu
 * Model: Aquarium Temp Sensor
 */

#include "aquarium_matter.h"
#include <inttypes.h>
#include <string.h>
#include "esp_log.h"
#include "esp_matter.h"
#include "esp_matter_attribute.h"
#include "esp_matter_console.h"
#include "esp_matter_ota.h"
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "AQUARIUM_MATTER";

// Device info
static const char* VENDOR_NAME = "Claude&Silviu";
static const char* PRODUCT_NAME = "Senzor Apa";
static const char* SERIAL_NUMBER = "AQ-TEMP-001";
static const char* HARDWARE_VERSION_STRING = "1.0";
static const char* SOFTWARE_VERSION_STRING = "1.0.0";

// Matter endpoint ID
static uint16_t temperature_endpoint_id = 0;

/**
 * Matter attribute update callback
 */
static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                          uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    if (type == attribute::PRE_UPDATE) {
        ESP_LOGD(TAG, "Attribute update: endpoint=0x%x, cluster=0x%" PRIx32 ", attribute=0x%" PRIx32, 
                 endpoint_id, cluster_id, attribute_id);
    }
    return ESP_OK;
}

/**
 * Matter identification callback (when user presses identify in Home app)
 */
static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id,
                                        uint8_t effect_id, uint8_t effect_variant, void *priv_data)
{
    ESP_LOGI(TAG, "Identification callback: type=%d, endpoint=0x%x", type, endpoint_id);
    // TODO: Flash LED or display something to identify device
    return ESP_OK;
}

/**
 * Set device basic information (manufacturer, model, etc.)
 */
static void set_device_info(node_t *node)
{
    // Get root endpoint (endpoint 0)
    endpoint_t *root_endpoint = endpoint::get(node, 0);
    if (!root_endpoint) {
        ESP_LOGW(TAG, "Root endpoint not found");
        return;
    }
    
    // Get Basic Information cluster
    uint32_t basic_cluster_id = BasicInformation::Id;
    cluster_t *basic_cluster = cluster::get(root_endpoint, basic_cluster_id);
    if (!basic_cluster) {
        ESP_LOGW(TAG, "Basic Information cluster not found");
        return;
    }
    
    // Update Vendor Name
    attribute_t *vendor_attr = attribute::get(basic_cluster, BasicInformation::Attributes::VendorName::Id);
    if (vendor_attr) {
        esp_matter_attr_val_t val = esp_matter_char_str((char*)VENDOR_NAME, strlen(VENDOR_NAME));
        attribute::set_val(vendor_attr, &val);
        ESP_LOGI(TAG, "Set Vendor Name: %s", VENDOR_NAME);
    }
    
    // Update Product Name
    attribute_t *product_attr = attribute::get(basic_cluster, BasicInformation::Attributes::ProductName::Id);
    if (product_attr) {
        esp_matter_attr_val_t val = esp_matter_char_str((char*)PRODUCT_NAME, strlen(PRODUCT_NAME));
        attribute::set_val(product_attr, &val);
        ESP_LOGI(TAG, "Set Product Name: %s", PRODUCT_NAME);
    }
    
    // Update Serial Number
    attribute_t *serial_attr = attribute::get(basic_cluster, BasicInformation::Attributes::SerialNumber::Id);
    if (serial_attr) {
        esp_matter_attr_val_t val = esp_matter_char_str((char*)SERIAL_NUMBER, strlen(SERIAL_NUMBER));
        attribute::set_val(serial_attr, &val);
        ESP_LOGI(TAG, "Set Serial Number: %s", SERIAL_NUMBER);
    }
    
    // Update Hardware Version String
    attribute_t *hw_attr = attribute::get(basic_cluster, BasicInformation::Attributes::HardwareVersionString::Id);
    if (hw_attr) {
        esp_matter_attr_val_t val = esp_matter_char_str((char*)HARDWARE_VERSION_STRING, strlen(HARDWARE_VERSION_STRING));
        attribute::set_val(hw_attr, &val);
    }
    
    // Update Software Version String
    attribute_t *sw_attr = attribute::get(basic_cluster, BasicInformation::Attributes::SoftwareVersionString::Id);
    if (sw_attr) {
        esp_matter_attr_val_t val = esp_matter_char_str((char*)SOFTWARE_VERSION_STRING, strlen(SOFTWARE_VERSION_STRING));
        attribute::set_val(sw_attr, &val);
    }
    
    ESP_LOGI(TAG, "Device info configured successfully");
}

/**
 * Initialize Matter Temperature Sensor
 */
bool aquarium_matter_init(void)
{
    ESP_LOGI(TAG, "Initializing Matter Temperature Sensor...");
    ESP_LOGI(TAG, "Manufacturer: %s", VENDOR_NAME);
    ESP_LOGI(TAG, "Model: %s", PRODUCT_NAME);
    
    // Create Matter node
    node::config_t node_config;
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
    
    if (!node) {
        ESP_LOGE(TAG, "Failed to create Matter node");
        return false;
    }
    
    // Create Temperature Sensor endpoint
    endpoint::temperature_sensor::config_t temp_sensor_config;
    // Set initial temperature (will be updated immediately with real reading)
    temp_sensor_config.temperature_measurement.measured_value = nullable<int16_t>(2500);  // 25.00°C initial
    temp_sensor_config.temperature_measurement.min_measured_value = nullable<int16_t>(-4000);  // -40°C
    temp_sensor_config.temperature_measurement.max_measured_value = nullable<int16_t>(12500);  // 125°C
    
    endpoint_t *endpoint = endpoint::temperature_sensor::create(node, &temp_sensor_config, ENDPOINT_FLAG_NONE, NULL);
    
    if (!endpoint) {
        ESP_LOGE(TAG, "Failed to create temperature sensor endpoint");
        return false;
    }
    
    temperature_endpoint_id = endpoint::get_id(endpoint);
    ESP_LOGI(TAG, "Temperature sensor endpoint created: 0x%x", temperature_endpoint_id);
    
    // Set device information (manufacturer, model, etc.)
    set_device_info(node);
    
    // Start Matter stack
    esp_err_t err = esp_matter::start(NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start Matter: %s", esp_err_to_name(err));
        return false;
    }
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║   MATTER TEMPERATURE SENSOR READY!     ║");
    ESP_LOGI(TAG, "╠════════════════════════════════════════╣");
    ESP_LOGI(TAG, "║  Manufacturer: Claude&Silviu           ║");
    ESP_LOGI(TAG, "║  Model: Senzor Apa                     ║");
    ESP_LOGI(TAG, "╠════════════════════════════════════════╣");
    ESP_LOGI(TAG, "║  PAIR WITH iPHONE:                     ║");
    ESP_LOGI(TAG, "║  1. Open Home app                      ║");
    ESP_LOGI(TAG, "║  2. Tap '+' -> Add Accessory           ║");
    ESP_LOGI(TAG, "║  3. Look for 'MATTER-3840'             ║");
    ESP_LOGI(TAG, "║  4. Or enter code: 20202021            ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
    
    return true;
}

/**
 * Update Matter temperature - Called from aquarium_controller.c
 */
extern "C" void aquarium_matter_update_temperature(float temp_celsius)
{
    ESP_LOGI(TAG, ">>> UPDATE TEMP CALLED: %.2f°C", temp_celsius);
    
    if (temperature_endpoint_id == 0) {
        ESP_LOGW(TAG, "Matter not initialized");
        return;
    }
    
    // Convert to Matter format (0.01°C resolution)
    // Matter uses int16 with 0.01°C units
    int16_t temp_matter = (int16_t)(temp_celsius * 100.0f);
    
    // Update attribute using esp_matter API
    esp_matter_attr_val_t val = esp_matter_nullable_int16(temp_matter);
    esp_err_t err = attribute::update(temperature_endpoint_id, 
                                       TemperatureMeasurement::Id,
                                       TemperatureMeasurement::Attributes::MeasuredValue::Id, 
                                       &val);
    
    if (err == ESP_OK) {
        ESP_LOGI(TAG, ">>> Matter temp updated: %.2f°C (raw: %d)", temp_celsius, temp_matter);
    } else {
        ESP_LOGE(TAG, ">>> FAILED to update Matter temp: %s", esp_err_to_name(err));
    }
}

/**
 * Print QR code for commissioning
 */
void aquarium_matter_print_qr_code(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Matter Pairing Code: 20202021");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "");
}

/**
 * Start commissioning mode
 */
void aquarium_matter_start_commissioning(void)
{
    ESP_LOGI(TAG, "Starting Matter commissioning...");
    chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
    ESP_LOGI(TAG, "Commissioning window opened - device is discoverable");
}
