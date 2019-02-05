#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas

const int num_items = 40 ,   // número de items
	        tam_vec   = 10 ;   // tamaño del buffer
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}; // contadores de verificación: consumidos

Semaphore puede_escribir = tam_vec;
Semaphore puede_leer = 0;
int vector_buffer[tam_vec];


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

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato()
{
   static int contador = 0 ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "producido: " << contador << endl << flush ;

   cont_prod[contador] ++ ;
   return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;

}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------


void  funcion_hebra_productora(  )
{
	int dato;
	int pos_escribir=0;													// Variable local donde almacenamos nuestro dato producido
   for( unsigned i = 0 ; i < num_items ; i++ )
	 {
		 dato = producir_dato();	 						// Producimos dato y almacenamos en dato
		 sem_wait(puede_escribir); 						// Entramos y bajamos en una unidad nuestro semaforo
		 vector_buffer[pos_escribir]=dato;		// Escribimos en el buffer nuestro dato
		 pos_escribir++;											// Aumentamos nuestra posicion de escritura
		 pos_escribir = pos_escribir%tam_vec;	// nos aseguramos de no salirnos del buffer
		 sem_signal(puede_leer);							// Aumentamos nuestro semaforo, señal para poder leer
	 }
 }
//----------------------------------------------------------------------
void funcion_hebra_consumidora(  )
{
	int dato ;
	int pos_leer = 0;													// Variable local donde almacenamos el dato leido
   for( unsigned i = 0 ; i < num_items ; i++ )
  	{
			sem_wait(puede_leer);								// Esperamos a la señal procedente de la hebra productora
			dato=vector_buffer[pos_leer];				// leemos nuestro dato y lo guardamos en dato
			pos_leer++;													// aumentamos nuestra posicion de lectura
			pos_leer = pos_leer%tam_vec;				// nos aseguramos de que no se salga del buffer
			sem_signal(puede_escribir);					// Aumentamos el semaforo puede_escribir en una unidad
			consumir_dato(dato);
		 }
}
//----------------------------------------------------------------------
int main()
{
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;
	 cout << "--------------------------------------------------------" << endl
        << "                    FIN																	" << endl
        << "--------------------------------------------------------" << endl
				<< flush ;


   test_contadores();
	 return 0;
}
