#include <OneButton.h>
#include "net.h"
#include "common.h"
#include "PreferencesUtil.h"
#include "tftUtil.h"
#include "task.h"

/**
Dudu天气时钟  版本2.0

本次更新内容：

1.配网页面更新，增加了城市的上级行政区划，用于排除重名城市。
(之前输入普陀，可能是浙江舟山市普陀区，也可能是上海市普陀区，有了上级行政区划，就可以进行精确划分了)

2.更改了天气接口，由于易客天气现在只免费3个月，所以这个版本将天气接口改为了和风天气，功能更强大、接口更稳定且永久免费。

3.增加了空气质量页面，包含了pm10颗粒物，pm2.5颗粒物，no2二氧化氮，so2二氧化硫，co一氧化碳以及o3臭氧。
*/


unsigned int prevDisplay = 0; // 实况天气页面上次显示的时间
unsigned int preTimerDisplay = 0; // 计数器页面上次显示的毫秒数/10,即10毫秒显示一次
unsigned long startMillis = 0; // 开始计数时的毫秒数
int synDataRestartTime = 60; // 同步NTP时间和天气信息时，超过多少秒就重启系统，防止网络不好时，傻等
bool isCouting = false; // 计时器是否正在工作
OneButton myButton(BUTTON, true);

void setup() {
  Serial.begin(115200);
  Serial.println("start setup");
  pinMode(10,OUTPUT);//gpio10,无源蜂鸣器
  // TFT初始化
  tftInit();
  //Serial.print("after tftinit");
  // 显示系统启动文字
  drawText("系统启动中...");
  //Serial.print("after drawtext");
  // 测试的时候，先写入WiFi信息，省的配网，生产环境请注释掉
  setInfo4Test();
  // 查询是否有配置过Wifi，没有->进入Wifi配置页面（0），有->进入天气时钟页面（1）
  getWiFiCity();
  // nvs中没有WiFi信息，进入配置页面
  if(ssid.length() == 0 || pass.length() == 0 || city.length() == 0 ){
    currentPage = SETTING; // 将页面置为配置页面
    wifiConfigBySoftAP(); // 开启SoftAP配置WiFi
  }else{ // 有WiFi信息，连接WiFi后进入时钟页面
    currentPage = WEATHER; // 将页面置为时钟页面
    // 连接WiFi,30秒超时重启并恢复出厂设置
    connectWiFi(30); 
    // 查询是否有城市id，如果没有，就利用city和adm查询出城市id，并保存为location
    if(location.equals("")){
      getCityID();
    }
    // 初始化一些列数据:NTP对时、实况天气、一周天气
    initDatas();
    tGetLocalTempCallback();//初始化本地温湿度
    // 绘制实况天气页面
    drawWeatherPage();
    // 多任务启动
    startRunner();
    // 初始化定时器，让查询天气的多线程任务在一小时后再使能
    startTimerQueryWeather();
    // 初始化按键监控
    myButton.attachClick(click);
    myButton.attachDoubleClick(doubleclick);
    myButton.attachLongPressStart(longclick);
    myButton.setPressMs(2000); //设置长按时间
    // myButton.setClickMs(300); //设置单击时间
    myButton.setDebounceMs(10); //设置消抖时长 
  }
}
const int tones[] = {
    261, // C4
    294, // D4
    329, // E4
    349, // F4
    392, // G4
    440, // A4
};
// 定义旋律（使用数组索引表示音符）
const int melody[] = {0, 0, 4, 4, 5, 5, 4, -1,
                      3, 3, 2, 2, 1, 1, 0, -1,
                      4, 4, 3, 3, 2, 2, 1, -1,
                      0, 0, 4, 4, 5, 5, 4, -1,
                      3, 3, 2, 2, 1, 1, 0}; // -1表示停顿





void sing_a_song()
{
  Serial.println("sing a song");
  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) 
  {
    int freq = (melody[i] == -1) ? 0 : tones[melody[i]];
    if (freq > 0) {
      tone(10, freq); // 播放对应频率的音调
        delay(500); // 每个音符持续500毫秒
    } else {
      noTone(10); // 停止音调
      delay(200); // 停顿200毫秒
        }
  }
}
void loop() {
  //Serial.print("start to run\r\n");
  //sing_a_song();
  myButton.tick();
  switch(currentPage){
    case SETTING:  // 配置页面
      doClient(); // 监听客户端配网请求
      break;
    case WEATHER:  // 天气时钟页面
      executeRunner();
      time_t now;
      time(&now);
      if(now != prevDisplay){ // 每秒更新一次时间显示
        prevDisplay = now;
        // 绘制时间、日期、星期
        drawDateWeek();
      }
      break;
    case AIR:  // 空气质量页面
      executeRunner();
      break;
    case FUTUREWEATHER:  // 未来天气页面
      executeRunner();
      break;
    case THEME:  // 黑白主题设置页面
      executeRunner();
      break;
    case TIMER:  // 计时器页面
      if(isCouting && (millis() / 10) != preTimerDisplay){ // 每十毫秒更新一次数字显示
        preTimerDisplay = millis() / 10;
        // 绘制计数器数字
        drawNumsByCount(timerCount + millis() - startMillis);
      }
      break;
    case ALARM:  // 闹钟页面
      executeRunner();
      break;
    case RESET:  // 恢复出厂设置页面
      executeRunner();
      break;
    default:
      break;
  }
}

