#include <Arduino.h>
#include <HTTPClient.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <wasm3.h>

WiFiMulti wifiMulti;
HTTPClient http;

String baseUrl = "http://192.168.8.199";
int clientID;
int js;
unsigned char *wasmModule;
int wasmModuleSize;

IM3Environment env;
IM3Runtime runtime;
IM3Module module;
IM3Function func_run;

#define FATAL(func, msg)              \
  {                                   \
    Serial.print("Fatal: " func " "); \
    Serial.println(msg);              \
    return;                           \
  }
#define TSTART()       \
  {                    \
    tstart = micros(); \
  }
#define TFINISH(s)               \
  {                              \
    tend = micros();             \
    Serial.print(s " in ");      \
    Serial.print(tend - tstart); \
    Serial.println(" us");       \
  }

int ssiregister()
{
  int id;
  if ((wifiMulti.run() ==
       WL_CONNECTED))
  {
    Serial.print("REGISTER\n");
    http.begin(baseUrl + "/register");
    Serial.print("[HTTP] GET /register...\n");
    int httpCode = http.GET();
    if (httpCode >
        0)
    {
      Serial.printf("[HTTP] Status: %d\n", httpCode);

      if (httpCode ==
          HTTP_CODE_OK)
      {
        id = http.getString().toInt();
        Serial.println(clientID);
      }
    }
    else
    {
      Serial.printf("[HTTP] Status: %s\n",
                    http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  else
  {
    Serial.print("connect failed");
  }
  delay(500);
  M5.Lcd.clear();
  return id;
}

void ssimodule()
{
  if ((wifiMulti.run() ==
       WL_CONNECTED))
  {
    Serial.print("MODULE\n");
    http.begin(baseUrl + "/module");
    Serial.print("[HTTP] GET /module...\n");
    int httpCode = http.GET();
    if (httpCode >
        0)
    {
      Serial.printf("[HTTP] STATUS: %d\n", httpCode);

      if (httpCode ==
          HTTP_CODE_OK)
      {
        // Get Content-Length
        int cl = http.getSize();
        wasmModuleSize = cl;
        wasmModule = (unsigned char *)malloc(cl + 1);
        // Malloc for the size of the binary Content-Length
        WiFiClient &payloadStream = http.getStream();
        int i = 0;
        while (payloadStream.available())
        {
          wasmModule[i] = payloadStream.read();
          // Serial.printf("%02X", c);
          i++;
        }
        // for (int i = 0; i < cl; i++)
        // {
        //   Serial.printf("%02X", wasmModule[i]);
        // }
        // // Free the allocation
        // finiWasmModule();
      }
    }
    else
    {
      Serial.printf("[HTTP] STATUS: %s\n",
                    http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  else
  {
    Serial.print("connect failed");
  }
  delay(5000);
  M5.Lcd.clear();
}

int ssijob(int id)
{
  int jobStart;
  if ((wifiMulti.run() ==
       WL_CONNECTED))
  {
    Serial.print("JOB\n");
    http.begin(baseUrl + "/job/" + String(id));
    Serial.print("[HTTP] GET /job...\n");
    int httpCode = http.GET();
    if (httpCode >
        0)
    {
      Serial.printf("[HTTP] Status: %d\n", httpCode);

      if (httpCode ==
          HTTP_CODE_OK)
      {
        jobStart = http.getString().toInt();
        Serial.println(jobStart);
      }
    }
    else
    {
      Serial.printf("[HTTP] Status: %s\n",
                    http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  else
  {
    Serial.print("connect failed");
  }
  delay(500);
  M5.Lcd.clear();
  return jobStart;
}

void ssisolve(int solution)
{
  int jobStart;
  if ((wifiMulti.run() ==
       WL_CONNECTED))
  {
    Serial.print("SOLVE\n");
    http.begin(baseUrl + "/solve/" + String(solution));
    Serial.print("[HTTP] GET /solve...\n");
    int httpCode = http.GET();
    if (httpCode >
        0)
    {
      Serial.printf("[HTTP] STATUS: %d\n", httpCode);

      if (httpCode ==
          HTTP_CODE_OK)
      {
        // jobStart = http.getString().toInt();
        // Serial.println(jobStart);
      }
    }
    else
    {
      Serial.printf("[HTTP] STATUS: %s\n",
                    http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  else
  {
    Serial.print("connect failed");
  }
  delay(500);
  M5.Lcd.clear();
  return;
}

void finiWasmModule()
{
  free(wasmModule);
  wasmModule = NULL;
}

void load_wasm()
{
  M3Result result = m3Err_none;

  if (!env)
  {
    env = m3_NewEnvironment();
    if (!env)
      FATAL("NewEnvironment", "failed");
  }

  m3_FreeRuntime(runtime);

  runtime = m3_NewRuntime(env, 1024, NULL);
  if (!runtime)
    FATAL("NewRuntime", "failed");

  result = m3_ParseModule(env, &module, wasmModule, wasmModuleSize);
  if (result)
    FATAL("ParseModule", result);

  result = m3_LoadModule(runtime, module);
  if (result)
    FATAL("LoadModule", result);

  result = m3_FindFunction(&func_run, runtime, "runner");
  if (result)
    FATAL("FindFunction", result);
}

void setup()
{
  M5.begin();
  M5.Power.begin();
  Serial.println("\nWasm3 v" M3_VERSION " (" M3_ARCH "), build " __DATE__ " " __TIME__);
  wifiMulti.addAP(
      "MUVR-2G",
      "imnottellingyou");
  M5.Lcd.print("\nConnecting Wifi...\n");

  delay(100);
  //  Main Loop
  clientID = ssiregister();
  ssimodule();
  uint32_t tend, tstart;
  TSTART();
  load_wasm();
  TFINISH("Wasm3 init");
  M5.Power.deepSleep();
}

void loop()
{
  // //delay(100);
  // js = ssijob(clientID);

  // // #ifdef ESP32
  // //   // On ESP32, we can launch in a separate thread (with 16Kb stack)
  // //   Serial.println("Running a separate task");
  // //   xTaskCreate(&ssitask, "wasm3", 32 * 1024, NULL, 5, NULL);
  // // #else
  // // ssitask(NULL);
  // // #endif
  // M3Result result;
  // uint32_t tend, tstart;
  // TSTART();
  // result = m3_CallV(func_run, js);

  // TFINISH("Done");

  // if (result == m3Err_none)
  // {
  //   uint32_t value = 0;
  //   result = m3_GetResultsV(func_run, &value);
  //   if (result)
  //     FATAL("GetResults: %s", result);

  //   Serial.print("Result: ");
  //   Serial.println(value);
  // }
  // else
  // {
  //   M3ErrorInfo info;
  //   m3_GetErrorInfo(runtime, &info);
  //   Serial.print("Error: ");
  //   Serial.print(result);
  //   Serial.print(" (");
  //   Serial.print(info.message);
  //   Serial.println(")");
  //   if (info.file && strlen(info.file) && info.line)
  //   {
  //     Serial.print("At ");
  //     Serial.print(info.file);
  //     Serial.print(":");
  //     Serial.println(info.line);
  //   }
  // }
}
