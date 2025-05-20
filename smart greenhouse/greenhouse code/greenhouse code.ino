//========================================LIBRARIES======================================//
#include <Servo.h>        
#include <SPI.h>              
#include <MFRC522.h>      
#include <EEPROM.h>       

#define LOCK_TIMEOUT  1000  // Время до блокировки замка после закрытия двери в мс 
#define MAX_TAGS        10   // Максимальное количество хранимых меток - ключей 
#define SERVO_PIN       9   // Пин сер во
#define BUZZER_PIN      22  // Пин баззера
#define RED_LED_PIN     49  // Пин красного светодиода
#define GREEN_LED_PIN   48  // Пин зеленого светодиода
#define RST_PIN         46  // Пин RST MFRC522
#define CS_PIN          53  // Пин SDA MFRC522
#define BTN_PIN         42  // Пин кнопки
#define DOOR_PIN        40  // Пин концевика двери, подтянут к VCC 
#define EE_START_ADDR   0   // Начальный адрес в EEPROM
#define EE_KEY        100   // Ключ EEPROM, для проверки на первое вкл.

MFRC522 rfid(CS_PIN, RST_PIN);  
Servo doorServo;                

#define DECLINE 0   
#define SUCCESS 1   
#define SAVED   2   
#define DELITED 3   

#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
#include <Wire.h>
#define DHT11PIN 10
#include <dht11.h>
Servo myservo;
dht11 DHT11;
#include "GyverHacks.h"
#include <IRremote.h>
#include <Keypad.h>
const int RECV_PIN = 6;
IRrecv irrecv(RECV_PIN);
decode_results results;
unsigned long key_value = 0;
#define LED1 49 // красный светодиод
#define LED2 48 // зеленый светодиод
#define RELAY 47 // реле на замок
#define NUM_KEYS 4 // количество знаков в коде
char key; 
char myarraw[NUM_KEYS] = { '2', '0', '0', '8'}; // массив с верным кодом
char button_pressed[NUM_KEYS]; //массив для хранения нажатых кнопок
int k = 0; // счетчик нажатий
int s = 0; // счетчик совпадений нажатых кнопок с верными
const byte ROWS = 4; // количество строк в матрице клавиатуры
const byte COLS = 4; // количество столбцов
char keys[ROWS][COLS] = { // таблица соответствия кнопок символам
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {31, 33, 35, 37}; // пины подключенных строк
byte colPins[COLS] = {39, 41, 43, 45}; // пины подключенных столбцов
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS ); // создаем объект клавиатуры для работы с ней
//========================================НеТрогать======================================//
unsigned long last_time;
int angle                      = 90;
int lightState                 = 0;
int leftlight                  = 0;
int rightlight                 = 0;
int water                      = 0;
int pumpState                  = 0;
int MT                         = 0;
//========================================Настройки======================================//
int lightPhotocellPin          = A0;
int rightPhotoPanelPin         = A1;
int leftPhotoPanelPin          = A2;
int waterLevelPin              = A3;
int coolerPin                  = A4;
int ledPin                     = 7;
int pumpPin                    = 8;
int maxTemp                    = 22;
int maxWaterLevel              = 5;
int maxdark                    = 400;
int motorSpeed                 = 175;
int door1                      = 1;
GTimer myTimer1(10000); // LockDoor Time
GTimer myTimer2(100); // Cooler Pwm Time
GTimer myTimer3(500); // Display clear Time

bool isOpen(void) {             // Функция должна возвращать true, если дверь физически открыта
  return digitalRead(DOOR_PIN); // Если дверь открыта - концевик размокнут, на пине HIGH
}

void lock(void) {               // Функция должна блокировать замок или нечто иное
  doorServo.attach(SERVO_PIN);
  doorServo.write(170);         // Для примера - запиранеие замка при помощи серво
  door1 = 0;
  delay(1000);
  //lcd.setCursor(9, 1);
  //lcd.print("D:Lock");
  delay(2000);
  doorServo.detach(); // Детачим серво, чтобы не хрустела
  Serial.println("lock");
}

