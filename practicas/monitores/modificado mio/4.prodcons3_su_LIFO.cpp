// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// archivo: prodcons_3_su_LIFO.cpp
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// del productor/consumidor, con un multiples consumidores y productores.
// Opcion LIFO (stack)
//
// Historial:
// Creado en OCTUBRE de 2017
// -----------------------------------------------------------------------------

#include <iostream>
#include <iomanip>
#include <random>
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM ;

  int 
	num_items = 40, // numero de items a producir
	num_hebras_consumidoras = 10,
	num_hebras_productoras = 10,
	items_productor = num_items/num_hebras_productoras,
	items_consumidor = num_items/num_hebras_consumidoras;

int array_compartido[num_hebras_productoras]={0};
mutex 
	mtx ;				//mutex de escritura en pantalla

unsigned
   cont_prod[num_items], // contadores de verificación: producidos
   cont_cons[num_items]; // contadores de verificación: consumidos

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//---------------------------------------------------------------------                       
 
 template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}


//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
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
// clase para monitor buffer, version LIFO, semántica SU, mult. produc y consum


class ProdConsSU: public HoareMonitor
{
private:
	static const int 				      // constantes
		num_celdas_total = 10;		  //num. entradas buffer
	int buffer[num_celdas_total],	
		primera_libre;
	CondVar 
		ocupadas,
		libres;					
public:
	ProdConsSU();					        //Constructor
	int leer();						        //extraer valor
	void escribir(int valor);		  //insertar valor
};
// -----------------------------------------------------------------------------
//				función llamada por el consumidor para extraer un dato

ProdConsSU::ProdConsSU(  )
{

   primera_libre = 0 ;
   ocupadas = newCondVar();   
   libres = newCondVar();
   
}
int ProdConsSU::leer(  )
{
 
   while( primera_libre== 0){
     ocupadas.wait();
   }

   // hacer la operación de lectura
   assert(0< primera_libre);
   primera_libre--;
   const int valor = buffer[primera_libre] ;
 
   // señalar al productor que puede escribir sobre la siguiente celda
   libres.signal();

   // devolver valor
   return valor ;
}
// -----------------------------------------------------------------------------
// 

void ProdConsSU::escribir(int valor)
{
	// Como no podemos hacer suposiciones de velocidad, tenemos que asegurarnos de que la posicion
   // de escritura no coja nunca a la posicion de lectura en el caso de que esta la doble.
   
   while (primera_libre == num_celdas_total){
      libres.wait();
   }
   // operacion de inserccion
   assert( primera_libre < num_celdas_total);
      buffer[primera_libre] = valor ;
   // Aumentamos y nos aseguramos de no salirnos del vector
   primera_libre++;

   // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
   ocupadas.signal();
}

// -----------------------------------------------------------------------------

void funcion_hebra_productora( MRef<ProdConsSU> monitor, int num_hebra )
{  
         for( unsigned i = 0 ; i < items_productor ; i++ )
         {
            int valor = producir_dato(num_hebra) ;
            monitor->escribir( valor );
         }
}

// -----------------------------------------------------------------------------

void funcion_hebra_consumidora( MRef<ProdConsSU>monitor, int num_hebra )
{
   for( unsigned i = 0 ; i < items_consumidor ; i++ )
   {
      int valor = monitor->leer();
      consumir_dato( valor ) ;
   }
}
// -----------------------------------------------------------------------------
// FUNCION MAIN

int main()
{
   cout << "-------------------------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (1 prod/cons, Monitor SC, buffer FIFO). " << endl
        << "-------------------------------------------------------------------------------" << endl
        << flush ;

   MRef<ProdConsSU> monitor = Create<ProdConsSU>();

   
  
   thread hebras_con[num_hebras_consumidoras],
          hebras_pro[num_hebras_productoras];

   for ( int i = 0 ; i < num_hebras_productoras ;i++){
      hebras_pro[i]= thread(funcion_hebra_productora, monitor, i);
   }
   for ( int i = 0 ; i< num_hebras_consumidoras ; i++){
      hebras_con[i] = thread(funcion_hebra_consumidora, monitor, i);
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






























