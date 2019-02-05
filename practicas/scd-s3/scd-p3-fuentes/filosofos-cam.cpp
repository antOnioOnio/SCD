// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos (sin camarero).
// Plantilla para completar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>


using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,
   num_procesos  = 2*num_filosofos + 1 ,
   etiq_com = 11,
   etiq_levantarse = 13,
   etiq_sentarse = 14,
   id_camarero = 10;

 int contador_filosofos = 0;

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

// ---------------------------------------------------------------------

void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)              % (num_procesos-1), //id. tenedor izq.
      id_ten_der = (id+num_procesos-2) % (num_procesos-1); //id. tenedor der.
  int valor=1;
  int sent = 1;
  int levant = 1;
  while ( true )
  {

    cout <<"Filosofo " << id << " solicita sentarse" <<endl;
    MPI_Ssend( &sent,  1, MPI_INT, id_camarero, etiq_sentarse, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
    MPI_Ssend( &valor,  1, MPI_INT, id_ten_izq, etiq_com, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
    MPI_Ssend( &valor,  1, MPI_INT, id_ten_der, etiq_com, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
    MPI_Ssend( &valor,  1, MPI_INT, id_ten_izq, etiq_com, MPI_COMM_WORLD);

    cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
    MPI_Ssend( &valor,  1, MPI_INT, id_ten_der, etiq_com, MPI_COMM_WORLD);

    cout << " Filosofo "<< id << " se levanta "<< endl;

    MPI_Ssend( &levant,  1, MPI_INT, id_camarero, etiq_levantarse, MPI_COMM_WORLD);

    cout << "Filosofo " << id << " comienza a pensar" << endl;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}
// ---------------------------------------------------------------------
void funcion_camarero ()
{
 
  int valor, tag_emisor_aceptable;
  MPI_Status estado;

  while (true)
  {
      if ( contador_filosofos == 4){
        tag_emisor_aceptable = etiq_levantarse;
      }
      else if (contador_filosofos == 0){
        tag_emisor_aceptable = etiq_sentarse;
      }
      else {
        tag_emisor_aceptable = MPI_ANY_TAG;
      }

    MPI_Recv (&valor, 1, MPI_INT, MPI_ANY_SOURCE, tag_emisor_aceptable ,MPI_COMM_WORLD, &estado);

    switch (estado.MPI_TAG)
    {
      case etiq_sentarse:
        if ( contador_filosofos <4)
        {
          contador_filosofos++;
          cout << " Camarero ha sentado a filosofo " << estado.MPI_SOURCE << endl;

        }
        break;
      case etiq_levantarse:
        cout << " Camarero ha levantado a filosofo "<< estado.MPI_SOURCE << endl;
        contador_filosofos--;
        break;
    }
  }

}
void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
     // ...... recibir petición de cualquier filósofo (completar)
    
     MPI_Recv (&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_com ,MPI_COMM_WORLD, &estado);
     // ...... guardar en 'id_filosofo' el id. del emisor (completar)
     id_filosofo = estado.MPI_SOURCE;

     cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

     // ...... recibir liberación de filósofo 'id_filosofo' (completar)
     MPI_Recv (&valor, 1, MPI_INT, id_filosofo, etiq_com ,MPI_COMM_WORLD, &estado);
     cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
  }
}
// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if ( id_propio % 2 == 0 && id_propio != 10 ) 
      {         // si es par y diferente al del camarero
         funcion_filosofos( id_propio ); //   es un filósofo
      }
      else if ( id_propio == 10)
      {
        funcion_camarero();
      } 
      else {                      // si es impar
         funcion_tenedores( id_propio ); //   es un tenedor
       }

   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