void unlock(void) {             // Функция должна разблокировать замок или нечто иное
  doorServo.attach(SERVO_PIN);
  doorServo.write(100);          // Для примера - отпирание замка при помощи серво
  door1 = 1;
  delay(1000);
  //lcd.setCursor(9, 1);
  //lcd.print("D:Open");
  delay(2000);
  doorServo.detach();           // Детачим серво, чтобы не хрустела
  Serial.println("unlock");
}


bool locked = true;       // Флаг состояния замка
bool needLock = false;    // Служебный флаг
uint8_t savedTags = 0;    // кол-во записанных меток
//========================================VoidSetup======================================//
void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  while (! Serial);
  pinMode(waterLevelPin, INPUT);
  pinMode(coolerPin, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(rightPhotoPanelPin, INPUT);
  pinMode(lightPhotocellPin, INPUT);
  pinMode(leftPhotoPanelPin, INPUT);
  rightlight = analogRead(rightPhotoPanelPin);
  leftlight = analogRead(leftPhotoPanelPin);
  myservo.attach(9);
  irrecv.enableIRIn();
  irrecv.blink13(false);
  pinMode(LED1, OUTPUT); // красный светодиод
  pinMode(LED2, OUTPUT); // зеленый светодиод
  pinMode(RELAY, OUTPUT); // реле управления замком
  digitalWrite(RELAY, LOW); // вход реле инверсный, поэтому его сразу включаем (?!)

  SPI.begin();
  rfid.PCD_Init();

  // Настраиваем пины
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(DOOR_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  // Полная очистка при включении при зажатой кнопке
  uint32_t start = millis();        // Отслеживание длительного удержания кнопки после включения
  bool needClear = 0;               // Чистим флаг на стирание
  while (!digitalRead(BTN_PIN)) {   // Пока кнопка нажата
    if (millis() - start >= 3000) { // Встроенный таймаут на 3 секунды
      needClear = true;             // Ставим флаг стирания при достижении таймаута
      indicate(DELITED);            // Подаем сигнал удаления
      break;                        // Выходим из цикла
    }
  }

  // Инициализация EEPROM
  if (needClear or EEPROM.read(EE_START_ADDR) != EE_KEY) { // при первом включении или необходимости очистки ключей
    for (uint16_t i = 0; i < EEPROM.length(); i++) EEPROM.write(i, 0x00); // Чистим всю EEPROM
    EEPROM.write(EE_START_ADDR, EE_KEY);                   // Пишем байт-ключ
  } else {                                                 // Обычное включение
    savedTags = EEPROM.read(EE_START_ADDR + 1);            // Читаем кол-во меток в памяти
  }

  // Начальное состояние замка
  if (savedTags > 0) {      // Если метки в памяти есть
    if (isOpen()) {         // И дверь сейчас открыта
      ledSetup(SUCCESS);    // Зеленый лед
      locked = false;       // Замок открыт
      unlock();             // На всякий случай дернем замок
    } else {                // Метки есть, но дверь закрыта
      ledSetup(DECLINE);    // Красный лед
      locked = true;        // Замок закрыт
      lock();               // Блокируем замок
    }

    
  } else {                  // Если меток записано
    ledSetup(SUCCESS);      // Зеленый лед
    locked = false;         // Замок разлочен
    unlock();               // На всякий случай разблокируем замок
  }

}
//========================================VoidLoop=======================================//
void loop() {
  if (myTimer3.isReady()) {
    lcd.clear();
  } 
  if (door1 == 1) {
    lcd.setCursor(9, 1);
    lcd.print("D:Open");
  } else {
    lcd.setCursor(9, 1);
    lcd.print("D:Lock");
  }
  //======================================AutoLightPanel===============================//
  leftlight = analogRead(leftPhotoPanelPin);
  rightlight = analogRead(rightPhotoPanelPin) + 57;
  if (millis() - last_time >= 200) {

    if (leftlight > rightlight + 15) {
      angle += 1;
      myservo.write(angle);
    }
    if (rightlight > leftlight) {
      angle -= 1;
      myservo.write(angle);
    }
    if (angle > 130) {
      angle -= 2;
    }
    if (angle < 40) {
      angle += 2;
    }
    last_time = millis();
  }
  Serial.print("| leftlight: ");
  Serial.print(leftlight);
  Serial.print("| rightlight: ");
  Serial.print(rightlight);
  Serial.print("| myservoVal: ");
  Serial.println(angle);
  //======================================AutoLight====================================//
  lightState = digitalRead(ledPin);
  if (rightlight < maxdark) {
    digitalWrite(ledPin, HIGH);
  }
  if (rightlight > maxdark) {
    digitalWrite(ledPin, LOW);
  }
  //Serial.print("| Lights: ");
  //Serial.print(lightState);
  //======================================Temperature==================================//
  int chk = DHT11.read(DHT11PIN);

  lcd.setCursor(0, 0);
  lcd.print("H:");
  lcd.print((float)DHT11.humidity, 1);
  lcd.print("%");
  //Serial.print("| Humidity: ");
  //Serial.print((float)DHT11.humidity, 2);
  //======================================Humidity=====================================//
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print((float)DHT11.temperature, 1);
  lcd.print("C");
  //Serial.print("| Tempereture: ");
  //Serial.print((float)DHT11.temperature, 2);


  //======================================AutoCooler====================================//
  if ((float)DHT11.temperature > maxTemp && (float)DHT11.humidity < 5) {
    analogWrite(coolerPin, 500);
  } else {
    analogWrite(coolerPin, 0);
  }
  if (myTimer2.isReady()) {
    analogWrite(coolerPin, 0);
  }
  if ((float)DHT11.temperature < maxTemp && (float)DHT11.humidity  > 25) {
  analogWrite(coolerPin, 0);
  }
  lcd.setCursor(9, 0);
  // lcd.print("MW:");
  // lcd.print(maxWaterLevel);
  //======================================waterLevel===================================//
  water = analogRead(waterLevelPin) / 10;
  //Serial.print("| WaterLevel: ");
  //Serial.println(water);
  lcd.setCursor(9, 0);
  lcd.print("WL:");
  lcd.print(water);
  //if (myTimer3.isReady()) {
  //lcd.clear();
  //}
  //======================================AutoPump=====================================//
  if (water < maxWaterLevel) {
    digitalWrite(pumpPin, HIGH);
  } else {
    digitalWrite(pumpPin, LOW);
  }

  //=====================================IRcontrol=====================================//
  if (irrecv.decode(&results)) {

    if (results.value == 0XFFFFFFFF)
      results.value = key_value;

    switch (results.value) {

      case 0xFF30CF:
        Serial.println("1");
        break ;

      case 0xFF18E7:
        Serial.println("2");
        break ;

      case 0xFF7A85:
        Serial.println("3");
        break ;

      case 0xFF10EF:
        Serial.println("4");
        break ;

      case 0xFF38C7:
        Serial.println("5");
        break ;

      case 0xFF5AA5:
        Serial.println("6");
        break ;

      case 0xFF42BD:
        Serial.println("7");
        break ;

      case 0xFF4AB5:
        Serial.println("8");
        break ;

      case 0xFF52AD:
        Serial.println("9");
        break ;

      case 0xFFA25D:
        Serial.println("ON/OFF");
        break;

      case 0xFF629D:
        maxTemp++;
        Serial.println("VOL+");
        break;

      case 0xFFE21D:
        Serial.println("STOP/FUNCTION");
        break;

      case 0xFF22DD:
        Serial.println("|<<");
        break;

      case 0xFF02FD:
        Serial.println(">||");
        break ;

      case 0xFFC23D:
        Serial.println(">>|");
        break ;

      case 0xFFE01F:
        maxWaterLevel --;
        Serial.println("DOWN");
        break ;

      case 0xFFA857:
        maxTemp--;
        Serial.println("VOL-");
        break ;

      case 0xFF906F:
        maxWaterLevel ++;
        Serial.println("UP");
        break ;

      case 0xFF6897:
        Serial.println("0");
        break ;

      case 0xFF9867:
        Serial.println("EQ");
        break ;

      case 0xFFB04F:
        Serial.println("ST/REPT");
        break ;

    }
    key_value = results.value;
    irrecv.resume();
  }
  //=====================================DoorLock======================================//

  key = keypad.getKey(); 
  if ( key != NO_KEY ) 
  {
    button_pressed [k] = key; //сохраняем эту кнопочку в массиве
    k = k + 1; // запоминаем сколько уже кнопок нажали
    if (k == NUM_KEYS) // если нажали нужное количество кнопок
    { for ( uint8_t i = 0; i < NUM_KEYS; i++) // пройдемся по всему массиву
      {
        if (button_pressed[i] == myarraw[i]) // и проверим нажатые кнопки с верным кодом
        { s = s + 1; // плюсуем счетчик совпадений


        }

      }

      if (s == NUM_KEYS) //если у нас все кнопки совпали с кодом, то включаем реле
      {
        indicate(SUCCESS);                                           // Если нашли - подаем сигнал успеха
        unlock();                                                    // Разблокируем
//        lockTimeout = millis();                                      // Обновляем таймаут
        locked = false;
//        digitalWrite (RELAY, HIGH); // включили реле
//        digitalWrite (LED2, HIGH); // зажгли зеленый светик (пользователь ввел верный код)
//        //delay(10000);// ждем 5 секунд пока горит светик зеленый и включено реле
//        if (RELAY == HIGH && key != NO_KEY) {
//          digitalWrite (RELAY, LOW); // гасим реле
//          digitalWrite (LED2, LOW); // гасим светик
//        }
        k = 0; //сбрасываем счетчик нажатий нашей переменной
        s = 0; // сбрасываем счетчик совпадений нашей переменной
      } else { // если не все кнопки совпали с верным кодом
        indicate(DECLINE);
//        rfidTimeout = millis();
//        digitalWrite (LED1, HIGH); // включаем красный светик (пользователь ввел неверный код)
//        delay (1000); // ждем 5 секунд
//        digitalWrite (LED1, LOW); // гасим красн светик
        k = 0; // обнуляем счетчики, чтобы начать все заново
        s = 0; //
      }
    }
  }




  static uint32_t lockTimeout;             // Таймер таймаута для блокировки замка

  // Открытие по нажатию кнопки изнутри
  if (locked and !digitalRead(BTN_PIN)) {  // Если дверь закрыта и нажали кнопку
    indicate(SUCCESS);                     // Зеленый лед
    unlock();                              // Разблокируем замок
    lockTimeout = millis();                // Запомнили время
    locked = false;                        // Замок разлочен
  }

  // Проверка концевика двери
  if (isOpen()) {                          // Если дверь открыта
    lockTimeout = millis();                // Обновляем таймер
  }

  // Блокировка замка по таймауту (ключей > 0, замок разлочен, таймаут вышел)
  if (savedTags > 0 and !locked and millis() - lockTimeout >= LOCK_TIMEOUT) {
    ledSetup(DECLINE); // Красный лед
    lock();            // Блокируем
    locked = true;     // Ставим флаг
  }

  // Поднесение метки
  static uint32_t rfidTimeout; // Таймаут рфид
  if (rfid.PICC_IsNewCardPresent() and rfid.PICC_ReadCardSerial()) { // Если поднесена карта
    if (isOpen() and !digitalRead(BTN_PIN) and millis() - rfidTimeout >= 500) { // И дверь открыта + кнопка нажата
      saveOrDeleteTag(rfid.uid.uidByte, rfid.uid.size);              // Сохраняем или удаляем метку
    } else if (locked) {                                             // Иначе если замок заблокирован
      if (foundTag(rfid.uid.uidByte, rfid.uid.size) >= 0) {          // Ищем метку в базе
        indicate(SUCCESS);                                           // Если нашли - подаем сигнал успеха
        unlock();                                                    // Разблокируем
        lockTimeout = millis();                                      // Обновляем таймаут
        locked = false;                                              // Замок разблокирован
      } else if (millis() - rfidTimeout >= 500) {                    // Метка не найдена (с таймаутом)
        indicate(DECLINE);                                           // Выдаем отказ
      }
    }
    rfidTimeout = millis();                                          // Обвновляем таймаут
  }

  // Перезагружаем RFID каждые 0.5 сек (для надежности)
  static uint32_t rfidRebootTimer = millis(); // Таймер
  if (millis() - rfidRebootTimer > 500) {     // Каждые 500 мс
    rfidRebootTimer = millis();               // Обновляем таймер
    digitalWrite(RST_PIN, HIGH);              // Дергаем резет
    delay(1);
    digitalWrite(RST_PIN, LOW);
    rfid.PCD_Init();                          // Инициализируем модуль
  }
}

void ledSetup(bool state) {
  if (state) {  // Зеленый
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);
  } else {      // Красный
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
  }
}

