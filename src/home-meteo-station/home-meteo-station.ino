#include <dht11.h>

// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain

#include "DHT.h"

#define DHTPIN 2     // what pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);

int anodPins[] = {A1, A2, A3, A4}; // Задаем пины для кажого разряда
int segmentsPins[] = {5, 6, 7, 8, 9, 10, 11, 12}; //Задаем пины для каждого сегмента (из 7 + 1(точка))

void setup() {
  // Все выходы программируем как OUTPUT
  for (int i = 0; i < 4; i++) {
    pinMode(anodPins[i], OUTPUT);
  }
  for (int i = 0; i <= 7; i++) {
    pinMode(segmentsPins[i], OUTPUT);
  }

  Serial.begin(9600); 
  Serial.println("DHTxx test!");
  pinMode(2, INPUT);
  dht.begin();
}

static int arr[4][8];

void loop() {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();
   
  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  logDataToSerial(hum, temp);
  getDigits(hum, false);
  for (int i=0; i<250; i++) {
    displayMessage(arr);
  }
  
  getDigits(temp, true);
  for (int i=0; i<250; i++) {
    displayMessage(arr);
  }
}

void logArraytoSerial() {
  for (int i=0; i<4; i++) {
    for (int j=0; j<=7; j++) {
      Serial.print(arr[i][j]);
    }
    Serial.println("");
  }
  Serial.println("---");  
}

void displayMessage(int dig[4][8]) {
  for (int i = 0; i < 4; i++) { // Каждый разряд по очереди
    for (int k = 0; k < 8; k++) {// Каждый сегмент по очереди - исходя из заданной карты
      digitalWrite(segmentsPins[k], ((dig[i][k] == 1) ? HIGH : LOW));
    }
    digitalWrite(anodPins[i], LOW);
    delay(1);
    digitalWrite(anodPins[i], HIGH);
  }
}

void logDataToSerial(float hum, float temp) {
  Serial.print("Humidity: "); 
  Serial.print(hum);
  Serial.print(" %\t");
  Serial.print("Temperature: "); 
  Serial.print(temp);
  Serial.println(" *C ");  
}

//{A, B, C, D, E, F, G,DP} - распиновка сегментов
int seg[12][8] = {
  {1, 1, 1, 1, 1, 1, 0, 0}, //Цифра 0
  {0, 1, 1, 0, 0, 0, 0, 0}, //Цифра 1
  {1, 1, 0, 1, 1, 0, 1, 0}, //Цифра 2
  {1, 1, 1, 1, 0, 0, 1, 0}, //Цифра 3
  {0, 1, 1, 0, 0, 1, 1, 0}, //Цифра 4
  {1, 0, 1, 1, 0, 1, 1, 0}, //Цифра 5
  {1, 0, 1, 1, 1, 1, 1, 0}, //Цифра 6
  {1, 1, 1, 0, 0, 0, 0, 0}, //Цифра 7
  {1, 1, 1, 1, 1, 1, 1, 0}, //Цифра 8
  {1, 1, 1, 1, 0, 1, 1, 0}, //Цифра 9
  {1, 0, 0, 1, 1, 1, 0, 0}, //Знак C
  {0, 0, 0, 0, 0, 0, 0, 0}  //Пустой символ
};

void getDigits (float value, boolean isTemperature) {
  int digits = 4; // У нас 4 разряда
  if (isTemperature) digits = 3; // Из-за минуса один символ убирается*/

  // Делим число на 2 группы - отдельно целую часть и дробную.
  int intPart = (int)abs(value);
  int intLength = ((String)intPart).length(); // Смотрим, сколько разрядов занимает целая часть

  char valueChars[8]; // По нормальному float в String преобразовать нельзя, поэтому...
  dtostrf(value, 5, 4, valueChars); //... копируем данные float в массив chars[]

  // На дробную часть у нас остается разрядов: digits-intLength
  String valueStr = valueChars;                                                 // Теперь с текстовым форматом можно работать
  //  Serial.print("valueStr: "); Serial.println(valueStr);

  int fracIndex = valueStr.indexOf(".");                                        // Получаем индекс начала дробной части
  String fracPartStr = valueStr.substring(fracIndex + 1, valueStr.length());    // Выделяем цифры дробной части
  int fracDigits = digits - intLength;
  fracPartStr = fracPartStr.substring(0, fracDigits);                           // Вычленяем цифры, которые будут выводиться на дисплей
  //  Serial.print("fracDigits: "); Serial.println(fracDigits);
  //  Serial.print("fracPartStr: "); Serial.println(fracPartStr);

  int fracInt = fracPartStr.toInt();                                            // Переменная для проверки, нужно ли что-то выводить, если нули, то нет
  //  Serial.print("fracInt: "); Serial.println(fracInt);

  // Собираем строку для отображения
  String output = (String)intPart;
  String outputFrac = ((digits - intLength <= 0) || (fracInt == 0)) ? "" : ((String)"." + fracPartStr);
  output += (String)outputFrac;
  output += (isTemperature) ? "C" : "";
  
  // Дополняем символы спереди, если цифр слишком мало, например для "-1" делаем "  -1"
  String spaces = "     ";
  digits = 4;
  if (~output.indexOf(".")) digits += 1;
  if (output.length() < digits) output = spaces.substring(0, digits - output.length()) + output;

  // Формирум данные для показа:
  
  int dig = -1;
  for (int i = 0; i < output.length(); i++) {
    String _char = output.substring(i, i + 1);

    if (_char != ".") dig += 1; // Точка не занимает сегмент - увеличиваем на 1

    int actualdigit = 11; // По умолчанию пустой символ
    if ((_char == "C")) {
      actualdigit = 10;
    }
    else if (_char == " " || _char == ".") {
    }
    else {
      actualdigit = _char.toInt();
    }

    if (_char == ".") {
      arr[dig][7] = 1; // Если нужно - ставим точку
    }
    else  {
      for (int n = 0; n <= 7; n++) {
        arr[dig][n] = seg[actualdigit][n];
      }
    }
  }
}
