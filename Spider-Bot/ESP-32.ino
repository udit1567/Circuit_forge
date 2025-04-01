
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <WiFi.h>
#include "esp_http_server.h"
#include <ESP32Servo.h>

#define CAMERA_MODEL_AI_THINKER
/* ======================================== */

#if defined(CAMERA_MODEL_WROVER_KIT)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 21
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 19
#define Y4_GPIO_NUM 18
#define Y3_GPIO_NUM 5
#define Y2_GPIO_NUM 4
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#elif defined(CAMERA_MODEL_M5STACK_PSRAM)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM 15
#define XCLK_GPIO_NUM 27
#define SIOD_GPIO_NUM 25
#define SIOC_GPIO_NUM 23

#define Y9_GPIO_NUM 19
#define Y8_GPIO_NUM 36
#define Y7_GPIO_NUM 18
#define Y6_GPIO_NUM 39
#define Y5_GPIO_NUM 5
#define Y4_GPIO_NUM 34
#define Y3_GPIO_NUM 35
#define Y2_GPIO_NUM 32
#define VSYNC_GPIO_NUM 22
#define HREF_GPIO_NUM 26
#define PCLK_GPIO_NUM 21

#elif defined(CAMERA_MODEL_M5STACK_PSRAM_B)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM 15
#define XCLK_GPIO_NUM 27
#define SIOD_GPIO_NUM 22
#define SIOC_GPIO_NUM 23

#define Y9_GPIO_NUM 19
#define Y8_GPIO_NUM 36
#define Y7_GPIO_NUM 18
#define Y6_GPIO_NUM 39
#define Y5_GPIO_NUM 5
#define Y4_GPIO_NUM 34
#define Y3_GPIO_NUM 35
#define Y2_GPIO_NUM 32
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 26
#define PCLK_GPIO_NUM 21

#elif defined(CAMERA_MODEL_M5STACK_WITHOUT_PSRAM)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM 15
#define XCLK_GPIO_NUM 27
#define SIOD_GPIO_NUM 25
#define SIOC_GPIO_NUM 23

#define Y9_GPIO_NUM 19
#define Y8_GPIO_NUM 36
#define Y7_GPIO_NUM 18
#define Y6_GPIO_NUM 39
#define Y5_GPIO_NUM 5
#define Y4_GPIO_NUM 34
#define Y3_GPIO_NUM 35
#define Y2_GPIO_NUM 17
#define VSYNC_GPIO_NUM 22
#define HREF_GPIO_NUM 26
#define PCLK_GPIO_NUM 21

#elif defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#else
#error "Camera model not selected"
#endif
/* ======================================== */

// LEDs GPIO
#define LED_OnBoard 4

// SERVOs GPIO
#define servo_LeftRight_Pin 14
#define servo_UpDown_Pin 15

const int PWM_freq = 5000;
const int ledChannel = 5;
const int PWM_resolution = 8;

int servo_LeftRight_Pos = 75;
int servo_UpDown_Pos = 75;

Servo servo_Dummy_1;
Servo servo_Dummy_2;
Servo servo_LeftRight;
Servo servo_UpDown;

const char *ssid = "Excitel 2.4";
const char *password = "#Udit1588";

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";
/* ======================================== */

/* ======================================== Empty handle to esp_http_server */
httpd_handle_t index_httpd = NULL;
httpd_handle_t stream_httpd = NULL;
/* ======================================== */

/* ======================================== HTML code for index / main page */
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>

<head>
    <title>ESP32-CAM Spider Bot Control Panel</title>
    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
    <style>
        body {
            font-family: 'Roboto', sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f0f4f8;
            color: #333;
            text-align: center;
        }

        h3 {
            margin-top: 20px;
            font-size: 28px;
            color: #0d6efd;
        }

        .container {
            max-width: 750px;
            margin: 20px auto;
            padding: 20px;
            background: hsl(198, 100%, 92%);
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            border-radius: 12px;
        }

        img {
            width: 68vh;
            height: 45vh;
            border-radius: 12px;
            margin-bottom: 20px;
            box-shadow: 0 2px 6px rgba(0, 0, 0, 0.1);
        }

        .slidecontainer {
            margin: 20px 0;
        }

        .slider {
            width: 80%;
            height: 12px;
            background: #ddd;
            border-radius: 6px;
            outline: none;
            -webkit-appearance: none;
            appearance: none;
        }

        .slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 24px;
            height: 24px;
            background: #0d6efd;
            border-radius: 50%;
            cursor: pointer;
        }

        .slider::-moz-range-thumb {
            width: 24px;
            height: 24px;
            background: #0d6efd;
            border-radius: 50%;
            cursor: pointer;
        }

        .control-panel {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
            gap: 15px;
            margin-top: 20px;
        }

        .button {
            padding: 12px 20px;
            font-size: 14px;
            font-weight: bold;
            color: #fff;
            background-color: #0d6efd;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            transition: transform 0.2s, background-color 0.3s;
        }

        .button:hover {
            background-color: #004aad;
        }

        .button:active {
            transform: scale(0.95);
            background-color: #003b8f;
        }

        @media (max-width: 600px) {
            h3 {
                font-size: 24px;
            }

            .button {
                font-size: 12px;
                padding: 10px 15px;
            }
        }
    </style>
