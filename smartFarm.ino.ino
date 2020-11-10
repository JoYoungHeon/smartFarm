#include <LiquidCrystal_I2C.h>              // lcd를 사용하기 위한 라이브러리 추가
#include <SoftwareSerial.h>                 // 블루투스를 사용하기 위한 라이브러리 추가

SoftwareSerial BTSerial(5,4);
LiquidCrystal_I2C lcd(0x27, 16, 2);         // LCD모듈 초기 주소 값, 영역 설정

static int InputData = -1;                  // InputData를 사용하여 고정 값을 넣기위해 전역변수를 사용
static int i = 0;                           // 블루투스와 휴대폰 사이에 값을 i를 통해 전달받을 예정이므로 i의 값을 초기화
static char c[10];

int Relaypin1 = 7;                          // 릴레이모듈 7번 핀으로 설정
int Soil_moisture = A1;                     // 토양수분센서 핀을 A1으로 설정
int Red = 9;                                // 신호등 모듈 Red핀 설정
int Yellow = 10;                            // 신호등 모듈 Yellow핀 설정
int Green = 11;                             // 신호등 모듈 Green핀 설정


void setup() {
  Serial.begin(9600);                       // 초당 9600비트를 전송
  BTSerial.begin(9600);                     // 블루투스 또한 초당 9600비트를 전송
  lcd.init();                               // lcd 초기화
  lcd.backlight();                          // lcd 백라이트 ON
  pinMode(Relaypin1, OUTPUT);               // 릴레이 제어 1번핀을 출력으로 설정
  pinMode(Red, OUTPUT);                     // Red 출력핀 설정
  pinMode(Yellow, OUTPUT);                  // Yellow 출력핀 설정
  pinMode(Green, OUTPUT);                   // Green 출력핀 설정
  
}

void loop() {
  char c1[10];                              // 블루투스와 연동하여 휴대폰에 값을 보내준 결과 숫자가 맨 앞 한자리만 출력되는 것을 확인하여 온도, 전압 값을 휴대폰으로 출력하기 위해 배열 생성
  char c2[10];
  int readValue;                            // 전압을 구하기 위해 전력값을 읽어올 필요가 있었다.
  float voltage;                            // 온도와 전압은 소숫점까지 출력하기 위해 float 사용
  float temperature;

  readValue = analogRead(A0);
  voltage = (int)(readValue * 5.0f)/1024.0f;  // 전압, 온도 공식을 사용하여 전압과 온도 값을 구해주었다.
  temperature = (voltage*100.0f);

  sprintf(c1,"%d",readValue);               // 블루투스를 통하여 휴대폰과 연동시 위와 같은 문제로 형변환 해주었다.
  sprintf(c2,"%d",(int)temperature);        

  Serial.print("전압 : ");                   // 실시간으로 상황을 살펴보기 위해 시리얼 모니터로 전압, 온도값 출력
  Serial.println(readValue);
  Serial.print("온도 : ");
  Serial.println(temperature);
  delay(5000);                              // 전압과 온도 값을 여유있게 보기 위해서 딜레이를 5초 정도 주었다.
  
  char c3[10];                              // 임의 값을 넘겨주기 위한 배열 생성
  
  while(BTSerial.available()){              // 블루투스 통신을 위한 코드 사용, 제조사마다 블루투스는 적용되는 코드가 다르기 때문에 부품사에서 기본적으로 제공해주는 코드를 포함시켜 주었다.
    byte data = BTSerial.read();
    Serial.write(data);
    c[i] = data;
    i++;
  }
  if(c[0] == 45){                           // 모바일로 펌프를 강제로 수분 값을 입력하여 모터를 마치 자동, 수동으로 바꾸기 위한 코드, 정수 자체만 입력 받았을 때 에러가 발생하여 문자열을 정수로 바꿔줄 필요가 있었음
    InputData = -1;                         // 900이상을 입력하면 릴레이모듈(워터펌프)가 자동으로 실행되며 음수가 들어가면 실행이 종료
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
  
  Serial.print("수분 값 : ");              // 수분 값 및 물의 양 휴대폰으로 전송
  Serial.println(Soil_moisture);  
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
    digitalWrite(Red, LOW);                               //신호등 모듈 소등
    digitalWrite(Yellow, LOW);
    digitalWrite(Green, LOW);
    lcd.clear();
    digitalWrite(Relaypin1, LOW);                         //릴레이모듈(워터펌프)동작 정지
    delay(10000);                                         //동작정지 시간 10초 설정
}
