#include <Arduino.h>
#include <DHT.h>
#include <BluetoothSerial.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
BluetoothSerial BT;
// dentro das {} é o endereço do slave
uint8_t endSlave[] = {0xF4, 0x65, 0x0B, 0x54, 0x70, 0x12};

#define pinDHT 5
#define tipoDHT DHT22
unsigned long tempoAnterior = 0;
const unsigned long intervalo = 2000;
// variaveis / o valor foi pra forçar a 1 escrita no lcd
float tempC_anterior = -999.0;
float umidade_anterior = -999.0;

DHT medidor(pinDHT, tipoDHT);

void setup()
{
  Serial.begin(115200);
  medidor.begin();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);

  lcd.print("   MASTER DHT22    ");

  lcd.setCursor(0, 1);
  lcd.print("temperatura: ");

  lcd.setCursor(0, 2);
  lcd.print("fahrenheit: ");

  lcd.setCursor(0, 3);
  lcd.print("umidade: ");

  if (BT.begin("BTMasterKaua", true)) // diferente do slave o true é depois da virgula
  {
    Serial.println("bluetooth iniciou corretamente");
  }
  else
  {
    Serial.println("erro ao iniciar o bluetooth");
  }
  if (BT.connect(endSlave))
  {
    Serial.println("Conectado com Sucesso");
  }
  else
  {
    Serial.println("erro ao conectar com bluetooth");
  }
}

void loop()
{
  unsigned long tempoAtual = millis();

  if (tempoAtual - tempoAnterior >= intervalo)
  {
    tempoAnterior = tempoAtual;

    float tempC = medidor.readTemperature();
    float tempF = medidor.readTemperature(true);
    float umidade = medidor.readHumidity();

    if (!isnan(tempC) && !isnan(umidade))
    {
      bool mudou = (tempC != tempC_anterior || umidade != umidade_anterior);
      bool passouTempo = (tempoAtual - tempoAnterior >= intervalo);

      // atualiza o lcd (SOMENTE SE HOUVER ALTERAÇÃO) ---
      if (mudou)
      {
        lcd.setCursor(13, 1);
        lcd.print(tempC, 1);
        lcd.print("C  ");

        lcd.setCursor(13, 2);
        lcd.print(tempF, 1);
        lcd.print("F  ");

        lcd.setCursor(13, 3);
        lcd.print(umidade, 1);
        lcd.print("%  ");

        // atualiza os históricos para não escrever de novo no LCD até mudar
        tempC_anterior = tempC;
        umidade_anterior = umidade;
      }

      // envia os dados (A CADA 2 SEGUNDOS OU SE MUDOU)
      if (mudou || passouTempo)
      {
        // reseta o temporizador dos 2 segundos
        if (passouTempo)
        {
          tempoAnterior = tempoAtual;
        }

        Serial.print("temp em C: ");
        Serial.println(tempC);
        Serial.print("temp em F: ");
        Serial.println(tempF);
        Serial.print("umidade: ");
        Serial.println(umidade);
        Serial.println("----------------");

        BT.print("Temperatura: ");
        BT.print(tempC, 1);
        BT.print(" C | Umidade: ");
        BT.print(umidade, 1);
        BT.println(" %");
      }
    }
  }

  if (BT.available()) // função que verifica se chegou algo no bluetooth e se sim é igual a true
  {
    // '\n' é usado pra verificar que chegou no final do texto
    String msgRecebida = BT.readStringUntil('\n'); // verifica se esta conectado, caso esteja conectado ele vai ler o que esta sendo recebido como texto
    msgRecebida.trim();                            // remove todos os caracteres em brancos e especiais
    Serial.printf("mensagem recebida: %s", msgRecebida);
  }
  if (Serial.available()) // verifica se tem algo na serial e mostra no bluetooth
  {
    String msgEnviar = Serial.readStringUntil('\n');
    msgEnviar.trim();      // remove todos os caracteres em brancos e especiais
    BT.println(msgEnviar); // envia pro bluetooth
  }
}