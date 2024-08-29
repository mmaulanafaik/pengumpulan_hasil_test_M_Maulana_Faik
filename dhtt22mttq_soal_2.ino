//#include <DHT22.h>
#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Definisi pin dan tipe sensor DHT
#define DHTPIN 4       // Pin GPIO ESP32 tempat DHT22 terhubung
#define DHTTYPE DHT22  // Tipe sensor DHT

// Membuat objek DHT
DHT dht(DHTPIN, DHTTYPE);

// Membuat objek WiFiClient dan PubSubClient untuk koneksi WiFi dan MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Kredensial WiFi
const char* ssid = "Kos Bu Soeyahni";       // SSID dari jaringan WiFi
const char* password = "11223745"; // Password dari jaringan WiFi

// Kredensial MQTT
const char* mqtt_broker = "broker.hivemq.com"; // URL atau IP dari MQTT broker
const char* mqtt_username = "";                // Username untuk autentikasi (jika ada)
const char* mqtt_password = "";                // Password untuk autentikasi (jika ada)
const int mqtt_port = 1883;                    // Port yang digunakan oleh MQTT broker

void setup() {
  Serial.begin(115200); // Inisialisasi komunikasi serial dengan baud rate 115200
  dht.begin(); // Inisialisasi sensor DHT
  setup_wifi();// Menghubungkan ke jaringan WiFi
  
  // Mengatur server MQTT dan callback untuk pesan masuk
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
}

// Fungsi untuk menghubungkan ESP32 ke jaringan WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);

  // Mulai koneksi ke jaringan WiFi menggunakan SSID dan password
  WiFi.begin(ssid, password);

  // Tunggu hingga terhubung ke WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi terhubung");
}

// Fungsi untuk menghubungkan kembali ke MQTT broker jika koneksi terputus
void reconnect() {
  // Loop hingga terhubung kembali ke MQTT broker
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    
    // Mencoba menghubungkan ke broker dengan client ID dan kredensial MQTT
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {
      Serial.println("terhubung");
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      Serial.println(" mencoba lagi dalam 5 detik");
      
      // Tunggu 5 detik sebelum mencoba lagi
      delay(5000);
    }
  }
}

// Fungsi callback yang akan dipanggil saat pesan diterima di topik yang dilanggan
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Pesan diterima di topik [");
  Serial.print(topic);
  Serial.print("] : ");
  
  // Menampilkan pesan yang diterima karakter demi karakter
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println();
}

// Fungsi utama yang akan terus-menerus berjalan setelah inisialisasi selesai
void loop() {
  // Pastikan ESP32 tetap terhubung ke MQTT broker
  if (!client.connected()) {
    reconnect();
  }
  
  // Melakukan loop untuk menjaga koneksi MQTT dan memproses pesan masuk
  client.loop();

  // Membaca data kelembapan dan suhu dari sensor DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Periksa apakah data yang dibaca valid
  if (isnan(h) || isnan(t)) {
    Serial.println("Gagal membaca dari sensor DHT!");
    return; // Keluar dari fungsi loop jika pembacaan gagal
  }

  // Mendefinisikan topik untuk pengiriman data suhu dan kelembapan
  String temperatureTopic = "daq1/suhu";
  String humidityTopic = "daq1/kelembapan";

  // Mengirimkan data suhu ke topik MQTT yang ditentukan
  client.publish(temperatureTopic.c_str(), String(t).c_str());
  
  // Mengirimkan data kelembapan ke topik MQTT yang ditentukan
  client.publish(humidityTopic.c_str(), String(h).c_str());

  // Menunggu 2 detik sebelum mengulangi pengiriman data
  delay(2000);
}
