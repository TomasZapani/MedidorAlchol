#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "xxxxx";
const char* password = "xxxxx";

const int sensorPin = 32;    // MQ-3 en GPIO32
const int botonPin = 33;     // Pulsador en GPIO33

int estadoAnterior = LOW;

void mostrarTexto(String texto) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println(texto);
  display.display();
}

void cuentaRegresiva(int segundos) {
  for (int i = segundos; i > 0; i--) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(40, 20);
    display.print("Sopla: ");
    display.println(i);
    display.display();
    delay(1000);
  }
}



String DecisorAlchol(int valor){
  
    if (valor<200){
      return "Normal";
      }
      else if(valor<350){
          return "Límite";
        }
      else if(valor<500){
          return "Alto";
        }
      else{
          return "Muy alto";
        }
  }


  
void setup() {
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);
  pinMode(botonPin, INPUT);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Fallo pantalla OLED"));
    for (;;);
  }

  mostrarTexto("¡Bienvenido!");
  delay(2000);
  mostrarTexto("Conectando WiFi...");
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Conectado. IP: ");
  Serial.println(WiFi.localIP());

  mostrarTexto("Conectado OK");
  delay(1500);
  mostrarTexto("Presione boton\ny sople");
}

void loop() {
  int estadoActual = digitalRead(botonPin);

  if (estadoActual == HIGH && estadoAnterior == LOW) {
    Serial.println("Boton presionado");

    cuentaRegresiva(3);

    int valorAlchol = analogRead(sensorPin);
    String clasificacion = DecisorAlchol(valorAlchol);
    
    Serial.print("Nivel de alcohol: ");
    Serial.println(valorAlchol);

    mostrarTexto("Listo...\nEnviando datos");

    // Envío WhatsApp
    WiFiClient cliente;
    if (cliente.connect("api.callmebot.com", 80)) {
      String mensaje = "Nivel de alcohol: " + String(valorAlchol) + " - Estado: " + clasificacion;

        // Reemplazar espacios con %20 (muy simple codificación básica)
      mensaje.replace(" ", "%20");
      mensaje.replace(":", "%3A");
      mensaje.replace("-", "%2D");

     String url = "/whatsapp.php?phone=+xxxxxxxxx&text=" + mensaje + "&apikey=xxxxx";

      cliente.println("GET " + url + " HTTP/1.1");
      cliente.println("Host: api.callmebot.com");
      cliente.println("Connection: close");
      cliente.println();

      // Leer respuesta (opcional)
      while (cliente.connected()) {
        String line = cliente.readStringUntil('\n');
        Serial.println(line);
      }

      cliente.stop();
      mostrarTexto("Datos enviados\ncon exito");
    } else {
      Serial.println("❌ Error al conectar con CallMeBot");
      mostrarTexto("Error al enviar");
    }

    delay(3000);
    mostrarTexto("Presione boton\ny sople");
  }

  estadoAnterior = estadoActual;
}
