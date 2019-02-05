#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

int valor=-1;
Semaphore ingr_disp[3] = {0,0,0};
Semaphore mostr_vacio=0;
Semaphore provee=1;

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

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero()
{

while(true){

sem_wait(mostr_vacio);

cout<<"Ingrediente:  "<<valor<<endl;

sem_signal(ingr_disp[valor]);
}

}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
void  funcion_hebra_proveedor(){
	while(true){
		sem_wait(provee);
		cout<<"Provee:"<<endl;
		valor=aleatorio<0,2>();
		sem_signal(mostr_vacio);
}
	}

// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
	ingr_disp[num_fumador].sem_wait();
   	cout<<"Retira el ingr: "<<num_fumador<<endl;
	sem_signal(provee);
   	fumar(num_fumador);

   }
}

//----------------------------------------------------------------------

int main()
{

thread hebra_proveedor(funcion_hebra_proveedor),
	hebra_estanquero(funcion_hebra_estanquero),
       hebra_fumador0(funcion_hebra_fumador,0),
       hebra_fumador1(funcion_hebra_fumador,1),
       hebra_fumador2(funcion_hebra_fumador,2);

  // esperar a que terminen todas las hebras
hebra_estanquero.join();
hebra_fumador0.join();
hebra_fumador1.join();
hebra_fumador2.join();
}
