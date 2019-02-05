// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// archivo: barbero_durmiente.cpp
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// del barbero durmiente
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
// Función que simula la acción de esperar fuera de la barberia

void esperaFueraBarberia( int num_cliente )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_espera( aleatorio<500,2000>() );

   // informa de que comienza a esperar

    cout << "Cliente : " << num_cliente
          << " empieza a esperar fuera (" << duracion_espera.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_espera' milisegundos
   this_thread::sleep_for( duracion_espera );

   // informa de que ha terminado de esperar

    cout << "Cliente: " << num_cliente << " termina de esperar." << endl;

}

//-------------------------------------------------------------------------
// Función que simula la acción de esperar mientras un cliente es pelado

void cortarPelocliente()
{

   // calcular milisegundos aleatorios de duración de la acción de cortar pelo
   chrono::milliseconds duracion_cortar( aleatorio<500,2000>() );

   // informa de que comienza a cortarse el pelo

    cout << "Cliente empieza a ser pelado (" << duracion_cortar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_cortar' milisegundos
   this_thread::sleep_for( duracion_cortar );

   // informa de que ha terminado de ser pelado

    cout << "Cliente termina de ser pelado." << endl;

}

// *****************************************************************************
// clase para monitor Barberia
// 

class Barberia: public HoareMonitor
{
private:
	//const static int clientes = 3;			// constante clientes
	CondVar estadoBarbero,				// Colas usadas
			estadoSala,
			sillaOcupada;
						
public:
	Barberia();
	void cortarPelo(int i);
	void siguienteCliente();
	void finCliente();		

};

// -----------------------------------------------------------------------------
// CONSTRUCTOR DE LA CLASE		

Barberia::Barberia()
{
	estadoBarbero = newCondVar();	
	estadoSala = newCondVar();
	sillaOcupada = newCondVar();
}
// -----------------------------------------------------------------------------
//función para cortar pelo

void Barberia::cortarPelo(int i )
{
	cout << " Cliente " << i << " ha entrado en la barberia" <<endl;
	// Si la cola del barbero esta vacia significa que el barbero esta dormido, lo despertamos
	if ( !estadoBarbero.empty()){					
		cout << i << "despierta al barbero "<<endl;
		estadoBarbero.signal(); 	// señal hacia la cola del barbero
	}	
	// si el barbero no esta dormido pero la silla esta ocupada, esperamos en la sala
	else if ( !sillaOcupada.empty()){
		cout << "Silla ocupada " << i << " espera en la sala" <<endl;
		estadoSala.wait();			// señal hacia la cola de la sala
	}

	cout << i << " comienza a pelarse " <<endl;
	sillaOcupada.wait();			// ocupamos la silla, señal a esta

}
// -----------------------------------------------------------------------------
//función para pasar a siguiente cliente

void Barberia::siguienteCliente()
{
	if ( estadoSala.empty()){					// si la sala esta vacia el barbero duerme
		cout << " No hay clientes, a descansar "<< endl;
		estadoBarbero.wait();					// señal de espera
	}
	// una vez que salimos de la espera avisamos a la cola de la sala de espera
	cout << " SIGUIENTE !!!"<< endl;	
	estadoSala.signal();						// señal a la cola de la sala				

}

// -----------------------------------------------------------------------------
//función para pasar a finalizar cliente

void Barberia::finCliente()
{
	sillaOcupada.signal();	// indicamos que la silla ya esta disponible	
}
// -----------------------------------------------------------------------------
//función hebra barbero

void funcion_hebra_barbero( MRef<Barberia> monitor)
{
	while(true)
	{
		monitor->siguienteCliente();
		cortarPelocliente();
		monitor->finCliente();
	}
}
// -----------------------------------------------------------------------------
//función hebras clientes

void funcion_hebra_cliente( MRef<Barberia> monitor,int num_cliente)
{
	while(true)
	{
		monitor->cortarPelo(num_cliente);
		esperaFueraBarberia(num_cliente);
	}
}



// -----------------------------------------------------------------------------
// FUNCION MAIN
int main()
{
	cout << "-------------------------------------------------------------------------------" << endl
        << "				Problema del barbero durmiente" << endl
        << "-------------------------------------------------------------------------------" << endl
        << flush ;


	MRef<Barberia> monitor = Create<Barberia>();
	constexpr int clientes = 3;
	thread hebras_clientes[clientes],
	 	   hebra_barbero;

	for ( int i = 0 ; i < clientes ; i++){
		hebras_clientes[i]= thread(funcion_hebra_cliente,monitor,i);
	}	

		hebra_barbero = thread(funcion_hebra_barbero,monitor ) ;

	for ( int i = 0 ; i < clientes ; i++){
		hebras_clientes[i].join();
	}	

	hebra_barbero.join();




}