</head>

<body>
    <div class="container">
        <h3>Spider Bot Control Panel</h3>
        <img src id="vdstream" alt="Video Stream">
        <div class="slidecontainer">
            <span>LED Flash</span>
            <input type="range" min="0" max="20" value="0" class="slider" id="mySlider">
        </div>
        <div class="control-panel">
            <button class="button" onclick="send_cmd('Stand')">STAND</button>
            <button class="button" onclick="send_cmd('Sit')">SIT</button>
            <button class="button" onclick="send_cmd('Step-forward')">STEP FORWARD</button>
            <button class="button" onclick="send_cmd('Step-back')">STEP BACK</button>
            <button class="button" onclick="send_cmd('Turn-left')">TURN LEFT</button>
            <button class="button" onclick="send_cmd('Turn-right')">TURN RIGHT</button>
            <button class="button" onclick="send_cmd('Hand-wave')">HAND WAVE</button>
            <button class="button" onclick="send_cmd('Hand-shake')">HAND SHAKE</button>
            <button class="button" onclick="send_cmd('Body-dance')">BODY DANCE</button>
        </div>
    </div>

    <script>
        window.onload = document.getElementById("vdstream").src = window.location.href.slice(0, -1) + ":81/stream";

        var slider = document.getElementById("mySlider");

        slider.oninput = function () {
            let slider_pwm_val = "S," + slider.value;
            send_cmd(slider_pwm_val);
        }

        function send_cmd(cmds) {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/action?go=" + cmds, true);
            xhr.send();
        }
    </script>
</body>

</html>
)rawliteral";
/* ======================================== */

/* ________________________________________________________________________________ Index handler function to be called during GET or uri request */
static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}
/* ________________________________________________________________________________ */

