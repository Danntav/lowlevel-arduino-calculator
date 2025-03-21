/*
Sites de referencia (muito bons inclusive)

https://soldered.com/learn/low-level-arduino-programmingassembler-blinking-led/?srsltid=AfmBOopgLBH0cTDaBfpI0xX8gFAMzqiY35Ah-dKcr73e0ZpR4RYePtGp
https://sam-mundayhall.medium.com/advanced-arduino-programming-led-blink-part-1-ccaaabfcefe2
https://store-usa.arduino.cc/products/arduino-uno-rev3?srsltid=AfmBOorLYbN74b1h6-qEIchTmkrEFODf0YBex7k90UulnigDTKrOBDee
Arduino assembly: https://gist.github.com/mhitza/8a4608f4dfdec20d3879

*/

#include <avr/io.h>
#include <util/delay.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char tabelaMapeamento[16] = {1, 2, 3, 65 , 4, 5, 6, 66, 7, 8, 9, 67, 42, 0, 35, 68};
long num1 = 0, num2 = 0;
long result = 0;
int oper;
bool inputNumber1 = true;
bool inputOper = true;
bool inputNumber2 = true;


void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);


  DDRD |= 0b00111100; //linhas (D2 a D5) como saída. 
  DDRB &= ~0b00001111; // Colunas (D8 a D11) como entrada, e o resto tanto faz.
  //Antes eu tinha feito DDRB |= 0b00000000, mas nao funcionou

  PORTB |= 0b00001111; //Ativa os resistores de pull-up D8 a D11

  //Pelo que pesquisei, colocar ativo esses resistores garante que nao vai ter flutuacoes na leitura
  //do pino, mantendo sempre em nivel alto, a menos que uma tecla seja pressionada.

}

int keyMatrix() {
  for (int linha = 2; linha <= 5; linha++){  //2 a 5 pois sao os bits definidos do DDRD
    PORTD = ~(1 << linha); //coloca 0 em cada um dos bits, ou seja, cada linha fica em LOW e depois confere cada uma das colunas
    _delay_ms(100); //delay para evitar flicker

    for (int coluna = 0; coluna < 4; coluna++){ // verifica se as colunas D8 ou D9 foram puxadas para zero
      //troca o valor da variavel coluna pois os bits associados tanto a D8 quanto D9 sao os bits 0 e 1 do PORTB.
      if(!(PINB & (1<<coluna))){ // verifica se as colunas D8 a D11 foram puxadas para zero.
        PORTD |= (1 << linha);  
                                //O que estava acontecendo é que eu nao estava resetando o PORT depois que detectava o sinal que a tecla foi apertada
                                //Com isso, a linha tava sempre ficando em LOW.
        return ((linha - 2) * 4) + (coluna + 1); //retorna a "posicao" de cada elemento do teclado

      }
    }
    PORTD |= (1 << linha); //reseta as linhas novamente
  }
  return -1; // Se nao apertar nenhuma tecla, retorna -1, como se fosse um erro.
}


void loop(){

  lcd.setCursor(0,0);
  lcd.print("MICROPROC. CALC.");
  long result = 0;


  while(inputNumber1){
    int tecla = keyMatrix();
    _delay_ms(50);
    if (tecla != -1){
      tecla = int(tabelaMapeamento[tecla-1]);

      if (tecla == 68){
        inputNumber1 = false;
        inputOper = true;
      }
      else if (tecla >= 0 && tecla < 10){
        num1 = num1*10 + tecla;
        if (num1 > 999){
          num1 /= 10;
          inputNumber1 = false;
          inputOper = true;
        }
      lcd.setCursor(0,1);
      lcd.print("    ");
      lcd.setCursor(0,1);
      lcd.print(num1);
      }
    }
  }

  Serial.print("num1:");
  Serial.println(num1);

  while(inputOper){
    int tecla = keyMatrix();
    _delay_ms(50);
    if (tecla != -1){
        tecla = int(tabelaMapeamento[tecla-1]);
        if (tecla == 65 || tecla == 66 || tecla == 35 || tecla == 42){
          oper = tecla;
          switch(oper){
            case 65:lcd.print("+");break;
            case 66:lcd.print("-");break;
            case 42:lcd.print("*");break;
            case 35:lcd.print("/");break;
          }
          inputOper = false;
          inputNumber2 = true;
        }
      }
  }

  Serial.print("oper:");
  Serial.println(oper);

  while(inputNumber2){
    int tecla = keyMatrix();
    _delay_ms(50);
    if (tecla != -1){
      tecla = int(tabelaMapeamento[tecla-1]);
      if (tecla == 68){
        inputNumber2 = false;
 
      }
      else if (tecla >= 0 && tecla < 10){
        num2 = num2*10 + tecla;

        if (num2 > 999){
          num2 /= 10;
          inputNumber2 = false;
        }
      lcd.print(num2);
      }
    }
  }

  Serial.print("num2:");
  Serial.println(num2);
  
  if (!inputNumber1 && !inputOper && !inputNumber2){
    switch(oper){
      case 65: result = num1 + num2; break;
      case 66: result = num1 - num2; break;
      case 42: result = num1 * num2; break;
      case 35:
        if(num2 != 0){
          result = num1 / num2;
        }
        else{
          Serial.println("Error! Divided by 0");
          lcd.setCursor(5,1);
          lcd.print("Erro");
          _delay_ms(2000);
          lcd.clear();
          return;
        }
        break;

      default:
        Serial.println("Not valid operator");
        _delay_ms(2000);
        lcd.clear();
        return;

    }
  
  Serial.print("resultado: ");
  Serial.println(result);

  lcd.setCursor(7,1);
  lcd.print("=");

  lcd.setCursor(8,1);
  lcd.print("    "); 
  lcd.setCursor(8,1);
  lcd.print(result);
  }

_delay_ms(5000);
if (int tecla = keyMatrix()){
  lcd.setCursor(0,1);
  lcd.print("                ");
}

num1 = 0;
num2 = 0;
inputNumber1 = true;
inputOper = false;
inputNumber2 = false;
}
