#include <Servo.h>
#include <Keypad.h>

Servo lockServo;

const int irSensorPin = 11; // 적외선 센서 핀
int packageDetected = 0;   // 택배 감지 상태
String enteredPassword = ""; // 키패드 입력
String correctPassword = "1234"; // 기본 비밀번호 (OCR로 받은 번호로 갱신 예정)

// 키패드 설정
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  pinMode(irSensorPin, INPUT);
  lockServo.attach(10); // 서보 모터 핀
  lockServo.write(0); // 초기 상태: 잠금 해제
  Serial.begin(9600); // ESP32-CAM과 통신
}

void loop() {
  int irState = digitalRead(irSensorPin);

  if (irState == LOW && packageDetected == 0) {
    // 택배 감지
    Serial.println("Package Detected"); // ESP32-CAM에 알림
    packageDetected = 1;
    delay(2000); // debounce 처리
  }

  // 키패드 입력 처리
  char key = keypad.getKey();
  if (key) {
    if (key == '*') {
      // 입력 완료
      if (enteredPassword == correctPassword) {
        Serial.println("Unlocking");
        lockServo.write(90); // 잠금 해제
        delay(5000); // 일정 시간 후 다시 잠금
        lockServo.write(0);
        packageDetected = 0; // 초기화
        correctPassword = 1234; //비밀번호 초기화
      } else {
        Serial.println("Incorrect Password");
      }
      enteredPassword = ""; // 입력 초기화
    } else if (key == '#') {
      // 입력 초기화
      enteredPassword = "";
    } else {
      // 비밀번호 추가
      enteredPassword += key;
    }
  }

  // ESP32-CAM으로부터 비밀번호 갱신
  if (Serial.available() > 0) {
    String pass;
    pass = Serial.readStringUntil('\n'); // ESP32-CAM에서 비밀번호 수신
    if(pass.length() == 4 && pass.toInt() != 0){
      correctPassword = pass;
      Serial.println("Password Updated");
    }
    else{
      Serial.println("invalid password");
    } 
  }
}