// Звуковой сигнал + лед
void indicate(uint8_t signal) {
  ledSetup(signal); // Лед
  switch (signal) { // Выбираем сигнал
    case DECLINE:
      Serial.println("DECLINE");
      for (uint8_t i = 0; i < 2; i++) {
        tone(BUZZER_PIN, 100);
        delay(300);
        noTone(BUZZER_PIN);
        delay(100);
      }
      return;
    case SUCCESS:
      Serial.println("SUCCESS");
      tone(BUZZER_PIN, 890);
      delay(330);
      noTone(BUZZER_PIN);
      return;
    case SAVED:
      Serial.println("SAVED");
      for (uint8_t i = 0; i < 2; i++) {
        tone(BUZZER_PIN, 890);
        delay(330);
        noTone(BUZZER_PIN);
        delay(100);
      }
      return;
    case DELITED:
      Serial.println("DELITED");
      for (uint8_t i = 0; i < 3; i++) {
        tone(BUZZER_PIN, 890);
        delay(330);
        noTone(BUZZER_PIN);
        delay(100);
      }
      return;
  }
}

// Сравнение двух массивов известного размера
bool compareUIDs(uint8_t *in1, uint8_t *in2, uint8_t size) {
  for (uint8_t i = 0; i < size; i++) {  // Проходим по всем элементам
    if (in1[i] != in2[i]) return false; // Если хоть один не сошелся - массивы не совпадают
  }
  return true;                          // Все сошлись - массивы идентичны
}

