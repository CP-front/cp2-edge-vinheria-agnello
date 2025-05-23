#include <EEPROM.h>  // Inclusão da biblioteca necessária
#include <Wire.h>  // Inclusão da biblioteca necessária
#include <LiquidCrystal_I2C.h>  // Inclusão da biblioteca necessária
#include "DHT.h"  // Inclusão da biblioteca necessária
#include <RTClib.h>  // Inclusão da biblioteca necessária
 
#define LOG_OPTION 1     // Opção para ativar a leitura do log
#define SERIAL_OPTION 0  // Opção de comunicação serial: 0 para desligado, 1 para ligado
#define UTC_OFFSET 0    // Ajuste de fuso horário para UTC-3
 
// Configurações do DHT22
#define DHTPIN 7
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
 
LiquidCrystal_I2C lcd(0x27, 16, 2); // Endereço de acesso: 0x3F ou 0x27
RTC_DS1307 RTC;
 
// Pino analógico conectado ao sensor LDR (fotoresistor)
const short int pinoLDR = A0;
 
// LEDs de sinalização (verde = ok, amarelo = alerta, vermelho = perigo)
const short int ledVermelho = 6;
const short int ledAmarelo  = 5;
const short int ledVerde    = 4;
 
// Buzzer que emite som de alerta
const short int pinoBuzzer = 3;
 
// Botão físico usado para calibração dos níveis de luz
const short int botaoSelecionar = 2;
const short int botaoProximo = A2;
const short int botaoVoltar = A3;
 
// Limites durante o setup
int limiarBaixo = 30;
int limiarAlto = 70;
 
// Tempo de espera entre leituras (em milissegundos)
const int tempoPrecisao = 100;
 
// Configurações da EEPROM
const int maxRecords = 100;
const int recordSize = 10; // Tamanho de cada registro em bytes
int startAddress = 0;
int endAddress = maxRecords * recordSize;
int currentAddress = 0;
 
int lastLoggedMinute = -1;
 
// Triggers de temperatura e umidade
float trigger_t_min = 10.0; // Exemplo: valor mínimo de temperatura
float trigger_t_max = 16.0; // Exemplo: valor máximo de temperatura
float trigger_u_min = 60.0; // Exemplo: valor mínimo de umidade
float trigger_u_max = 80.0; // Exemplo: valor máximo de umidade
 
// Animação logo Visionary Solutions
int i;
//Cria caracter A personalizado
byte AA[8] = {
  B00001,
  B00010,
  B00100,
  B00100,
  B01000,
  B10000,
  B10000,
  B10000
};
//Cria caracter B personalizado
byte BB[8] = {
  B10000,
  B10000,
  B10000,
  B01000,
  B00100,
  B00100,
  B00010,
  B00001
};
//Cria caracter V personalizado
byte VV[8] = {  
  B10001,
  B10001,
  B10001,
  B10001,
  B10011,
  B01010,
  B01010,
  B00110,
};
 
//Cria caracter S personalizado
byte S[8] = {  
  B11000,
  B11111,
  B10000,
  B10000,
  B01111,
  B00010,
  B00110,
  B11000
};
 
//Cria caracter G personalizado
byte GG[8] = {  
  B10000,
  B01000,
  B00100,
  B00100,
  B00010,
  B00001,
  B00001,
  B00001
};
//Cria caracter H personalizado
byte H[8] = {
  B00001,
  B00001,
  B00001,
  B00010,
  B00100,
  B00100,
  B01000,
  B10000
};
 
// Controle de menu
int opcaoSelecionada = 0;
String opcoesMenu[] = {"LUM", "UMI", "TEMP"};
const int totalOpcoes = 3;
bool mostrandoInfo = false;
 
