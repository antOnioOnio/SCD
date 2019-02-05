// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// archivo: prodcons_1_sc_FIFO.cpp
// Ejemplo de un monitor en C++11 con semántica SC, para el problema
// del productor/consumidor, con un único productor y un único consumidor.
// Opcion FIFO 
//
// Historial:
// Creado en Octubre de 2017
// Antonio Garcia Castillo
// -----------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>

using namespace std ;

constexpr int
   num_items  = 40 ;     // número de items a producir/consumir

mutex
   mtx ;                 // mutex de escritura en pantalla
unsigned
   cont_prod[num_items], // contadores de verificación: producidos
   cont_cons[num_items]; // contadores de verificación: consumidos

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
   mtx.lock();
   cout << "producido: " << contador << endl << flush ;
   mtx.unlock();
   cont_prod[contador] ++ ;
   return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   if ( num_items <= dato )
   {
      cout << " dato === " << dato << ", num_items == " << num_items << endl ;
      assert( dato < num_items );
   }
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "                  consumido: " << dato << endl ;
   mtx.unlock();
}
//----------------------------------------------------------------------

void ini_contadores()
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  cont_prod[i] = 0 ;
      cont_cons[i] = 0 ;
   }
}

//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << flush ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

// *****************************************************************************
// clase para monitor buffer, version FIFO, semántica SC, un prod. y un cons.

class ProdCons1SC
{
 private:
 static const int              //  constantes:
   num_celdas_total = 10;      //  núm. de entradas del buffer
 int                           //  variables permanentes
   buffer[num_celdas_total],   //  buffer de tamaño fijo, con los datos
   posicion_leer ,             //  indice de celda de la próxima inserción
   posicion_escribir,
   usados;
 mutex
   cerrojo_monitor ;           //  cerrojo del monitor
 condition_variable            //  colas condicion:
   ocupadas,                   //  cola donde espera el consumidor (usados == 0)
   libres ;                    //  cola donde espera el productor  (usados == num_celdas_total)

 public:                       //  constructor y métodos públicos
   ProdCons1SC(  ) ;           //  constructor
   int  leer();                //  extraer un valor (sentencia L) (consumidor)
   void escribir( int valor ); //  insertar un valor (sentencia E) (productor)
} ;
// -----------------------------------------------------------------------------

ProdCons1SC::ProdCons1SC(  )
{
   posicion_leer = 0 ;
   posicion_escribir = 0;
   usados = 0;
   
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato

int ProdCons1SC::leer(  )
{
   unique_lock<mutex> guarda( cerrojo_monitor );   // ganar la e.m del monitor con una guarda

   if ( usados == 0){                              // esperar bloqueado hasta que haya escritos
      ocupadas.wait(guarda);
   }

   const int valor = buffer[posicion_leer] ;       // hacer la operación de lectura

   posicion_leer++ ;                               // aumentamos posicion de lectura
   
   posicion_leer = posicion_leer%num_celdas_total; // Nos aseguramos de que no se salga de nuestro buffer

   usados--;                                       // Disminuimos el valor usados

   
   libres.notify_one();                            // señalar al productor que puede escribir 
                                                   // sobre la siguiente celda

   return valor ;                                  // devolver valor
}
// -----------------------------------------------------------------------------

void ProdCons1SC::escribir( int valor )
{
   unique_lock<mutex> guarda( cerrojo_monitor );            // ganar la e.m del monitor con una guarda

   if (usados == num_celdas_total){                         // Esperamos bloqueados si usados alcanza 
         libres.wait(guarda)                                // el limite del buffer;
   }
  
   buffer[posicion_escribir] = valor ;                      // operacion de escritura
   
   posicion_escribir++;                                     // Aumentamos y aseguramos de estar en los limites
   posicion_escribir = posicion_escribir%num_celdas_total;
   usados++;                                                // Aumentamos el valor usados

   
   ocupadas.notify_one();    // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
}


// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora( ProdCons1SC * monitor )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int valor = producir_dato() ;
      monitor->escribir( valor );
   }
}
// -----------------------------------------------------------------------------

void funcion_hebra_consumidora( ProdCons1SC * monitor )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int valor = monitor->leer();
      consumir_dato( valor ) ;
   }
}
// -----------------------------------------------------------------------------

int main()
{
   cout << "-------------------------------------------------------------------------------" << endl
        << "Prooblema de los productores-consumidores (1 prod/cons, Monitor SC, buffer FIFO). " << endl
        << "-------------------------------------------------------------------------------" << endl
        << flush ;

   ProdCons1SC monitor ;

   thread hebra_productora ( funcion_hebra_productora, &monitor ),
          hebra_consumidora( funcion_hebra_consumidora, &monitor );

   hebra_productora.join() ;
   hebra_consumidora.join() ;

   // comprobar que cada item se ha producido y consumido exactamente una vez
   test_contadores() ;
}
