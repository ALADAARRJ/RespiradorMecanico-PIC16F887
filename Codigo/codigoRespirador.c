///////////////////////////////////////////////////////////////////////////
////           UNIVERSIDAD NACIONAL AUTONOMA DE MEXICO                 ////
////            FACULTAD DE ESTUDIOS SUPERIORES ARAGON                 ////
////                 INGENIERIA EN COMPUTACION                         ////
////                                                                   ////
////             PROYECTO FINAL PARA LAS MATERIAS DE:                  ////
//// "MICROPROCESADORES Y MICROCONTROLADORES" Y "SISTEMAS DE CONTROL"  ////
///                                                                    ////
///                   "RESPIRADOR ARTIFICIAL"                          ////
///                                                                    ////
/// INTEGRANTES:                                                       ////
///               *AMADOR TABOADA DIEGO URIEL                          ////
///               *ARREDONDO JIMENEZ ALAN DANIEL                       ////
///               *RAMIREZ TAPIA LUIS FERNANDO                         ////
///////////////////////////////////////////////////////////////////////////
/*
ENUNCIADO: EL SIGUIENTE PROGRAMA TIENE COMO FUNCION, MANEJAR UN HARDWARE QUE HA
SIDO ESPECIALMENTE DISENNADO PARA FUNCIONAR DE LA MANO CON ESTE CODIGO.

SU PROPOSITO ES AYUDAR AL MEDICO O PARAMEDICO EN SU LABOR DE "RESUCITACION O AUXILIO"
EN EL PROCESO DE RESPIRACION,  AL PACIENTE QUE POR SU CUENTA NO PUEDE REALIZAR ESTE PROCESO.
*/

#include<16f887.h> 
#fuses INTRC_IO,NOWDT,PUT,NOBROWNOUT
#device ADC=8
#use delay(INTERNAL=500000) //Configuracion del reloj.
#include"lcd.h"
#use RS232(baud=9600,xmit=PIN_C6,rcv=PIN_C7,timeout=100)
//Configuracion de puertos
#use fast_io(D)
#use fast_io(E)

int8 var_respiraciones=0;

int16 var_adc=0;


float seg_ondulacion=0;

int16 pos=31;
int16 t_subida=0;
int16 t_bajada_meseta=0;
int16 t_meseta=0;
int16 t_bajada=0;
int16 t_pausa=0;


int1 f_emergencia=0;
int1 f_stop=0;
int1 f_inicio=0;
int1 f_ventilar=0;

int16 variable; 
float ventilas;

char modo = 'A';
char instruccion;
   

void mostrar_variable(int8 var_sep);
void solo_mostrar_variable(void);


#INT_TIMER1
void inttimer1_(){
   solo_mostrar_variable();
   output_toggle(PIN_C3);
   set_timer1(59036);
}

#INT_RB
void IntPortB4_7(){
   if(!input(PIN_B4) && input(PIN_B5) && input(PIN_B7)){
      f_inicio=1;
   }
   else if(!input(PIN_B5) && input(PIN_B4) && input(PIN_B7)){
      reset_cpu();
   }
   else if(!input(PIN_B7) && input(PIN_B5) && input(PIN_B4)){
      if(f_emergencia==0){
         setup_ccp2(CCP_PWM);
         int16 pwm_sonido=315;
         set_pwm2_duty(pwm_sonido);
         f_emergencia=1;
      }
      else{
         f_emergencia=0;
         setup_ccp1(CCP_OFF);
         int16 pwm_sonido=0;
         set_pwm2_duty(pwm_sonido);
      }
   }
   delay_ms(200);
}