void setup() {
  // Configura os pinos como entrada ou saída
  pinMode(pinoLDR, INPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(pinoBuzzer, OUTPUT);
  pinMode(botaoSelecionar, INPUT_PULLUP); // Usa resistor interno de pull-up
  pinMode(botaoProximo, INPUT_PULLUP);
  pinMode(botaoVoltar, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
 
  dht.begin();
  Serial.begin(9600);

  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0xFF);  // Escreve 0xFF na posição i
  }

  Serial.println("EEPROM limpa!");

  int ldrValue = analogRead(pinoLDR);  // Leitura do LDR (0-1023)
  int porcentagemLum = map(ldrValue, 0, 1023, 100, 0);
  float humidity = dht.readHumidity(); // Leitura da umidade
  float temperature = dht.readTemperature(); // Leitura da temperatura
 
  lcd.init();   // Inicialização do LCD
  lcd.backlight();  // Ligando o backlight do LCD
 
  RTC.begin();    // Inicialização do Relógio em Tempo Real
  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // RTC.adjust(DateTime(2024, 5, 6, 08, 15, 00));  // Ajustar a data e hora apropriadas uma vez inicialmente, depois comentar
 
  EEPROM.begin();
 
  //Cria os caractéres especiais na memória do LCD
  lcd.createChar(0, AA);
  lcd.createChar(1, BB);
  lcd.createChar(2, VV);  
  lcd.createChar(3, S);  
  lcd.createChar(4, GG);  
  lcd.createChar(5, H);
  // Inicializa o display LCD com 16 colunas e 2 linhas
  lcd.begin(16, 2);
 
  //Posiciona o cursor na primeira coluna(0) e na primeira linha(0) do LCD  
  lcd.setCursor(16, 0);  
  lcd.print("   Visionary "); //Escreve no LCD "Visionary!"  
  lcd.setCursor(16, 1); //Posiciona o cursor na primeira coluna(0) e na segunda linha(1) do LCD  
  lcd.print("    Solutions "); //Escreve no LCD "Solutions"  
  delay(3000);  
 
  //efeito de deslocamento para esquerda  
  for (i =0; i <16; i++){    
  lcd.scrollDisplayLeft();    
  delay(20);  
  }  
  delay(1000); //Aguarda 1 segundo  
 
  //Blinkando  
  delay(400);    
  lcd.noDisplay();  
  delay(400);  
  lcd.display();  
  delay(400);  
  lcd.noDisplay();  
  delay(400);  
  lcd.display();  
  delay(400);  
  lcd.display();  
  delay(400);  
 
  //para esquerda  
  for (i =0; i <16; i++){    
  lcd.scrollDisplayLeft();    
  delay(40);  
  }    
 
  lcd.clear();  
 
  //caracteres especiais  
  lcd.setCursor(17,0);  
  lcd.write(byte(2));
  lcd.setCursor(18,1);
  lcd.write(byte(3));
  lcd.setCursor(16,0);
  lcd.write(byte(0));
 
  lcd.setCursor(16,1);
  lcd.write(byte(1));
  lcd.setCursor(19,0);
  lcd.write(byte(4));
  lcd.setCursor(19,1);
  lcd.write(byte(5));
  delay(500);  
 
  // para esquerda  
  for (i =0; i <10; i++){    
  lcd.scrollDisplayLeft();    
  delay(20); //Aguarda 1 segundo  
  }  
  delay(1000); //1 segundo      
 
  //Pisca  
  delay(400);  
  lcd.noDisplay();  
  delay(400);  
  lcd.display();  
  delay(400);  
  lcd.noDisplay();  
  delay(400);  
  lcd.display();  
  delay(400);  
  lcd.display();  
  delay(400);  
 
  //deslocamento para Direita  
  for (i =0; i <12; i++){    
  lcd.scrollDisplayRight();    
  delay(20); //Aguarda 1 segundo  
  }  
  delay(500); //Aguarda 1/0.5 segundo  
 
  lcd.clear();
 
  lcd.print("      BEM"); // Mensagem inicial
  lcd.setCursor(0, 1); // Vai para a segunda linha
  lcd.print("     VINDO!");
  delay(3000); // Exibe a mensagem por 3 segundos
  lcd.clear(); // Limpa o display
 
  // Inicia processo de calibração do LDR
  setupLDR();
  delay(1000);
}

// Adição de variáveis globais para média da luminosidade
unsigned long tempoInicioMedia = 0;
const unsigned long intervaloMedia = 10000; // 10 segundos
int somaLuminosidade = 0;
int leiturasRealizadas = 0;
int mediaLuminosidade = 0;
 
