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
   chrono::milliseconds duracion_fumar( aleatorio<1000,2000>() );

   // informa de que comienza a fumar

    cout << "Fumador: " << num_fumador << " empieza a fumar " << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador: " << num_fumador << " termina de fumar, comienza espera de ingrediente." << endl;
    this_thread::sleep_for( duracion_fumar );

}


//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero
Semaphore provee = 1;
Semaphore estanquero = 0;
Semaphore mostrar_vacio = 1;
Semaphore ingr_disp[2] = {0,0};

int vector_buffer[10];

void funcion_hebra_proveedor()
{
  while(true)
  {
    sem_wait(provee);
    int num = -1;
    for ( int i = 0; i<10; i++)
    {
      num=aleatorio<0,1>() ;
      vector_buffer[i]=num;
    }
    cout << "\t SU PROVEEDOR GUAPO HA GENERAO EL PAQUETON \n\n" ;
    sem_signal(estanquero);

  }

}
void funcion_hebra_estanquero()
{
  chrono::milliseconds duerme( aleatorio<100,2000>() );
  while(true)
  {
    sem_wait(estanquero);
    cout << "\t   SU ESTANQUERO RESHULON HA RICIBIO EL PAQUETON \n\n" ;

    int ingre = -1;
    for ( int i = 0; i<10; i++)
    {
        sem_wait(mostrar_vacio);
        ingre = vector_buffer[i];
        cout<< "Estanquero ha ponio\t"<< ingre << "\n";
        this_thread::sleep_for(duerme);

        sem_signal(ingr_disp[ingre]);
    }

    sem_signal(provee);
  }
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
  chrono::milliseconds duerme( aleatorio<100,2000>() );
   while( true )
   {
     sem_wait(ingr_disp[num_fumador]);

     cout<<"Cliente: "<< num_fumador << " ha retirado el ingrediente   "<< num_fumador << endl;
     this_thread::sleep_for(duerme);

     sem_signal ( mostrar_vacio);
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
            hebra_fumador_1 ( funcion_hebra_fumador, 1 );

           hebra_proveedor.join();
           hebra_estanquero.join() ;
           hebra_fumador_0.join() ;
           hebra_fumador_1.join();


 return 0;
}
