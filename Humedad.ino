#include <Wire.h>               
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd( 0x27, 16, 2 );
#define LED_VERDE         0x02  // Puerto de conexión LED verde 100% de humedad.
#define LED_AZUL          0x03  // Puerto de conexión LED azul 75% de humedad.
#define LED_BLANCO        0x04    // Puerto de conexión LED blanco 50% de humedad.
#define LED_AMARILLO      0x05    // Puerto de conexión LED amarillo 25% de humedad.
#define LED_ROJO          0x06    // Puerto de conexión LED rojo 0% de humedad.
#define SOIL                A0    // Puerto analógico para el sensor de humeda. 
#define MAX_SOIL_VALUE       1020    // Valor máximo obtenido en la simulación.

/*
  Nota: Algunas caracterírticas varían entre el sensor real del simulado,
        el real en la lectura del ADC muestra 0 en su máxima lectura de humedad
          caso contrario a la simulación, es necesario modificar la lógica de la lectura.
*/

/* Reemplazo en el código    condicional      se cumple/no se cumple */
#define controlRojo( a )     a <= 12      ? HIGH : LOW  // Si se recibe un valor menor a 12 entonces equivale a un HIGH(alto, 1, verdadero), en caso contrario un LOW(bajo, 0, falso).
#define controlAmarillo( a ) a > 12 && a <= 25  ? HIGH : LOW //condicion para el 25%
#define controlBlanco( a )   a > 25 && a <= 50  ? HIGH : LOW //condicion para el 50%
#define controlAzul( a )     a > 50 && a <= 75  ? HIGH : LOW //condicion para el 75%
#define controlVerde( a )    a > 75 && a <= 100 ? HIGH : LOW //condicion para el 100%


/* Variable que indica el valor analógico obtenido del sensor */
float Soilmoisture = 0;

byte caracter0 [ ] = { //gotita 0%
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000                   /* ⬜⬜⬜⬜⬜ */
};
byte caracter1 [ ] = { //gotita 25%
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B01110,                  /* ⬜⬛⬛⬛⬜ */
  B00000                   /* ⬜⬜⬜⬜⬜ */
};
byte caracter2 [ ] = {//gotita 50%
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B11111,                  /* ⬛⬛⬛⬛⬛ */
  B11111,                  /* ⬛⬛⬛⬛⬛ */
  B01110,                  /* ⬜⬛⬛⬛⬜ */
  B00000                   /* ⬜⬜⬜⬜⬜ */
};
byte caracter3 [ ] = {//gotita 75%
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B11111,                  /* ⬛⬛⬛⬛⬛ */
  B11111,                  /* ⬛⬛⬛⬛⬛ */
  B11111,                  /* ⬛⬛⬛⬛⬛ */
  B01110,                  /* ⬜⬛⬛⬛⬜ */
  B00000                   /* ⬜⬜⬜⬜⬜ */
};
byte caracter4 [ ] = {//gotita 100%
  B00000,                  /* ⬜⬜⬜⬜⬜ */
  B00100,                  /* ⬜⬜⬛⬜⬜ */                 
  B01110,                  /* ⬜⬛⬛⬛⬜ */                
  B11111,                  /* ⬛⬛⬛⬛⬛ */                
  B11111,                  /* ⬛⬛⬛⬛⬛ */
  B11111,                  /* ⬛⬛⬛⬛⬛ */
  B01110,                  /* ⬜⬛⬛⬛⬜ */
  B00000                   /* ⬜⬜⬜⬜⬜ */
};


void setup( void ) {

  lcd.init ( ); //Inicializa el modulo adaptador LCD a I2C       
  lcd.createChar ( 0, caracter0 );  //Crea un carácter personalizado para su uso en la pantalla LCD
  lcd.createChar ( 1, caracter1 ); //num es el número de carácter y datos es una matriz que contienen los pixeles del carácter
  lcd.createChar ( 2, caracter2 );
  lcd.createChar ( 3, caracter3 );
  lcd.createChar ( 4, caracter4 );
  
  Serial.begin ( 115200 );  // Baudrate a 115200 bits/segundo. (velocidad de tranferencia de informarmacion)
  uint8_t LEDS [ ] = { LED_VERDE, LED_AZUL, LED_BLANCO, LED_AMARILLO, LED_ROJO }; // se declara un array cuyo cada miembro contiene el pin digital al cual está conectado su LED correspondiente.
  
  for ( uint8_t i = 0; i < sizeof( LEDS ); i++ ) {  // Se recorre el array; sizeof retorna el número de bytes que necesita en memoria, solo en este caso como cada miembro puede almacenar 8 bits, retorna el tamaño del array.
   
    pinMode ( LEDS [ i ], OUTPUT );         // Se declara cada pin almacenado en el array como salida.
    digitalWrite ( LEDS [ i ], LOW );       // Se inicializa cada pin almacenado en el array en BAJO.(apagado)
    
 
    
  }
  
  pinMode ( SOIL, INPUT );  // El único pin de entrada en este caso es el que pertenece al sensor de humedad del suelo.
  
}

/* Función que retorna el porcentaje de humedad obtenida por el sensor */
float getSoilmoisture ( void ) {
  
  return ( float ) analogRead( SOIL ) * 100 / MAX_SOIL_VALUE; // Se retorna el valor correspondiente en una regla de 3 para calcular el 100% según la cantidad máxima que proporciona el sensor, en este caso es: 876;
      
}