void loop() {
    DateTime now = RTC.now();
 
    // Calculando o deslocamento do fuso horário
    int offsetSeconds = UTC_OFFSET * 3600; // Convertendo horas para segundos
    now = now.unixtime() + offsetSeconds; // Adicionando o deslocamento ao tempo atual
 
    // Convertendo o novo tempo para DateTime
    DateTime adjustedTime = DateTime(now);
 
    if (LOG_OPTION) get_log();
 
    // Verifica se o minuto atual é diferente do minuto do último registro
    if (adjustedTime.minute() != lastLoggedMinute) {
        lastLoggedMinute = adjustedTime.minute();
 
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(1000);                       // wait for a second
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
        delay(1000);                       // wait for a second
 
        // Ler os valores de temperatura e umidade
        int ldrValue = analogRead(pinoLDR);  // Leitura do LDR (0-1023)
        int porcentagemLum = map(ldrValue, 0, 1023, 100, 0);
        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();
 
        if (!isnan(humidity) && !isnan(temperature)) {
            if (temperature < trigger_t_min || temperature > trigger_t_max || humidity < trigger_u_min || humidity > trigger_u_max) {
                int tempInt = (int)(temperature * 100);
                int humiInt = (int)(humidity * 100);
                int lumInt = porcentagemLum;
                EEPROM.put(currentAddress, now.unixtime());
                EEPROM.put(currentAddress + 4, tempInt);
                EEPROM.put(currentAddress + 6, humiInt);
                EEPROM.put(currentAddress + 8, lumInt);
                getNextAddress();
            }
        }
    }
 
    if (SERIAL_OPTION) {
        Serial.print(adjustedTime.day());
        Serial.print("/");
        Serial.print(adjustedTime.month());
        Serial.print("/");
        Serial.print(adjustedTime.year());
        Serial.print(" ");
        Serial.print(adjustedTime.hour() < 10 ? "0" : ""); // Adiciona zero à esquerda se hora for menor que 10
        Serial.print(adjustedTime.hour());
        Serial.print(":");
        Serial.print(adjustedTime.minute() < 10 ? "0" : ""); // Adiciona zero à esquerda se minuto for menor que 10
        Serial.print(adjustedTime.minute());
        Serial.print(":");
        Serial.print(adjustedTime.second() < 10 ? "0" : ""); // Adiciona zero à esquerda se segundo for menor que 10
        Serial.print(adjustedTime.second());
        Serial.print("\n");
    }
 
    if (mostrandoInfo) {
      mostrarInformacao(opcaoSelecionada);
      if (digitalRead(botaoVoltar) == LOW) {
        mostrandoInfo = false;
        delay(100);
      }
    } else {
        mostrarMenu();
        navegarMenu();
    }

    // ===== Cálculo da média da luminosidade em paralelo =====
    unsigned long tempoAtual = millis();
    if (tempoAtual - tempoInicioMedia < intervaloMedia) {
        int leituraLDR = analogRead(pinoLDR);
        int porcentagemLum = map(leituraLDR, 0, 1023, 100, 0);
        somaLuminosidade += porcentagemLum;
        leiturasRealizadas++;
    } else {
        if (leiturasRealizadas > 0) {
            mediaLuminosidade = somaLuminosidade / leiturasRealizadas;
            Serial.print("Media Luminosidade 10s: ");
            Serial.println(mediaLuminosidade);
        }
        tempoInicioMedia = tempoAtual;
        somaLuminosidade = 0;
        leiturasRealizadas = 0;
    }
    delay(2000);
}
 
void mostrarMenu() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Menu:");
    lcd.setCursor(0, 1);
    lcd.print("Escolha > " + opcoesMenu[opcaoSelecionada]);
    delay(200);
}
 
void navegarMenu() {
  if (digitalRead(botaoProximo) == LOW) {
    opcaoSelecionada = (opcaoSelecionada + 1) % totalOpcoes;
    delay(300);
  }
  if (digitalRead(botaoSelecionar) == LOW) {
    mostrandoInfo = true;
    delay(300);
  }
}
 
void mostrarInformacao(int opcao) {
  lcd.clear();
  switch (opcao) {
    case 0: { // Luminosidade
      int leituraLDR = analogRead(pinoLDR);
      int porcentagemLum = map(leituraLDR, 0, 1023, 100, 0);
      lcd.setCursor(0, 0);
      lcd.print("Lum:");
      lcd.print(porcentagemLum);
      lcd.print("% ");
      statusLuminosidade(porcentagemLum);
      break;
    }
    case 1: { // Umidade
      float hum = dht.readHumidity();
      lcd.setCursor(0, 0);
      lcd.print("Umid:");
      lcd.print(hum, 1);
      lcd.print("% ");
      verificarStatus(hum, trigger_u_min, trigger_u_max);
      break;
    }
    case 2: { // Temperatura
      float temp = dht.readTemperature();
      lcd.setCursor(0, 0);
      lcd.print("Temp:");
      lcd.print(temp, 1);
      lcd.print("C ");
      verificarStatus(temp, trigger_t_min, trigger_t_max);
      break;
    }
  }
}
 
