#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

SoftwareSerial BTSerial(5,4);
LiquidCrystal_I2C lcd(0x27, 16, 2);         // LCD모듈 초기 주소 값, 영역 설정

static int InputData = -1;                  //전역변수 사용
static int i = 0;
static char c[10];

int Relaypin1 = 7;                          // 릴레이모듈 7번 핀으로 설정
int Soil_moisture = A1;                     // 토양수분센서 핀을 A1으로 설정
int Red = 9;                                // 신호등 모듈 Red핀 설정
int Yellow = 10;                             // 신호등 모듈 Yellow핀 설정
int Green = 11;                             // 신호등 모듈 Green핀 설정


void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);
  lcd.init();
  lcd.backlight();                          // 백라이트 ON
  pinMode(Relaypin1, OUTPUT);               // 릴레이 제어 1번핀을 출력으로 설정
  pinMode(Red, OUTPUT);                     // Red 출력핀 설정
  pinMode(Yellow, OUTPUT);                  // Yellow 출력핀 설정
  pinMode(Green, OUTPUT);                   // Green 출력핀 설정
  
}

void loop() {
  char c1[10];                              // 온도, 전압 값을 휴대폰으로 출력하기 위해 배열 생성
  char c2[10];
  int readValue;
  float voltage;
  float temperature;

  readValue = analogRead(A0);
  voltage = (int)(readValue * 5.0f)/1024.0f;  // 전력, 온도 공식
  temperature = (voltage*100.0f);

  sprintf(c1,"%d",readValue);               // 형변환
  sprintf(c2,"%d",(int)temperature);        

  Serial.print("전압 : ");                   // 시리얼 모니터로 전압, 온도값 출력
  Serial.println(readValue);
  Serial.print("온도 : ");
  Serial.println(temperature);
  delay(5000);
  
  char c3[10];                              // 임의 값을 넘겨주기 위한 배열 생성
  i=0;
  
  while(BTSerial.available()){              // 블루투스 통신을 위한 코드 작성
    byte data = BTSerial.read();
    Serial.write(data);
    c[i] = data;
    i++;
  }
  if(c[0] == 45){                           // 펌프를 자동, 수동으로 바꾸기 위한 코드
    InputData = -1;
  }
  else{
    for(int j = 0; j < i; j++){
      c3[j] = c[j] ;
    }
    InputData = atoi(c3);  
  }
  
  while(Serial.available()){
    byte data = Serial.read();
    BTSerial.write(data);
  }
  
  BTSerial.write("Voltage : ");             // 전압, 온도 값을 휴대폰으로 전송
  BTSerial.write(c1);
  BTSerial.write("\n");
  BTSerial.write("Temperature : ");
  BTSerial.write(c2);
  BTSerial.write("\n");
  
  
  
  Flower_kit_on();
  
}
void Flower_kit_on() {                      
  int Soil_moisture = analogRead(A1);

  if(InputData < 0){
    Soil_moisture = analogRead(A1);
  }
  else{
    Soil_moisture = InputData;
  }
  
  Serial.print("수분 값 : ");              // 수분 값 휴대폰으로 전송
  Serial.println(Soil_moisture);  
//  char c3[10];
//  sprintf(c3, "%d", InputData);
  BTSerial.write("Water : ");
  BTSerial.write(Soil_moisture);
  BTSerial.write("\n");
  
  if (Soil_moisture > 800 )                 //수분 값이 900 초과의 조건
  {
    digitalWrite(Red, HIGH);                //신호등 모듈 빨간색 점등
    digitalWrite(Yellow, LOW);
    digitalWrite(Green, LOW);
    lcd.setCursor(0, 0);                    // 0번째, 0라인
    lcd.print("Water please");              // Water please 출력
    digitalWrite(Relaypin1, HIGH);          //릴레이모듈(워터펌프)동작
  }
  if (Soil_moisture > 600 && Soil_moisture < 800)        //수분 값이 700 초과 900 미만의 조건
  {
    digitalWrite(Red, LOW);                              //신호등 모듈 노란색 점등
    digitalWrite(Yellow, HIGH);
    digitalWrite(Green, LOW);
  }
  else if (Soil_moisture < 600)                          //수분 값이 700 미만의 조건
  {
    digitalWrite(Red, LOW);                              //신호등 모듈 초록색 점등
    digitalWrite(Yellow, LOW);
    digitalWrite(Green, HIGH);
    lcd.setCursor(0, 0);                                 // 0번째, 0라인
    lcd.print("Water enough");
    digitalWrite(Relaypin1, LOW);                        //릴레이모듈(워터펌프)동작 정지
    delay(1000);
    Flower_kit_off();                                    //동작정지 함수 호출
  }
}
void Flower_kit_off() {
    digitalWrite(Red, LOW);                               //신호등 모듈 빨간색 점등
    digitalWrite(Yellow, LOW);
    digitalWrite(Green, LOW);
    lcd.clear();
    digitalWrite(Relaypin1, LOW);                         //릴레이모듈(워터펌프)동작
    delay(10000);                                         //동작정지 시간 설정
}
