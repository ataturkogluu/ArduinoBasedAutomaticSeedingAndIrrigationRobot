  #include <Servo.h>
  #include "NewPing.h" //Ultrasonik sensör için kütüphane
  #include <DHT.h>  // Sıcaklık ve Nem sensörü için kütüphane
  #include <SoftwareSerial.h>

  SoftwareSerial espSerial(0, 1);  // ESP8266-01 modülünün RX ve TX pinleri
  String ssid = "Agribot";
  String password = "Agribot123";
  String host = "agribot-64c42-default-rtdb.firebaseio.com";
  String auth = "UjHTPVpBIzUTls8Z8Hu5Imgg5nWQnhLfs6dKyWjy";

  // Motor sürücü pin tanımları
  int in1 = 2;   // Motor A yönü
  int in2 = 3;   // Motor A yönü
  int in3 = 4;   // Motor B yönü
  int in4 = 5;  // Motor B yönü

  // İkinci motor sürücü pin tanımları 
  int in5 = 8; //CD ROM Motor A yönü
  int in6 = 9; //CD ROM Motor B yönü
  int in7 = 10; //Delme Motoru A yönü
  int in8 = 11; //Delme Motoru B yönü

  //Su seviyesi ve su pompası
  const int sensorPin = A15; // su seviye sensörü pin'i
  const int pumpPin = 12; // su pompası pin'i

  //Sıcaklık ve Nem sensörü
  #define DHT11Pin 13
  DHT dht(DHT11Pin, DHT11);

  // Toprak ve Nem Sensörü
  #define sensorPin A0   // Toprak nem sensörünün bağlı olduğu analog pin
  int sensorValue = 0;   // Toprak nem sensöründen alınan değer
  int threshold = 500;   // Sensör okuması için belirlenen eşik değeri

  //Ultrasonik sensör
  #define triggerPin 30
  #define echoPin 31
  #define maxDistance 400	// Maximum distance we want to ping for (in centimeters).

  NewPing sonar(triggerPin, echoPin, maxDistance); // NewPing setup of pins and maximum distance.

  int Speed = 100;
  //Servo Motor pinleri 6-7 void setup() içerisinde
  Servo servoMotorToprakNem;
  Servo servoMotorTohumlama;

  void setup() {
    // Motor sürücü pinlerini çıkış olarak ayarla
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
    pinMode(in5, OUTPUT);
    pinMode(in6, OUTPUT);
    pinMode(in7, OUTPUT);
    pinMode(in8, OUTPUT);

    pinMode(pumpPin, OUTPUT); // pompayı çıkış olarak ayarla

    servoMotorToprakNem.attach(6);  // Servo motoru pini 6'ya bağlanır
    servoMotorTohumlama.attach(7);  // Servo motoru pini 7'ya bağlanır
  
    Serial.begin(9600);
    espSerial.begin(9600);
    connectWiFi();
  }

  void loop() {

    if (isWiFiConnected()) {
      Serial.println("WiFi connected");
      // WiFi bağlantısı varsa yapılması gereken işlemler
      String command = getFirebaseCommand(); // Firebase'den komut al
      processCommand(command); // Komuta göre işlem yap

      } else {
      Serial.println("WiFi not connected");
      // WiFi bağlantısı yoksa yapılması gereken işlemler
      //connectWiFi(); // WiFi bağlantısı yoksa tekrar bağlan
    }

    // Firebase'den tarlaGenislik ve tarlaUzunluk değerlerini çek
    int tarlaGenislik, tarlaUzunluk;
    getFirebaseDimensions(tarlaGenislik, tarlaUzunluk);
    int adimUzunlugu = 1;   // Her adımda kaç birim hareket edeceğini belirler
    int toplamAdim = 0;  // Toplam adımların sayısını tutar

    // Sensör verilerini oku
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int soilMoisture = analogRead(sensorPin);
    int waterLevel = analogRead(A3);
    int distance = sonar.ping_cm();

    // Sensör verilerini Firebase'e gönder
    sendFirebaseData("temperature", temperature);
    sendFirebaseData("humidity", humidity);
    sendFirebaseData("water_level", waterLevel);
    sendFirebaseData("distance", distance);
    if (distance != 0 && distance < 20) {
    // Engel var, Firebase'e mesaj gönder
    String message = "Aracın önünde engel var!";
    sendFirebaseData("message", message.toFloat());
    return;  // Döngüden çık
  }

    for (int i = 0; i < tarlaUzunluk; i += adimUzunlugu) {
      for (int j = 0; j < tarlaGenislik; j += adimUzunlugu) {
        // Adım adım ilerle
        ileri();
        delay(1000);  // 1 saniye ilerle, süreyi dikkatlice ayarlayın
        dur();
        // Delme işlemi
        DCDondurme();
        cdromAsagi();
        delay(1000);  // 1 saniye dön ve aşağı in, süreyi dikkatlice ayarlayın
        dur();
        // Yukarı çıkma işlemi
        cdromYukari();
        delay(1000);  // 1 saniye yukarı çık, süreyi dikkatlice ayarlayın
        dur();
        // Tohumlama işlemi
        servoTohumlama();
        sendFirebaseData("soil_moisture", soilMoisture);
        delay(1000);  // Süreyi dikkatlice ayarlayın
        // Sulama işlemi
        suPompasiCalistir();
        delay(1000);  // Süreyi dikkatlice ayarlayın
        suPompasiDurdur();
      }

      if (i != tarlaUzunluk - 1) {  // son satıra gelindiğinde dönmeyi atla
        if (i % 2 == 0) {
          sagaDon();
          ileri();
          delay(1000); 
          dur();
          sagaDon();
      } else {
          solaDon();
          ileri();
          delay(1000);
          dur();
          solaDon();
        }
      }
  
      toplamAdim++;

      // Arazi kontrolü
      if (toplamAdim == tarlaGenislik * tarlaUzunluk) {
        Serial.println("Tum arazi basariyla tarandi!");
        // Tüm araziyi gezdikten sonra dur
        return;
      } else {
        Serial.println("Tohumlama ve Sulama devam ediyor.");
      }
    }
  }

  void connectWiFi() 
  {
    String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
    espSerial.println(cmd);
    delay(2000);
    while (espSerial.available() > 0) {
      String response = espSerial.readStringUntil('\n');
      Serial.println(response);
      if (response.indexOf("OK") != -1) {
        Serial.println("WiFi connected");
      break;
      } else if (response.indexOf("FAIL") != -1) {
        Serial.println("WiFi connection failed");
        break;
      }
    }
  }
  
  void sendFirebaseData(String key, float value) {
    String cmd = "GET /" + key + ".json?auth=" + auth + "&value=" + String(value);
    String request = "AT+CIPSTART=\"TCP\",\"" + host + "\",80";
    espSerial.println(request);
    delay(2000);
    if (espSerial.find("OK")) {
      Serial.println("TCP connection established");
      String sendData = "AT+CIPSEND=";
      sendData += String(cmd.length());
      espSerial.println(sendData);
      delay(1000);
      if (espSerial.find(">")) {
        espSerial.print(cmd);
        espSerial.println((char)26);
        delay(1000);
        while (espSerial.available()) {
          String response = espSerial.readString();
          Serial.println(response);
        }
        espSerial.println("AT+CIPCLOSE");
      }
    }
  }

  String getFirebaseCommand() {
    String cmd = "GET /command.json?auth=" + auth;  // Firebase komutlarının tutulduğu "command" düğümünü hedefleyin
    String request = "AT+CIPSTART=\"TCP\",\"" + host + "\",80";
    espSerial.println(request);
    delay(2000);
    if (espSerial.find("OK")) {
      Serial.println("TCP connection established");
      String sendData = "AT+CIPSEND=";
      sendData += String(cmd.length());
      espSerial.println(sendData);
      delay(1000);
      if (espSerial.find(">")) {
        espSerial.print(cmd);
        espSerial.println((char)26);
        delay(1000);
        while (espSerial.available()) {
          String response = espSerial.readString();
          Serial.println(response);
          if (response.indexOf(":") != -1) {
            int valueIndex = response.indexOf(":") + 1;
            int endIndex = response.indexOf("\r");
            String valueString = response.substring(valueIndex, endIndex);
            return valueString;
          }
        } 
        espSerial.println("AT+CIPCLOSE");
      }
    }
    return "";  // Komut bulunamadıysa boş bir String döndürün
  }

  // Komuta göre işlem yapmak için fonksiyon
  void processCommand(String command) {
    if (command == "A") {
      ileri();
    } else if (command == "B") {
      geri();
    } else if (command == "C") {
      sagaDon();
    } else if (command == "D") {
      solaDon();
    } else if (command == "E") {
      DCDondurme();
      cdromAsagi();
      delay(1000);
      dur();
      cdromYukari();
      delay(1000);
      dur();
    } else if (command == "F") {
      servoTohumlama();
    } else if (command == "G") {
      suPompasiCalistir();
      delay(1000);
      suPompasiDurdur();
    }
  }


  bool isWiFiConnected() 
  {
    String cmd = "AT+CWJAP?";
    espSerial.println(cmd);
    delay(2000);
    while (espSerial.available() > 0) {
      String response = espSerial.readStringUntil('\n');
      if (response.indexOf(ssid) != -1) {
        return true;
      }
    }
    return false;
  }

  void getFirebaseDimensions(int& tarlaGenislik, int& tarlaUzunluk) {
    String cmd = "GET /dimensions.json?auth=" + auth;  // Firebase'den genişlik ve uzunluk değerlerini çekmek için "dimensions" düğümünü hedefleyin
    String request = "AT+CIPSTART=\"TCP\",\"" + host + "\",80";
    espSerial.println(request);
    delay(2000);
    if (espSerial.find("OK")) {
      Serial.println("TCP connection established");
      String sendData = "AT+CIPSEND=";
      sendData += String(cmd.length());
      espSerial.println(sendData);
      delay(1000);
      if (espSerial.find(">")) {
        espSerial.print(cmd);
        espSerial.println((char)26);
        delay(1000);
        while (espSerial.available()) {
          String response = espSerial.readString();
          Serial.println(response);
          if (response.indexOf("tarlaGenislik") != -1 && response.indexOf("tarlaUzunluk") != -1) {
            int genislikIndex = response.indexOf(":") + 1;
            int uzunlukIndex = response.indexOf(":", genislikIndex) + 1;
            int endIndex = response.indexOf("\r");
            String genislikString = response.substring(genislikIndex, uzunlukIndex - 2);
            String uzunlukString = response.substring(uzunlukIndex, endIndex);
            tarlaGenislik = genislikString.toInt();
            tarlaUzunluk = uzunlukString.toInt();
            return;
          }
        }
        espSerial.println("AT+CIPCLOSE");
      }
    }
    // Hata durumunda varsayılan değerler kullanılabilir
    tarlaGenislik = 10;
    tarlaUzunluk = 10;
  }


  // Arabayı ileriye hareket ettirmek için fonksiyon
  void ileri() 
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
  }

  void geri() 
  {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
  }

  // Arabayı sağa döndürmek için fonksiyon
  void sagaDon() 
  {
    digitalWrite(in1, Speed);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, Speed);
  }

  // Arabayı sola döndürmek için fonksiyon
  void solaDon() 
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, Speed);
    digitalWrite(in3, Speed);
    digitalWrite(in4, LOW);
  }

  // Arabayı durdurmak için fonksiyon
  void dur()
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
    Serial.print("Dur'a girdi");
  }

  void toprakNem()
  {
    sensorValue = analogRead(sensorPin);
    Serial.print("Toprak nem: ");
    Serial.println(sensorValue);
  }

  void servoToprakNem()
  {
    servoMotorToprakNem.write(0);
    delay(1000);
    servoMotorToprakNem.write(120);
    delay(1000);
    servoMotorToprakNem.write(0);
    delay(1000);
  }

  void servoTohumlama()
  {
    servoMotorTohumlama.write(0);
    delay(1000);
    servoMotorTohumlama.write(120);
    delay(1000);
    servoMotorTohumlama.write(0);
    delay(1000);  
  }

  void cdromAsagi()
  {
    digitalWrite(in5, HIGH);
    digitalWrite(in6, LOW);
  }

  void cdromYukari()
  {
    digitalWrite(in5, LOW);
    digitalWrite(in6, Speed);
  }

  void DCDondurme()
  {
    digitalWrite(in7,HIGH);
    digitalWrite(in8,LOW);
  }

  void ultrasonikSensor()
  {
    Serial.print("Distance = ");
    Serial.print(sonar.ping_cm());
    Serial.println(" cm");  
  }

  void suSeviyesiSensor()
  {
    int sensorValue = analogRead(sensorPin); // sensör okumasını al
    Serial.print("Su Seviyesi: "); 
    Serial.println(sensorValue);             // su seviyesini seri monitöre yazdır
  }

  void suPompasiCalistir()
  {
    digitalWrite(pumpPin, HIGH);           // pompayı çalıştır
  }

  void suPompasiDurdur()
  {
    digitalWrite(pumpPin, LOW);           // pompayı durdur
  }

  void SicaklikNemSensor()
  {
    float nem = dht.readHumidity();
    float sicaklik = dht.readTemperature();

    Serial.print("Nem ");
    Serial.print(nem);
    Serial.print("% ");

    Serial.print("Sıcaklık ");
    Serial.print(sicaklik);
    Serial.println("*C");
  }