void statusLuminosidade(int valor) {
  lcd.setCursor(0, 1);
  if (valor <= limiarBaixo) {
    lcd.print("Status: OK     ");
    leds(1, 0, 0);
    delay(1000);
    noTone(pinoBuzzer);
  } else if (valor <= limiarAlto) {
    lcd.print("Status: Alerta");
    leds(0, 1, 0);
    delay(1000);
    tone(pinoBuzzer, 1000);  // tom médio
    delay(3000);
    noTone(pinoBuzzer);
  } else {
    lcd.print("Status: Perigo");
    leds(0, 0, 1);
    delay(1000);
    tone(pinoBuzzer, 500);   // tom grave
    delay(1000);
    noTone(pinoBuzzer);
  }
}
 
void verificarStatus(float valor, float min, float max) {
  lcd.setCursor(0, 1);
  if (valor >= min && valor <= max) {
    lcd.print("Status: OK     ");
    leds(1, 0, 0);
    noTone(pinoBuzzer);
  } else if ((valor > max && valor <= max + 5) || (valor < min && valor >= min - 5)) {
    lcd.print("Status: Alerta ");
    leds(0, 1, 0);
    tone(pinoBuzzer, 1000);
    delay(3000);
    noTone(pinoBuzzer);
  } else {
    lcd.print("Status: Perigo ");
    leds(0, 0, 1);
    tone(pinoBuzzer, 500);
    delay(1000);
    noTone(pinoBuzzer);
  }
}
 
void leds(int verde, int amarelo, int vermelho) {
  digitalWrite(ledVerde, verde);
  digitalWrite(ledAmarelo, amarelo);
  digitalWrite(ledVermelho, vermelho);
}
 
void getNextAddress() {
    currentAddress += recordSize;
    if (currentAddress >= endAddress) {
        currentAddress = 0; // Volta para o começo se atingir o limite
    }
}
 
void get_log() {
    Serial.println("Data stored in EEPROM:");
    Serial.println("Timestamp\t\tTemperature\tHumidity");
 
    for (int address = startAddress; address < endAddress; address += recordSize) {
        long timeStamp;
        int tempInt, humiInt, lumInt;
 
        // Ler dados da EEPROM
        EEPROM.get(address, timeStamp);
        EEPROM.get(address + 4, tempInt);
        EEPROM.get(address + 6, humiInt);
        EEPROM.get(address + 8, lumInt);
 
        // Converter valores
        float temperature = tempInt / 100.0;
        float humidity = humiInt / 100.0;
 
        // Verificar se os dados são válidos antes de imprimir
        if (timeStamp != 0xFFFFFFFF) { // 0xFFFFFFFF é o valor padrão de uma EEPROM não inicializada
            //Serial.print(timeStamp);
            DateTime dt = DateTime(timeStamp);
            Serial.print(dt.timestamp(DateTime::TIMESTAMP_FULL));
            Serial.print("\t");
            Serial.print(temperature);
            Serial.print(" C\t\t");
            Serial.print(humidity);
            Serial.println(" %\t");
            Serial.print(lumInt);
            Serial.println(" % (Luz)");
        }
    }
}
 
// Inicialização do sensor LDR, guiado por mensagens no display
void setupLDR() {
  lcd.clear();
  lcd.setCursor(0, 0);
 
  // Mensagem de início do setup
  char mensagem[] = "Iniciando SETUP";
  animacaoPrint(mensagem, 150);
  lcd.setCursor(0, 1);
  lcd.print("Pressione");
 
  // Pressionar para continuar
  while (digitalRead(botaoSelecionar) == HIGH);
  delay(100);
  while (digitalRead(botaoSelecionar) == LOW);
}
 
// Texto no LCD com efeito de digitação
// Aparece a logo
void animacaoPrint(char mensagem[], int tempo) {
  lcd.clear();
 
  for (int i = 0; i < strlen(mensagem); i++) {
    lcd.print(mensagem[i]);
    delay(tempo);
 
    if (i > 14) {
      lcd.setCursor(i - 15, 1);
    }
  }
}