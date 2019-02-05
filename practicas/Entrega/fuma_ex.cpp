#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;






//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}


//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador: " << num_fumador
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador: " << num_fumador << " termina de fumar, comienza espera de ingrediente." << endl;

}




Semaphore mostrar_vacio = 1;		 // Creamos semaforo que servira para mostrar el mostrador vacio
Semaphore ingr_disp[3] = {0,0,0}; 	 // Creamos tres semaforos, uno para cada ingrediente
Semaphore est[2] ={0,0};		 // Creamos dos semaforos, uno para cada hebra de estanquero

//----------------------------------------------------------------------
// 		función que ejecuta las hebras de los estanqueros
//----------------------------------------------------------------------
void funcion_hebra_estanquero(int num_estanquero)
{
    chrono::milliseconds retardo( aleatorio<20,200>() );

    int anterior = -1;			// Creamos dos variables que nos servira para saber si 
    int actual = -1;			// se ha repetido el elemento dos veces seguidas
    while(true)
    {
      	

      sem_wait(est[num_estanquero]); 	// Esperamos la señal que vendra generada aleatoriamente desde el main
      actual = aleatorio<0,2>();

      sem_wait(mostrar_vacio);

      cout<< "Estanquero "<<num_estanquero <<  " ha puesto "<< actual << endl;
      
      this_thread::sleep_for( retardo );	
      	

      if ( actual == anterior)	 	 // Si el ingrediente actual coincide con el anterior, imprimimos por pantalla el mensaje
      {
        cout <<  "El estanquero "<<num_estanquero << " ha producido dos veces seguidas el mismo ingrediente" ;
	this_thread::sleep_for( retardo );
      }

      sem_signal(ingr_disp[actual]);     // Señal para las hebras fumadoras


      anterior = actual;		 // Actualizamos el valor de anterior a actual
		
      if(num_estanquero == 0)		// Damos la señal a la otra hebra estanquero
	{
     	 sem_signal(est[1]);

	}
      else {
	sem_signal(est[0]);
	}
    }
}


//----------------------------------------------------------------------
// 		función que ejecuta las hebras de los fumadores
//----------------------------------------------------------------------
void  funcion_hebra_fumador( int num_fumador )
{
   chrono::milliseconds retardo( aleatorio<20,200>() );

   int contador_ingr = 0;	// Contador que nos sirve para saber si el consumidor tiene uno o dos ingredientes 
   while( true )
   {
     sem_wait(ingr_disp[num_fumador]);	// esperamos señal 
     cout<<"Cliente: "<< num_fumador << " ha retirado el ingrediente "<< num_fumador << endl;
     this_thread::sleep_for( retardo );
     contador_ingr ++;			// Aumentamos contador ingrediente	
     sem_signal ( mostrar_vacio);	
      if (contador_ingr == 2){		// Si el contador de ingredientes esta a 2 d el consumidor empieza a fumar
    	 fumar(num_fumador);
	 contador_ingr = 0;		// Actualizamos el contador a 0
      }	
   }
}

//----------------------------------------------------------------------

int main()
{
  cout << "--------------------------------------------------------" << endl
       << "Problema de los fumadores ." << endl
       << "--------------------------------------------------------" << endl
       << flush ;

    		// Creamos las hebras correspondientes
    thread hebra_estanquero1( funcion_hebra_estanquero,0 ),
           hebra_estanquero2( funcion_hebra_estanquero,1),
           hebra_fumador_0( funcion_hebra_fumador, 0 ),
           hebra_fumador_1( funcion_hebra_fumador, 1 ),
           hebra_fumador_2( funcion_hebra_fumador, 2 );
		// Generamos la señal aleatoriamente para empezar con el estanquero 1 o estanquero 2
           sem_signal(est[aleatorio<0,1>()]);

           hebra_estanquero1.join() ;
           hebra_estanquero2.join();
           hebra_fumador_0.join() ;
           hebra_fumador_1.join();
           hebra_fumador_2.join();

 return 0;
}
