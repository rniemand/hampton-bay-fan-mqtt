// ===================================================================
// Configure WiFi and MQTT
// ===================================================================
#define WIFI_SSID "ENTER_WIFI_SSID"
#define WIFI_PASS "ENTER_WIFI_SSID_PASS"
#define MQTT_HOST "192.168.0.60"
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASS ""
#define MQTT_CLIENT_NAME "HAMPTONBAY"
#define BASE_TOPIC "home/fans"


// ===================================================================
// MQTT Topics
// ===================================================================
#define LOGGING_TOPIC BASE_TOPIC "/log"
#define STATUS_TOPIC BASE_TOPIC "/status"
#define LOGGING_TOPIC BASE_TOPIC "/logs"
#define SUBSCRIBE_TOPIC_ON_SET BASE_TOPIC "/+/on/set"
#define SUBSCRIBE_TOPIC_ON_STATE BASE_TOPIC "/+/on/state"
#define SUBSCRIBE_TOPIC_SPEED_SET BASE_TOPIC "/+/speed/set"
#define SUBSCRIBE_TOPIC_SPEED_STATE BASE_TOPIC "/+/speed/state"
#define SUBSCRIBE_TOPIC_LIGHT_SET BASE_TOPIC "/+/light/set"
#define SUBSCRIBE_TOPIC_LIGHT_STATE BASE_TOPIC "/+/light/state"


// ===================================================================
// Configure CC1101
// ===================================================================
// 303.631 - (original) determined from FAN-9T remote tramsmissions
// 303.875 - (personal) this seems to work for my fans
#define RF_FREQUENCY    303.875
#define RF_PROTOCOL     11
#define RF_REPEATS      8
#define RF_PULSE_LENGTH 320


// ===================================================================
// Optional configuration
// ===================================================================
#define DEBUG_MODE false
