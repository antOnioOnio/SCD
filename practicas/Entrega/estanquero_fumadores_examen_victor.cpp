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


//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero
Semaphore provee = 1;
Semaphore estanquero = 0;
Semaphore mostrar_vacio = 1;
Semaphore ingr_disp[3] = {0,0,0};

int ingrediente_producido = -1;

void funcion_hebra_proveedor()
{
  while(true)
  {
    sem_wait(provee);
      ingrediente_producido=aleatorio<0,2>() ;


    cout << "\t SU PROVEEDOR GUAPO HA GENERAO UN INGREDIENTE \n\n" ;
    sem_signal(estanquero);
  }
}
void funcion_hebra_estanquero()
{

  while(true)
  {
    sem_wait(estanquero);
    cout << "\t   SU ESTANQUERO RESHULON HA RICIBIO EL INGREDIENTE \n\n" ;

        sem_wait(mostrar_vacio);
        cout<< "Estanquero ha ponio\t"<< ingrediente_producido << "\n";

        sem_signal(ingr_disp[ingrediente_producido]);

  }
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
     sem_wait(ingr_disp[num_fumador]);

     cout<<"Cliente: "<< num_fumador << " ha retirado el ingrediente   "<< num_fumador << endl;

     sem_signal ( mostrar_vacio);
     sem_signal ( provee);
     fumar(num_fumador);

   }
}

//----------------------------------------------------------------------

int main()
{
  cout << "--------------------------------------------------------" << endl
       << "Problema de los fumadores (solución LIFO)." << endl
       << "--------------------------------------------------------" << endl
       << flush ;

    thread  hebra_proveedor (funcion_hebra_proveedor),
            hebra_estanquero ( funcion_hebra_estanquero ),
            hebra_fumador_0 ( funcion_hebra_fumador, 0 ),
            hebra_fumador_2 ( funcion_hebra_fumador, 2 ),
            hebra_fumador_1 ( funcion_hebra_fumador, 1 );

           hebra_proveedor.join();
           hebra_estanquero.join() ;
           hebra_fumador_0.join() ;
           hebra_fumador_1.join();


 return 0;
}
