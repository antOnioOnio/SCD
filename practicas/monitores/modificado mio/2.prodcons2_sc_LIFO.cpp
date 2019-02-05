// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// archivo: prodcons_2_sc_LIFO.cpp
// Ejemplo de un monitor en C++11 con semántica SC, para el problema
// del productor/consumidor, con multiples consumidores y producotores
// 
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
   num_items  = 40 ,     // número de items a producir/consumir
   num_hebras_consumidoras = 10,
   num_hebras_productoras = 10,
   items_productor = num_items/num_hebras_productoras,
   items_consumidor = num_items/num_hebras_consumidoras;

   int array_compartido[num_hebras_productoras]={0};
   
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
// FUNCIONES PRODUCTORAS Y CONSUMIDORAS
//----------------------------------------------------------------------

int producir_dato(int hebra)
{
   // Ganamos exclusion mutua, pues no queremos alterar datos del array compartido
   
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   // inicializamos las posiciones de nuestro vector 
   if (array_compartido[hebra]==0){
      array_compartido[hebra]=hebra*items_productor;
   }
      
      int contador = array_compartido[hebra];
      mtx.lock();
      cout << "Hebra "<<hebra << " produce: " << contador << endl << flush ;
      array_compartido[hebra]++;
      mtx.unlock();
      
      cont_prod[contador] ++ ;
   return contador;
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
// clase para monitor buffer, version LIFO, semántica SC, mult. produc y consu.

class ProdCons1SC
{
 private:
 static const int              //  constantes:
   num_celdas_total = 10;      //  núm. de entradas del buffer
 int                           //  variables permanentes
   buffer[num_celdas_total],   //  buffer de tamaño fijo, con los datos
   primera_libre;
   
 mutex
   cerrojo_monitor ;           //  cerrojo del monitor
 condition_variable            //  colas condicion:
   ocupadas,                   //  cola donde espera el consumidor (posiscion_escribir != posicion_leer)
   libres ;                    //  cola donde espera el productor  (posicion_escribir+1 != posicion_leer)

 public:                       //  constructor y métodos públicos
   ProdCons1SC(  ) ;           //  constructor
   int  leer();                //  extraer un valor (sentencia L) (consumidor)
   void escribir( int valor ); //  insertar un valor (sentencia E) (productor)
} ;
// -----------------------------------------------------------------------------

ProdCons1SC::ProdCons1SC(  )
{

   primera_libre = 0;
   
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato

int ProdCons1SC::leer(  )
{
     // ganar la exclusión mutua del monitor con una guarda
   unique_lock<mutex> guarda( cerrojo_monitor );

   
   // esperar bloqueado hasta que haya escritos
   // tambien nos sirve para que no se lea una posicion antes de ser escrita
   while( primera_libre== 0){
     ocupadas.wait(guarda);
   }

   // hacer la operación de lectura
   assert(0< primera_libre);
   primera_libre--;
   const int valor = buffer[primera_libre] ;
 
   // señalar al productor que puede escribir sobre la siguiente celda
   libres.notify_one();

   // devolver valor
   return valor ;
}
// -----------------------------------------------------------------------------

void ProdCons1SC::escribir( int valor )
{
   // ganar la exclusión mutua del monitor con una guarda
   unique_lock<mutex> guarda( cerrojo_monitor );

   // Como no podemos hacer suposiciones de velocidad, tenemos que asegurarnos de que la posicion
   // de escritura no coja nunca a la posicion de lectura en el caso de que esta la doble.
   
   while (primera_libre == num_celdas_total){
      libres.wait(guarda);
   }
   // operacion de inserccion
   assert( primera_libre < num_celdas_total);
      buffer[primera_libre] = valor ;
   // Aumentamos y nos aseguramos de no salirnos del vector
   primera_libre++;

   // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
   ocupadas.notify_one();
}
// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora( ProdCons1SC * monitor, int num_hebra )
{  
         for( unsigned i = 0 ; i < items_productor ; i++ )
         {
   
         int valor = producir_dato(num_hebra) ;

         monitor->escribir( valor );
         }
}
// -----------------------------------------------------------------------------

void funcion_hebra_consumidora( ProdCons1SC * monitor, int num_hebra )
{
   for( unsigned i = 0 ; i < items_consumidor ; i++ )
   {
      int valor = monitor->leer();
      consumir_dato( valor ) ;
   }
}
// -----------------------------------------------------------------------------

int main()
{
   cout << "-------------------------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (Mult. prod/cons, Monitor SC, buffer LIFO). " << endl
        << "-------------------------------------------------------------------------------" << endl
        << flush ;


   
   ProdCons1SC monitor ;
   thread hebras_con[num_hebras_consumidoras],
          hebras_pro[num_hebras_productoras];

   for ( int i = 0 ; i < num_hebras_productoras ;i++){
      hebras_pro[i]= thread(funcion_hebra_productora, &monitor, i);
   }
   for ( int i = 0 ; i< num_hebras_consumidoras ; i++){
      hebras_con[i] = thread(funcion_hebra_consumidora, &monitor, i);
   }

   for ( int i = 0 ; i <num_hebras_productoras; i ++){
      hebras_pro[i].join();
   }
   for ( int i = 0 ; i<num_hebras_consumidoras; i++){
      hebras_con[i].join();
   }

   // comprobar que cada item se ha producido y consumido exactamente una vez
   test_contadores() ;
}