////////////////////////// 按键区///////////////////////
// 单击操作，用来切换各个页面
void click(){
  if(currentPage == TIMER){
    if(!isCouting){
      // Serial.println("开始计数");
      startMillis = millis();
    }else{
      // Serial.println("停止计数");
      timerCount += millis() - startMillis;
      // 绘制计数器数字
      drawNumsByCount(timerCount);
    }
    isCouting = !isCouting;
  }
}
void doubleclick(){
  switch(currentPage){
    case WEATHER:
      disableAnimScrollText();
      drawAirPage();
      currentPage = AIR;
      break;
    case AIR:
      drawFutureWeatherPage();
      currentPage = FUTUREWEATHER;
      break;
    case FUTUREWEATHER:
      drawThemePage();
      currentPage = THEME;
      break;
    case THEME:
      drawTimerPage();
      currentPage = TIMER;
      break;
    case TIMER:
      drawAlarmPage();
      currentPage = ALARM;
      break;
    case ALARM:
      drawResetPage();
      currentPage = RESET;
      break;
    case RESET:
      drawWeatherPage();
      enableAnimScrollText();
      currentPage = WEATHER;
      break;
    default:
      break;
  }
}
uint8_t alarmHour = 0;
uint8_t alarmMinute = 0;
uint8_t alarmSecond = 0;
uint8_t currentAlarmSection = 0;//0-小时，1-分钟，2-秒
uint8_t isSettingAlarm = 0;//0-未设置，1-设置中


void longclick(){
  if(currentPage == RESET){
    Serial.println("恢复出厂设置");
    // 恢复出厂设置并重启
    clearWiFiCity();
    restartSystem("已恢复出厂设置", false);
  }else if(currentPage == THEME){
    Serial.println("更改主题");
    if(backColor == BACK_BLACK){ // 原先为黑色主题，改为白色
      backColor = BACK_WHITE;
      backFillColor = 0xFFFF;
      penColor = 0x0000;
    }else{
      backColor = BACK_BLACK;
      backFillColor = 0x0000;
      penColor = 0xFFFF;
    }
    // 将新的主题存入nvs
    setBackColor(backColor);
    // 返回实时天气页面
    drawWeatherPage();
    enableAnimScrollText();
    currentPage = WEATHER;
  }else if(currentPage == TIMER){
    Serial.println("计数器归零");
    timerCount = 0; // 计数值归零
    isCouting = false; // 计数器标志位置为false
    drawNumsByCount(timerCount); // 重新绘制计数区域，提示区域不用变
  }else if(currentPage == ALARM){
    //长按开始设置闹钟，第一次长按设置小时，第二次长按设置分钟，第三次长按设置秒，第四次长按设置完毕
    //短按一次，当前设置的值加1，最大不超过24小时，60分钟，60秒
    


    if(isSettingAlarm == 0){
      isSettingAlarm = 1;
      alarmHour = 0;
      alarmMinute = 0;
      alarmSecond = 0;
      currentAlarmSection = 0;
    }else{
      //设置完毕，开始修改闹钟时间，并用task来检查是否要闹钟
      isSettingAlarm = 0;
      //将设置的时间存入nvs
      Preferences preferences;
      preferences.begin("alarm", false);
      preferences.putUChar("hour", alarmHour);
      preferences.putUChar("minute", alarmMinute);
      preferences.putUChar("second", alarmSecond);
      preferences.end();


    }
      drawAlarmPage();
    //Serial.println("播放音乐");
    //sing_a_song();

  }

}
////////////////////////////////////////////////////////


// 初始化一些列数据:NTP对时、实况天气、一周天气
void initDatas(){
  uint8_t err_cnt=0;
  startTimerShowTips(); // 获取数据时，循环显示提示文字
  // 记录此时的时间，在同步数据时，超过一定的时间，就直接重启
  time_t start;
  time(&start);
  // 获取NTP并同步至RTC,第一次同步失败，就一直尝试同步
  getNTPTime();
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)){
    time_t end;
    time(&end);
    if((end - start) > synDataRestartTime){
      restartSystem("同步数据失败", true);
    }
    Serial.println("时钟对时失败...");
    getNTPTime();
  }
  //第一次查询实况天气,如果查询失败，就一直反复查询
  getNowWeather();
  while(!queryNowWeatherSuccess){
    time_t end;
    time(&end);
    if((end - start) > synDataRestartTime){
      restartSystem("同步数据失败", true);
    }
    err_cnt++;
    if(err_cnt > 3) { // 错误超过三次退出。
      sleep(1);
      restartSystem("同步数据失败", true);
    }
    Serial.println("实况天气查询失败...重试中...");
    sleep(1);
    getNowWeather();
  }
  //第一次查询空气质量,如果查询失败，就一直反复查询
  err_cnt = 0;
  getAir();
  while(!queryAirSuccess){
    time_t end;
    time(&end);
    if((end - start) > synDataRestartTime){
      restartSystem("查询空气质量失败", true);
    }
    err_cnt++;
    if(err_cnt > 3) { // 错误超过三次退出。
      sleep(1);
      restartSystem("查询空气质量失败", true);
    }
    sleep(1);
    getAir();
  }
  //第一次查询一周天气,如果查询失败，就一直反复查询
  err_cnt = 0;
  getFutureWeather();
  while(!queryFutureWeatherSuccess){
    time_t end;
    time(&end);
    if((end - start) > synDataRestartTime){
      restartSystem("同步数据失败", true);
    }
    err_cnt++;
    if(err_cnt > 3) { // 错误超过三次退出。
      sleep(1);
      restartSystem("查询未来天气失败", true);
    }
    sleep(1); 
    getFutureWeather();
  }
  //结束循环显示提示文字的定时器
  timerEnd(timerShowTips);
  //将isStartQuery置为false,告诉系统，启动时查询天气已完成
  isStartQuery = false;
}