/* ________________________________________________________________________________ stream handler function to be called during GET or uri request. */
static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char *part_buf[64];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK)
    {
        return res;
    }

    /* ---------------------------------------- Loop to show streaming video from ESP32 Cam camera. */
    while (true)
    {
        fb = esp_camera_fb_get();
        if (!fb)
        {
            Serial.println("Camera capture failed (stream_handler)");
            res = ESP_FAIL;
        }
        else
        {
            if (fb->width > 400)
            {
                if (fb->format != PIXFORMAT_JPEG)
                {
                    bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                    esp_camera_fb_return(fb);
                    fb = NULL;
                    if (!jpeg_converted)
                    {
                        Serial.println("JPEG compression failed");
                        res = ESP_FAIL;
                    }
                }
                else
                {
                    _jpg_buf_len = fb->len;
                    _jpg_buf = fb->buf;
                }
            }
        }
        if (res == ESP_OK)
        {
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if (fb)
        {
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        }
        else if (_jpg_buf)
        {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if (res != ESP_OK)
        {
            break;
        }
    }
    /* ---------------------------------------- */
    return res;
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (data.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
/* ________________________________________________________________________________ */

/* ________________________________________________________________________________ cmd handler function to be called during GET or uri request. */
static esp_err_t cmd_handler(httpd_req_t *req)
{
    char *buf;
    size_t buf_len;
    char variable[32] = {
        0,
    };

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1)
    {
        buf = (char *)malloc(buf_len);
        if (!buf)
        {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
        {
            if (httpd_query_key_value(buf, "go", variable, sizeof(variable)) == ESP_OK)
            {
            }
            else
            {
                free(buf);
                httpd_resp_send_404(req);
                return ESP_FAIL;
            }
        }
        else
        {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        free(buf);
    }
    else
    {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    int res = 0;
    char *command = variable;
    if (command == "stand")
    {
        Serial.println(variable);
    }
    else if (command == "step_forward")
    {
        Serial.println(variable);
    }
    else if (command == "step_back")
    {
        Serial.println(variable);
    }
    else if (command == "turn_left")
    {
        Serial.println(variable);
    }
    else if (command == "turn_right")
    {
        Serial.println(variable);
    }
    else if (command == "hand_wave")
    {
        Serial.println(variable);
    }
    else if (command == "hand_shake")
    {
        Serial.println(variable);
    }
    else if (command == "body_dance")
    {
        Serial.println(variable);
    }
    else if (command == "sit")
    {
        Serial.println(variable);
    }
    else
    {
        Serial.println(variable);
    }

    String getData = String(variable);
    String resultData = getValue(getData, ',', 0);

    if (resultData == "S")
    {
        resultData = getValue(getData, ',', 1);
        int pwm = map(resultData.toInt(), 0, 20, 0, 255);
    }

    if (resultData == "B")
    {
        resultData = getValue(getData, ',', 1);

        if (resultData == "U")
        {
            if (servo_UpDown_Pos >= 5)
            {
                servo_UpDown_Pos -= 5;
                servo_UpDown.write(servo_UpDown_Pos);
            }
        }

        if (resultData == "D")
        {
            if (servo_UpDown_Pos <= 175)
            {
                servo_UpDown_Pos += 5;
                servo_UpDown.write(servo_UpDown_Pos);
            }
        }

        if (resultData == "R")
        {
            if (servo_LeftRight_Pos <= 175)
            {
                servo_LeftRight_Pos += 5;
                servo_LeftRight.write(servo_LeftRight_Pos);
            }
        }

        if (resultData == "L")
        {
            if (servo_LeftRight_Pos >= 5)
            {
                servo_LeftRight_Pos -= 5;
                servo_LeftRight.write(servo_LeftRight_Pos);
            }
        }
    }

    if (res)
    {
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

void startCameraWebServer()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler,
        .user_ctx = NULL};

    httpd_uri_t cmd_uri = {
        .uri = "/action",
        .method = HTTP_GET,
        .handler = cmd_handler,
        .user_ctx = NULL};

    httpd_uri_t stream_uri = {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = NULL};

    if (httpd_start(&index_httpd, &config) == ESP_OK)
    {
        httpd_register_uri_handler(index_httpd, &index_uri);
        httpd_register_uri_handler(index_httpd, &cmd_uri);
    }

    config.server_port += 1;
    config.ctrl_port += 1;
    if (httpd_start(&stream_httpd, &config) == ESP_OK)
    {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }

    Serial.println();
    Serial.println("Camera Server started successfully");
    Serial.print("Camera Stream Ready! Go to: http://");
    Serial.println(WiFi.localIP());
    Serial.println();
}

void setup()
{

    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);
    Serial.setDebugOutput(false);
    Serial.println();
    pinMode(LED_OnBoard, OUTPUT);
    Serial.println();
    Serial.println("------------");
    Serial.println("Start setting Servos...");
    servo_LeftRight.setPeriodHertz(50); //--> // Standard 50hz servo
    servo_UpDown.setPeriodHertz(50);    //--> // Standard 50hz servo
    servo_Dummy_1.attach(12, 1000, 2000);
    servo_Dummy_2.attach(13, 1000, 2000);
    servo_LeftRight.attach(servo_LeftRight_Pin, 1000, 2000);
    servo_UpDown.attach(servo_UpDown_Pin, 1000, 2000);

    servo_LeftRight.write(servo_LeftRight_Pos);
    servo_UpDown.write(servo_UpDown_Pos);
    Serial.println("Setting up the servos was successful.");
    Serial.println("------------");
    Serial.println();
    delay(2000);
    Serial.println();
    Serial.println("------------");
    Serial.println("Start configuring and initializing the camera...");
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound())
    {
        config.frame_size = FRAMESIZE_VGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    }
    else
    {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        ESP.restart();
    }

    Serial.println("Configure and initialize the camera successfully.");
    Serial.println("------------");
    Serial.println();

    WiFi.mode(WIFI_STA);
    Serial.println("------------");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
    connecting_process_timed_out = connecting_process_timed_out * 2;
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        digitalWrite(LED_OnBoard, HIGH);
        delay(250);
        digitalWrite(LED_OnBoard, LOW);
        delay(250);
        if (connecting_process_timed_out > 0)
            connecting_process_timed_out--;
        if (connecting_process_timed_out == 0)
        {
            delay(1000);
            ESP.restart();
        }
    }
    digitalWrite(LED_OnBoard, LOW);
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("------------");
    Serial.println("");

    startCameraWebServer();
}
/* ________________________________________________________________________________ */

void loop()
{

    delay(1);
}
