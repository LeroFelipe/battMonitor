#include <UIPEthernet.h>

int test = 0;
int i = 0;
long calcTime[2] = {0};
float leituras[10] = {0};
float autonomy = 0.00;
float bat_percentage = 0.00;
float voltageStable = 0.00;
unsigned long ultimaLeitura = 0;
const unsigned long intervaloLeitura = 1000;
String str_autonomy;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(172, 20, 252, 2);
EthernetServer server(55000);

void setup() {
  //Serial.begin(9600);
  randomSeed(analogRead(1));
  pinMode(A0, INPUT);
  Ethernet.begin(mac, ip);
  server.begin();
}

void loop() {
  
  EthernetClient client = server.available();

  unsigned long tempoAtual = millis();

  if (tempoAtual - ultimaLeitura >= intervaloLeitura) {
    readBattery();
    ultimaLeitura = tempoAtual;
  }
  if (client) {
    processClientRequest(client);
  }
}

void processClientRequest(EthernetClient client){
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();

    // Send the HTML page
    client.println("<!DOCTYPE html>");
    client.println("<html lang=\"pt-br\"><head><meta charset=\"UTF-8\"><title>Battery Monitor</title>\
    <style>\
    body {\
      background-color: #333;\
      color: #fff;\
      font-family: Arial, sans-serif;\
      text-align: center;\
    }\
    .painel{\
      border: 2px solid #00FFFF;\
      border-radius: 6%;\
      max-width: 60%;\
      margin: 0 auto;\
    }\   
    h1 {font-size: 6vw;}\
    h2 {font-size: 3vw;}\
    </style></head>");

    client.println("<body>");
    client.println("<div class=\"painel\">");
    client.println("<h1>Banco de Baterias:</h1>");
    client.println("<h2>Tensão do Banco: " + String(voltageStable) + "V</h2>");
    client.println("<h2>Banco em: " + String(bat_percentage) + "%</h2>");
    client.println("<h2>Autonomia: " + str_autonomy + "</h2>");
    client.println("</div></body></html>");
    client.stop();
}

void readBattery(){

  float bat_percent_ini = 0;
  float sensorValue;
  float voltage = 0.00;

  if (i < 10){
    sensorValue = analogRead(A0);
    voltage = ((sensorValue * 54) / 1024) - 3.5;
    voltage = round(voltage * 10.0) / 10.0;
    leituras[i] = voltage;
    i++;
  }else{
    voltageStable = encontrarValorMaisRepetido(leituras, 10);
    if (voltageStable >= 50.3){ voltageStable = voltageStable + 3;}
    bat_percentage = mapfloat(voltageStable, 41, 53.4, 0, 100);
    /*Serial.print(" Estável: ");
    Serial.print(voltageStable);
    Serial.println(" ");*/
    bubbleSort(leituras, 10);
    i = 0;
  }

  if (bat_percentage > bat_percent_ini + 2 ){
    calcTime[0] = 0;
    calcTime[1] = 0;
    autonomy = 0;
    test = 0;
  }

  if (test == 0){
    bat_percent_ini = bat_percentage;
    test = 1;
    str_autonomy = "Rede Elétrica";
  }

  if ((bat_percentage <= bat_percent_ini - 2.5) && test == 1){
    bat_percent_ini = bat_percentage;
    calcTime[0] = millis();
    test = 2;
    str_autonomy = "Calculando...";
  }

  if ((bat_percentage <= bat_percent_ini - 2.5) && test == 2){
    calcTime[1] = millis();
    float timeDiff = ((calcTime[1] - calcTime[0]) / 3600000.00);
    float battDiff = (bat_percentage / (bat_percent_ini - bat_percentage));
    autonomy = (timeDiff * battDiff); 
    test = 0;
  }

  if (bat_percentage >= 100){
    bat_percentage = 100;
  }

  if (bat_percentage <= 0){
    bat_percentage = 1;
  }
    
  if(autonomy > 0.00){
    int inteiro = autonomy;
    float decimal = autonomy - inteiro;
    int minutos = int(decimal*60);
    str_autonomy = String(inteiro) + "h" + String(minutos) + "min";
  }
}

float encontrarValorMaisRepetido(float arr[], int tamanho) {

  float valorAtual, valorMaisRepetido;
  int contagemAtual, contagemMaisRepetida = 0;
  
  for (int j = 0; j < tamanho; j++) {
    valorAtual = arr[j];
    contagemAtual = 1; // Inicialize a contagem para o valor atual
    
    // Verifique as próximas posições da array para encontrar repetições
    for (int k = j + 1; k < tamanho; k++) {
      if (arr[k] == valorAtual) {
        contagemAtual++;
      }
    }    
    // Se a contagem atual for maior que a contagem mais repetida anteriormente,
    // atualize o valor mais repetido e a contagem mais repetida.
    if (contagemAtual > contagemMaisRepetida) {
      contagemMaisRepetida = contagemAtual;
      valorMaisRepetido = valorAtual;
    }
  }
  
  return valorMaisRepetido;
}

void bubbleSort(float arr2[], int n) {
  bool trocou; // Variável para rastrear se houve troca em uma iteração
  do {
    trocou = false;
    for (int l = 1; l < n; l++) {
      // Compara elementos adjacentes e troca se estiverem fora de ordem
      if (arr2[l - 1] > arr2[l]) {
        // Troca
        float temp = arr2[l - 1];
        arr2[l - 1] = arr2[l];
        arr2[l] = temp;
        trocou = true; // Indica que houve uma troca nesta iteração
      }
    }
  } while (trocou); 
  
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