void indicator ( uint8_t value )  { //en la linea 134 se manda a llamar la funcion donde recibe Soilmoisture que es donde se almacena el % del sensor
  //dependiendo del value se ejecuta la variable con ayuda del reemplazo del código condicional 
  digitalWrite( LED_ROJO, controlRojo( value ) ); //0% seco
  digitalWrite( LED_AMARILLO, controlAmarillo ( value ) );//25% 
  digitalWrite( LED_BLANCO, controlBlanco ( value ) );//50% 
  digitalWrite( LED_AZUL, controlAzul ( value ) );//75%
  digitalWrite( LED_VERDE, controlVerde ( value ) );  //100% humedad ideal
  
}

void loop( void ) {
   
  //Serial.println ( (float)analogRead ( A0 ) *100 / 876 );
  Soilmoisture = getSoilmoisture ( );   // Se obtiene el porcentaje de humedad.
  Soilmoisture=(100-Soilmoisture); //conversion para invertir el valor  0%=seco  y 100% humedo

  /* Se muestra en la consola serial. */
  Serial.print ( "La humedad del suelo es de: " );
  Serial.print ( Soilmoisture );
  Serial.println ( " %" );
  /* Se manda encender los LEDs de acuerdo al nivel de humedad. */
  indicator ( Soilmoisture );
  
  delay ( 250 ); //se espera 2 segundos y medio
 
  
  if(  Soilmoisture <= 12.00 ){ //cuando esta en 0% se pondra estos valore en la LCD
    lcd.backlight(); //Enciende la Luz del Fondo del LCD
    lcd.setCursor ( 0, 0 ); //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.
    lcd.write ( 0 ); //muestra el icono en la pantalla
    lcd.setCursor ( 2, 0 ); //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.
    lcd.print ( "La humedad del" ); //manda el mensaje a mostar
    lcd.setCursor ( 0, 1 );  //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.   
    lcd.print ( "suelo es de:" );//manda el mensaje a mostar
    lcd.setCursor ( 12, 1 );    //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.
    lcd.print ( Soilmoisture ); //manda a mostrar el valor del sensor
    delay ( 1000 );  //espera 1 seg           
    lcd.clear ( );    //borra pantalla    
  }else{
      if(12.00 < Soilmoisture && Soilmoisture <=25.00 ){//cuando esta en 25% se pondra estos valore en la LCD
        lcd.backlight();  //Enciende la Luz del Fondo del LCD    
        lcd.setCursor ( 0, 0 ); //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.
        lcd.write ( 1 );//Enciende la Luz del Fondo del LCD
        lcd.setCursor ( 2, 0 ); //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.    
        lcd.print ( "La humedad del" );//manda el mensaje a mostar
        lcd.setCursor ( 0, 1 ); //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.    
        lcd.print ( "suelo es de:" );//manda el mensaje a mostar
        lcd.setCursor ( 12, 1 );//establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.    
        lcd.print ( Soilmoisture );  //manda a mostrar el valor del sensor
        delay ( 1000 ); //espera 1 seg          
        lcd.clear ( ); //borra pantalla    
      }else{
          if(25.00 < Soilmoisture && Soilmoisture <=50.00 ){ //cuando esta en 50% se pondra estos valore en la LCD
            lcd.backlight();   //Enciende la Luz del Fondo del LCD   
            lcd.setCursor ( 0, 0 ); //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.
            lcd.write ( 2 );//Enciende la Luz del Fondo del LCD
            lcd.setCursor ( 2, 0 ); //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.    
            lcd.print ( "La humedad del" );//manda el mensaje a mostar
            lcd.setCursor ( 0, 1 ); //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.    
            lcd.print ( "suelo es de:" );//manda el mensaje a mostar
            lcd.setCursor ( 12, 1 ); //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.   
            lcd.print ( Soilmoisture );  //manda a mostrar el valor del sensor
            delay ( 1000 );   //espera 1 seg           
            lcd.clear ( );  //borra pantalla       
          }else{
              if(50.00 < Soilmoisture && Soilmoisture <=75.00 ){//cuando esta en 75% se pondra estos valore en la LCD
                lcd.backlight(); //Enciende la Luz del Fondo del LCD     
                lcd.setCursor ( 0, 0 );//establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD. 
                lcd.write ( 3 );//Enciende la Luz del Fondo del LCD
                lcd.setCursor ( 2, 0 ); //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.    
                lcd.print ( "La humedad del" );//manda el mensaje a mostar
                lcd.setCursor ( 0, 1 );  //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.   
                lcd.print ( "suelo es de:" );//manda el mensaje a mostar
                lcd.setCursor ( 12, 1 );  //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.  
                lcd.print ( Soilmoisture );  //manda a mostrar el valor del sensor
                delay ( 1000 );   //espera 1 seg           
                lcd.clear ( ); //borra pantalla          
              }else{
                  if(75.00 < Soilmoisture && Soilmoisture <=100.00){//cuando esta en 100% se pondra estos valore en la LCD
                    lcd.backlight();//Enciende la Luz del Fondo del LCD        
                    lcd.setCursor ( 0, 0 );//establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD. 
                    lcd.write ( 4 );//Enciende la Luz del Fondo del LCD
                    lcd.setCursor ( 2, 0 ); //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.    
                    lcd.print ( "La humedad del" );//manda el mensaje a mostar
                    lcd.setCursor ( 0, 1 ); //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.  
                    lcd.print ( "suelo es de:" );//manda el mensaje a mostar
                    lcd.setCursor ( 12, 1 ); //establece la ubicación en la que se mostrará el texto escrito para la pantalla LCD.   
                    lcd.print ( Soilmoisture ); //manda a mostrar el valor del sensor 
                    delay ( 1000 );  //espera 1 seg            
                    lcd.clear ( ); //borra pantalla          
                  }
              }
          }
      }
  }
  
  

}