// Поиск метки в EEPROM
int16_t foundTag(uint8_t *tag, uint8_t size) {
  uint8_t buf[8];   // Буфер метки
  uint16_t address; // Адрес
  for (uint8_t i = 0; i < savedTags; i++) { // проходим по всем меткам
    address = (i * 8) + EE_START_ADDR + 2;  // Считаем адрес текущей метки
    EEPROM.get(address, buf);               // Читаем метку из памяти
    if (compareUIDs(tag, buf, size)) return address; // Сравниваем - если нашли возвращаем асдрес
  }
  return -1;                                // Если не нашли - вернем минус 1
}

// Удаление или запись новой метки
void saveOrDeleteTag(uint8_t *tag, uint8_t size) {
  int16_t tagAddr = foundTag(tag, size);                      // Ищем метку в базе
  uint16_t newTagAddr = (savedTags * 8) + EE_START_ADDR + 2;  // Адрес крайней метки в EEPROM
  if (tagAddr >= 0) {                                         // Если метка найдена - стираем
    for (uint8_t i = 0; i < 8; i++)  {                        // 8 байт
      EEPROM.write(tagAddr + i, 0x00);                        // Стираем байт старой метки
      EEPROM.write(tagAddr + i, EEPROM.read((newTagAddr - 8) + i)); // На ее место пишем байт последней метки
      EEPROM.write((newTagAddr - 8) + i, 0x00);               // Удаляем байт последней метки
    }
    EEPROM.write(EE_START_ADDR + 1, --savedTags);             // Уменьшаем кол-во меток и пишем в EEPROM
    indicate(DELITED);                                        // Подаем сигнал
  } else if (savedTags < MAX_TAGS) {                          // метка не найдена - нужно записать, и лимит не достигнут
    for (uint16_t i = 0; i < size; i++) EEPROM.write(i + newTagAddr, tag[i]); // Зная адрес пишем новую метку
    EEPROM.write(EE_START_ADDR + 1, ++savedTags);             // Увеличиваем кол-во меток и пишем
    indicate(SAVED);                                          // Подаем сигнал
  } else {                                                    // лимит меток при попытке записи новой
    indicate(DECLINE);                                        // Выдаем отказ
    ledSetup(SUCCESS);
  }
}
