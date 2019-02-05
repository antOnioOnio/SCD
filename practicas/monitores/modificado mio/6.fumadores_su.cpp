// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// archivo: fumadores_su.cpp
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// del estanquero/fumadores.
// 
//
// Historial:
// Creado en Noviembre de 2017
// -----------------------------------------------------------------------------



#include <iostream>
#include <iomanip>
#include <random>
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM ;

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
   chrono::milliseconds duracion_fumar( aleatorio<20,2000>() );

   // informa de que comienza a fumar

    cout << "Fumador : " << num_fumador
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador: " << num_fumador << " termina de fumar, comienza espera de ingrediente." << endl;

}
//-------------------------------------------------------------------------
// Funcion que produce un ingrediente aleatorio entre el cero y el dos

int producirIngrediente(){
	chrono::milliseconds duracion_poner( aleatorio<100,2000>() );
	int i = aleatorio<0,2>();
	this_thread::sleep_for( duracion_poner );

	return i ;
}

// *****************************************************************************
// clase para monitor EstancoSU

class EstancoSU: public HoareMonitor
{
private:
	const static int fumadores = 3;			// constante fumadores
	CondVar fumador[fumadores],				// Colas usadas
			//estanquero,
			mostrador;
	int 	ingrediente;					
public:
	EstancoSU();
	void obtenerIngrediente(int i);
	void ponerIngrediente(int i);
	void esperarRecogidaIngrediente();		

};

// -----------------------------------------------------------------------------
// CONSTRUCTOR DE LA CLASE		

EstancoSU::EstancoSU()
{
	for (int i =0; i<fumadores; i++){
	 fumador[i] = newCondVar();

	}
	 //estanquero = newCondVar();
	 mostrador = newCondVar();
	 //estanquero = newCondVar();
	 ingrediente = -1; 
}

// -----------------------------------------------------------------------------
//función para poner el ingrediente sobre el mostrador

void EstancoSU::ponerIngrediente(int ing)
{
  ingrediente = ing;					// Actualizamos nuestro ingrediente, variable privada de la clase
  cout <<" Estanguero pone "<<ing<<endl;// Imprimimos por pantalla
  fumador[ing].signal();				// Avisamos al fumador que esta en espera
		
}
// -----------------------------------------------------------------------------
//función para obtener el ingrediente del mostrador

void EstancoSU::obtenerIngrediente(int ing)
{
	if(ingrediente!=ing){
		fumador[ing].wait();						// el fumador queda bloqueado mientras el ingrediente
		                    						// no sea el suyo
	}
	cout<<" fumador "<<ing<<" recoge "<<ing<<endl;	
	ingrediente = -1;								// Actualizamos	
	mostrador.signal();								// Hacemos la señal correspondiente al estanquero	

}
// -----------------------------------------------------------------------------
//función para bloquear al estanquero hasta que sea retirado el ingrediente

void EstancoSU::esperarRecogidaIngrediente()
{
	if( ingrediente!=-1){		
		mostrador.wait();		// Esperamos la señal proveniente del fumador al recoger
		                 		// el ingrediente
	}	
}

// -----------------------------------------------------------------------------

void funcion_hebra_fumador( MRef<EstancoSU> monitor,int num_fumador)
{
	while(true)
	{
		monitor->obtenerIngrediente(num_fumador);
		fumar(num_fumador);
	}
}

// -----------------------------------------------------------------------------

void funcion_hebra_estanquero(MRef<EstancoSU> monitor)
{
	while(true)
	{
	int ing = producirIngrediente();		
	monitor->ponerIngrediente(ing);
	monitor->esperarRecogidaIngrediente();
	}
}
// -----------------------------------------------------------------------------
// FUNCION MAIN
int main()
{
	cout << "-------------------------------------------------------------------------------" << endl
        << "				Problema del estanquero y los fumadores" << endl
        << "-------------------------------------------------------------------------------" << endl
        << flush ;


	MRef<EstancoSU> monitor = Create<EstancoSU>();
	constexpr int fumadores = 3;
	thread hebras_fumadoras[fumadores],
	 	   hebra_estanquero;

	for ( int i = 0 ; i < fumadores ; i++){
		hebras_fumadoras[i]= thread(funcion_hebra_fumador,monitor,i);
	}	

		hebra_estanquero = thread(funcion_hebra_estanquero,monitor ) ;

	for ( int i = 0 ; i < fumadores ; i++){
		hebras_fumadoras[i].join();
	}	

	hebra_estanquero.join();




}










