void main(){
   set_tris_D(0x00);
   set_tris_E(0x00);
   
   output_D(0);
   
   port_b_pullups(255);
   
   setup_adc(ADC_CLOCK_INTERNAL);
   setup_adc_ports(sAN0);
   set_adc_channel(0);
   
   setup_timer_2(T2_DIV_BY_16,155,1);
   setup_ccp1(CCP_OFF);
   setup_ccp2(CCP_OFF);
   setup_timer_1(T1_DISABLED);
   f_stop=1;
   
   disable_interrupts(INT_TIMER1);
   enable_interrupts(INT_RB);
   enable_interrupts(GLOBAL);
   
   while(true){
      switch(modo){
         case 'A': //Automatico
            instruccion = getch();
            ///////////////////CODIGO LCD
            lcd_init();  //Inicializamos el LCD
            setup_adc_ports(sAN0); 
            setup_adc(ADC_CLOCK_INTERNAL); //Utilizamos el reloj interno para generar una frecuencia de muestreo
            variable=read_adc(); 
            ventilas = 12+variable/6.7105; //Regla de tres, con una constante para obtener el numero de ventilaciones por minuto 
            printf(lcd_putc, "VentiladorFisico");  
            printf(lcd_putc, "\nVentilas/m: %1.0f",ventilas);
            delay_ms(100); 
            printf("%Lu,%1.0f\n\r", variable, ventilas);
            //////////////////FIN CODIGO LCD
            
            if(f_ventilar==1){
               pos=30;
               while(pos<=47){
                  set_pwm1_duty(pos);
                  delay_ms(t_subida);
                  pos=pos+1;
               }
               pos=47;
               while(pos<=42){
                  set_pwm1_duty(pos);
                  delay_ms(t_bajada_meseta);
                  pos=pos+1;
               }
               delay_ms(t_meseta);
               pos=37;
               set_pwm1_duty(pos);
               
               while(pos>=30){
                  set_pwm1_duty(pos);
                  delay_ms(t_bajada);
                  pos=pos-1;
               }
               delay_ms(t_pausa);
            }
            else if(f_inicio==1){
               seg_ondulacion=60000.0/var_respiraciones;
               t_subida=seg_ondulacion*0.0123529;
               t_bajada_meseta=seg_ondulacion*0.016;
               t_meseta=seg_ondulacion*0.14;
               t_bajada=seg_ondulacion*0.0442857;
               t_pausa=seg_ondulacion*0.26;
               
               f_ventilar=1;
               f_inicio=0;
               f_stop=0;
               setup_ccp1(CCP_PWM);
               set_pwm1_duty(pos);
               
               enable_interrupts(INT_TIMER1);
               setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
               set_timer1(53036);
            }
            else if(f_stop==1){
               var_adc=read_adc();
               var_respiraciones=12+var_adc/6.7105;
               mostrar_variable(var_respiraciones);
            }
            if (instruccion == 'M'){
               modo = 'M';
            }
            break;
         case 'M': //Manual
            instruccion = getch();
            lcd_init();  //Inicializamos el LCD
            setup_adc_ports(sAN0); 
            setup_adc(ADC_CLOCK_INTERNAL); //Utilizamos el reloj interno para generar una frecuencia de muestreo
            variable=read_adc(); 
            ventilas = 12+variable/6.7105; //Regla de tres, con una constante para obtener el numero de ventilaciones por minuto 
            printf(lcd_putc, "VentiladorManual");  
            printf(lcd_putc, "\nVentilas/m: %1.0f",ventilas);
            delay_ms(100); 
            printf("%Lu,%1.0f\n\r", variable, ventilas);
            /////////////////FIN CODIGO LCD
            if(instruccion == 'I'){
               f_inicio=1;
            }
            else if(instruccion =='P'){
               reset_cpu();
            }
            else if(instruccion =='E'){
               if(f_emergencia==0){
                  setup_ccp2(CCP_PWM);
                  int16 pwm_sonido=315;
                  set_pwm2_duty(pwm_sonido);
                  f_emergencia=1;
               }
               else{
                  f_emergencia=0;
                  setup_ccp1(CCP_OFF);
                  int16 pwm_sonido=0;
                  set_pwm2_duty(pwm_sonido);
               }
            }
            while(true){
               if(f_ventilar==1){
                  pos=30;
                  while(pos<=47){
                     set_pwm1_duty(pos);
                     delay_ms(t_subida);
                     pos=pos+1;
                  }
                  pos=47;
                  while(pos<=42){
                     set_pwm1_duty(pos);
                     delay_ms(t_bajada_meseta);
                     pos=pos+1;
                  }
                  delay_ms(t_meseta);
                  pos=37;
                  set_pwm1_duty(pos);
                     
                  while(pos>=30){
                     set_pwm1_duty(pos);
                     delay_ms(t_bajada);
                     pos=pos-1;
                  }
                  delay_ms(t_pausa);
               }
               else if(f_inicio==1){
                  seg_ondulacion=60000.0/var_respiraciones;
                  t_subida=seg_ondulacion*0.0123529;
                  t_bajada_meseta=seg_ondulacion*0.016;
                  t_meseta=seg_ondulacion*0.14;
                  t_bajada=seg_ondulacion*0.0442857;
                  t_pausa=seg_ondulacion*0.26;
                     
                  f_ventilar=1;
                  f_inicio=0;
                  f_stop=0;
                  setup_ccp1(CCP_PWM);
                  set_pwm1_duty(pos);
                   
                  enable_interrupts(INT_TIMER1);
                  setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
                  set_timer1(53036);
               }
               else if(f_stop==1){
                  var_adc=read_adc();
                  var_respiraciones=12+var_adc/6.7105;
                  mostrar_variable(var_respiraciones);
               }               
            break;
            }
      }

   }
}


void mostrar_variable(int8 var_sep){

}

void solo_mostrar_variable(void){
}
