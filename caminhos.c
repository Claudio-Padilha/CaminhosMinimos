#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INFINITO 1500000000									

/*****************         Enum structure with the supported algorithms              ********************/

enum ALGORITMOS {
  INVALIDO=0,
  FLOYD_WARSHAL=1,
  BELLMAND_FORD=2,
  DIJKSTRA=3,
  JOHNSON=4
};


/*****************              Digraph Structures Definitions                       ********************/

typedef int TId;
typedef int TPeso;

struct SAresta {				// edge structure
  TId destino;
  TPeso peso;
};

typedef struct SAresta TAresta;

struct ListaAdj {				// a list of edegs
  struct ListaAdj *prox;
  TAresta aresta;
};

typedef struct ListaAdj TNoLista;

struct SVertice {				// node structure
  TId grauSaida;
  TId grauEntrada;
  TPeso peso;
  
  // Edes coming out from a node.
  TNoLista *direto;
};

typedef struct SVertice TVertice;

struct SDigrafo {				// Digraph structure
  TId n;
  TId m;
  TVertice *vertices;
};

typedef struct SDigrafo TDigrafo;

        
/*******************                     Function Headers                                  *********************/

int inicializa_digrafo(TDigrafo *g, TId n);  	// Initializes a digraph to contain n nodes and m edges.

TDigrafo * cria_digrafo(TId n); 		// Creates and initializes a digraph to contain n nodes and m edges.

const TAresta * conectar_vertices(TDigrafo *g, TId u, TId v, TPeso peso); 	//Connects two nodes witha an edge.

int ** floyd_warshal (TDigrafo *g);   	// Returns a matrix representing the smallest costs between each pair of nodes using Floyd Warshal's algorithm

int * bellmand_ford (TDigrafo *g, int origem, int *has_negative_cycle);	   // Returns a vector representing the smallest costs between origem and each other node using Bellmand Ford's algorithm

int * dijkstra (TDigrafo *g, int origem);		// Returns a matrix representing the smallest costs between each pair of nodes using Dijkstra's algorithm

int ** johnson (TDigrafo *g);			// Returns a matrix representing the smallest costs between each pair of nodes using Johnson's algorithm

int tem_aresta_negativa (TDigrafo *g); 	// Returns TRUE (1) if Digraph has a negative edge

void matriz_para_arquivo(int **custos, int n, FILE *saida);    // Puts the matrix of costs on a file

int swap (int *vetor, int indiceA, int indiceB);		// swap two elements on a vector

int restaura_heap_up (int *heap, int *dist, int indice); 		// Fixes heap going up

int restaura_heap_down (int *heap, int *dist, int indice, int tam); 	// Fixes heap going down

int remove_min_heap (int *heap, int *tam, int *dist); 		// Returns the smallest element on heap

int adiciona_min_heap (int *heap, int *tam, int *dist, int no); 		// Adds a node to heap


/*******************                      Functions for Dijkstra's Heap                      **********************/


int swap (int *vetor, int indiceA, int indiceB){
	int aux = vetor[indiceA];
	vetor[indiceA] = vetor[indiceB];
	vetor[indiceB] = aux;
	
	return 1;
}

int restaura_heap_up (int *heap, int *dist, int tam){
	int filho = tam - 1;
	int pai;
	
	if (filho%2 == 0)
		pai = (filho>>1) - 1;
	else
		pai = filho>>1;
	
	
	while (filho > 0 && dist[heap[pai]] > dist[heap[filho]]){
		swap (heap, pai, filho);
		filho = pai;
		if (filho%2 == 0)
			pai = (filho>>1) - 1;
		else
			pai = filho>>1;
	}
	
	return 1;
}

int restaura_heap_down (int *heap, int *dist, int indice, int tam){
	int pai = indice;
	int filhoEsq = (pai << 1) + 1;
	int filhoDir = (pai << 1) + 2;
	int filhoMenor;
	
	while (filhoEsq < tam){
		
		filhoMenor = filhoEsq;
		if (filhoDir < tam && dist[heap[filhoDir]] < dist[heap[filhoEsq]])
			filhoMenor = filhoDir;
		
		
		if (dist[heap[pai]] < dist[heap[filhoMenor]]){
			break;
		}else {
			swap (heap, pai, filhoMenor);
		}
		
		pai = filhoMenor;
		filhoEsq = (pai << 1) + 1;
		filhoDir = (pai << 1) + 2;
	}
	
	return 1;
}

int remove_min_heap(int *heap, int *tam, int *dist){
	(*tam) = (*tam) - 1;
	
	if (*tam > 0){
		swap (heap, 0, *tam);
		restaura_heap_down (heap, dist, 0, *tam);
	}
	
	return heap[*tam];
}

int adiciona_min_heap (int *heap, int *tam, int *dist, int no){
	heap[*tam] = no;
	*tam = (*tam) + 1;
	restaura_heap_up(heap, dist, *tam);
}


/*******************                  Digraph Initialization Functions                      *********************/

int inicializa_digrafo(TDigrafo *g, TId n) {
  g->vertices = (TVertice*) malloc (sizeof(TVertice[n]));
  if (g->vertices == NULL)
    return 0;
  g->n = n;
  g->m = 0;
  int i;
  for (i=0; i < n; i++) {
    g->vertices[i].direto = NULL;
    g->vertices[i].grauEntrada = 0;
    g->vertices[i].grauSaida = 0;
    g->vertices[i].peso = 0;
  }
  return 1;
}

TDigrafo * cria_digrafo(TId n) {
  TDigrafo * g = (TDigrafo *) malloc (sizeof(TDigrafo));
  if (g == NULL)
    return NULL;
  if (inicializa_digrafo(g, n)){
    return g;
  }else {
    free(g);
    return NULL;
  }
}

const TAresta * conectar_vertices(TDigrafo *g, TId u, TId v, TPeso peso){
	
	if (u < g->n && v < g->n){													// Verifies if the nodes are valid
		g->vertices[u].grauSaida = g->vertices[u].grauSaida + 1; 				// updates the number of edges comming out from node u
		g->vertices[v].grauEntrada = g->vertices[v].grauEntrada + 1; 			// updates the number of edges comming into node v		
		g->m = g->m + 1;														// updates the digraph edges number
		
		TNoLista * no = malloc(sizeof(TNoLista));								// creates new node for edges list of u node
		
		if (no == NULL){
			printf("Nao foi possivel alocar noh\n");								// Verifies the node allocation
			return NULL;
		}

		no->aresta.destino = v;													
		no->aresta.peso = peso;													// update edge's destiny, weight and name			
		
		if (g->vertices[u].direto != NULL){										// Verifies if the edegs list is not empty
			TNoLista * aux = g->vertices[u].direto;								// take the first item from the edges list
			
			while (aux->prox != NULL  && aux->prox->aresta.destino < v)
				aux = aux->prox;												// looks for the node's place on the edges list (sorted by node)											
			
			no->prox = aux->prox;												// updates new node's next pointer and inserts the new node on the list
			aux->prox = no;														
		}else {	
			g->vertices[u].direto = no;											// inserts node in the beggining of the list if empty
			no->prox = NULL;
		}

		TAresta * aresta = &(no->aresta);
		return aresta;															// takes and returns the edge's adress
		
	}else {
		printf("Vertice invalido. Nao foi possivel inserir aresta!\n");			
	}
	
	return NULL;																// returns NULL if the edge is invalid
}


/*******************    Algorithms for searching for the smallest costs between each pair of nodes  **********************/
/*******************                       Also some auxilary functions                             **********************/

int tem_aresta_negativa (TDigrafo *g){
	int i;
	int n = g->n;
	for (i = 0; i < n; i++) {	// Go through the graph's nodes
		TNoLista * aux = g->vertices[i].direto;		// Take the first edge coming out from the node, if there is any

		while (aux != NULL){
			if (aux->aresta.peso < 0)				// Looks for a negative edge coming out from the node
				return 1;							// retruns TRUE if a negative edge is found
				
			aux = aux->prox;
		}		
	}
	
	return 0;										// Didn´t find a negative edge. Returns False
}

void matriz_para_arquivo(int **custos, int n, FILE *saida){
	fprintf(saida, "%d\n", n);
	
	for (int i=0; i<n; i++){
		for (int j=0; j<n; j++){
			if (custos[i][j] != INFINITO){
				fprintf(saida, "%d", custos[i][j]);
			}else {
				fprintf(saida, "inf");
			}
			
			if (j == n-1){
				fprintf(saida, "\n");
			}else {
				fprintf(saida, " ");
			}
		}
	}
}  

int ** floyd_warshal (TDigrafo *g){
	int n = g->n;
	int **custos = (int**)malloc(sizeof(int*[n]));
	
	for (int i = 0; i<n; i++)
		custos[i] = (int*) malloc(sizeof(int[n]));	
	
	for (int i = 0; i<n; i++){
		for (int j = 0; j<n; j++){
			if (i != j){
				custos[i][j] = INFINITO;						// Matrix with costs filled with infinity (i!=j)
			} else												// and zeros (i=j)
				custos[i][j] = 0;
		}
	}
	
	
	for (int i = 0; i<n; i++) {									// Goes through nodes
		TNoLista *aux = g->vertices[i].direto;					// Gets list of edges
		while (aux != NULL){
			custos[i][aux->aresta.destino] = aux->aresta.peso;  // Fill the matrix with the costs of the edges
			aux=aux->prox;
		}
	}
	
	for (int k = 0; k < n; k++){
		for (int j = 0; j < n; j++){              
			for (int i = 0; i < n; i++){
				if ((custos[i][k] != INFINITO)&&(custos[k][j] != INFINITO)&&
				(custos[i][j] > custos[i][k] + custos[k][j])) {
					custos[i][j] = custos[i][k] + custos[k][j];	// Fill the matrix with the costs of the smallest paths
				}
			}
		}
	}
	
	return custos;
}   	

int * bellmand_ford (TDigrafo *g, int origem, int *has_negative_cycle){
	int n = g->n;
	int *dist_min = (int *) malloc(n*sizeof(int));												// Smallest costs
	
	for (int i=0; i<n; i++)
		dist_min[i] = INFINITO;									// Infinity for all distances
	
	dist_min[origem] = 0;										// Distance to the node itself set to zeros
 
	for (int i=0; i<n; i++){										// n iteractions
		
		for (int i = 0; i<n; i++) {									// Goes through nodes
			TNoLista *aux = g->vertices[i].direto;					// Gets list of edges
			while (aux != NULL){
				if (dist_min[i] + aux->aresta.peso < dist_min[aux->aresta.destino]){
					dist_min[aux->aresta.destino] = dist_min[i] + aux->aresta.peso;
				}
				aux = aux->prox;
			}
		}
	}
	
	for (int i = 0; i<n; i++) {	
		TNoLista *aux = g->vertices[i].direto;					
		while (aux != NULL){
			if (dist_min[i] + aux->aresta.peso < dist_min[aux->aresta.destino]){
				*has_negative_cycle = 1;											// extra iteration to check for negative cicle
				i = n;
				break;
			}
			aux = aux->prox;
		}	
	}
	
	return dist_min;
}	   

int * dijkstra (TDigrafo *g, int origem){
	int n = g->n;
	int * dist = malloc(n*sizeof(int));
	int heap[n];
	int tam = 0;
	
	dist[origem] = 0;													// Distance to source equals zero
	for (int j=0; j<n; j++){
		if (j != origem)
			dist[j] = INFINITO;												// Fills dist vector with infinity
		
		adiciona_min_heap (heap, &tam, dist, j);							// Puts node on min heap
	}
	
	int no;
	while (tam > 0){													// While heap is not empty
		no = remove_min_heap(heap, &tam, dist);
		int dist_alt;
		TNoLista * aux = g->vertices[no].direto;						// List of neighbors
		
		while (aux != NULL){											// While there is any neighbor
			dist_alt = dist[no] + aux->aresta.peso;
			
			if (dist_alt < dist[aux->aresta.destino]){
				dist[aux->aresta.destino] = dist_alt;					// Priority reduced
				for (int k=0; k<tam; k++){
					if (heap[k] == aux->aresta.destino){				
						restaura_heap_up (heap, dist, k+1); 			// Fixes heap
						break;
					}
				}
			}
			
			aux = aux->prox;
		}
	}
	
	return dist;
}	

int ** johnson (TDigrafo *g){
	int n = g->n;
	int negative_cycle_johnson = 0;
	int *custo_de_novo_no = (int*) malloc(sizeof(int[n]));
	
	custo_de_novo_no = bellmand_ford (g, n-1, &negative_cycle_johnson);			// Gets new costs with Bellmand Ford
	
	if (!negative_cycle_johnson) {
		TNoLista *aux;					
		for (int i=0; i<n; i++){								// Goes through nodes
			aux = g->vertices[i].direto;					 	// Gets list of edges
				while (aux != NULL){
					aux->aresta.peso = aux->aresta.peso + custo_de_novo_no[i] - custo_de_novo_no[aux->aresta.destino]; // Updates edges' weights
					aux = aux->prox;
				}
		}
		
		g->n = g->n - 1; 											// extra node is removed
		n = n - 1;
	
		int ** custos = (int**)malloc(sizeof(int*[n]));
		for (int i = 0; i<n; i++)											// Allocates the matrix
			custos[i] = (int*) malloc(sizeof(int[n]));
		
		int * linha;
		for (int i=0; i<n; i++){											// Runs Dijkstra for each node
			linha = dijkstra (g, i);
			for (int j=0; j<n; j++){
				custos[i][j] = linha[j];									// Puts each line on matrix
			}
			free(linha);
		}
		
		for (int i=0; i<n; i++)
			for (int j=0; j<n; j++)
				custos[i][j] = custos[i][j] - custo_de_novo_no[i] + custo_de_novo_no[j];  // Fixes matrix of costs
		
		return custos;
	}
	
	return NULL;												// Has negative cycle
}		
	

/*******************            Main                *********************/   

int main (int argc, char **argv) {
	
	// Verifies if the correct numbers of arguments were passed
	if (argc != 4) {
    printf("Quantidade de armgumentos invalidas!\nInforme: algoritmo grafo arquivo_de_saida\n");
    return 1;
	}

  // Tries to open the digraph's file
  FILE *fdigrafo = fopen(argv[2], "r");
 
  if (fdigrafo == NULL){
	  printf("Nao foi possivel abrir o grafo!\n");  	// Couldn't open digraph's file   		
	  return 2;
  }
  
  // Gets the string representing the algorithm
  char * salgoritmo = argv[1];
  int algoritmo = INVALIDO;
  // Verifies which algorithm was passed as argument
  if (strcmp(salgoritmo, "floyd") == 0)
	  algoritmo = FLOYD_WARSHAL;
  else if (strcmp(salgoritmo, "bellman") == 0)
	  algoritmo = BELLMAND_FORD;
  else if (strcmp(salgoritmo, "dijkstra") == 0)
	  algoritmo = DIJKSTRA;
  else if (strcmp(salgoritmo, "johnson") == 0)
	  algoritmo = JOHNSON;
  
  
  // Loads the digraph into our structure representation    
 
  int n;
  int m;
  int custo;
  int u;													// u and v are nodes
  int v;
  
  TDigrafo * digrafo = NULL;
  
  char letra;
  
  if (algoritmo != INVALIDO){		// Initializes graph if it isn't invalid
	  fscanf(fdigrafo, "%c %d %d\n", &letra, &n, &m);				// Gets the number of nodes and edges
	  
	  if (letra == 'G'){  
	  
		if (algoritmo == JOHNSON)
			digrafo = cria_digrafo(n+1);									// Initializes Graph for johnson
		else
			digrafo = cria_digrafo(n);										// Initializes Graph for others algorithms
	  }else {
		printf("Formato do grafo invalido. Arquivo de saida nao foi gerado!"); 		// Invalid graph
		return 2;
	  }
	 
	  while (fscanf(fdigrafo, "%c %d %d %d\n", &letra, &u, &v, &custo) == 4){			 
		if (letra = 'E'){
			conectar_vertices(digrafo, u, v, custo);							// Connects two nodes with an edge
		}else{
			printf("Formato do grafo invalido. Arquivo de saida nao foi gerado!"); 		// Invalid graph
			return 2;
		}	
	  }
	  
	  if (algoritmo == JOHNSON) {
		  for (int i=0; i<n-1; i++)
			  conectar_vertices(digrafo, n, i, 0);										// Connects extra edge for Johnson
	 }
  } 
  // Opens the file to write the matrix representing costs
  
  FILE *saida = fopen(argv[3], "w");
  
  int ** custos;
  // Executes the proper algorithm
  switch(algoritmo){											
	
	case INVALIDO :
		printf("Algoritmo nao reconhecido. Arquivo de saida nao foi gerado!\n");	// Didn´t recognize the algorithm
		fclose(saida);
		break;
		
	case FLOYD_WARSHAL:
		custos = floyd_warshal(digrafo);								// gets the matrix using Floyd Warshal			
		int floyd_negative_cycle = 0;
		for (int i = 0; i<n; i++){
			if (custos[i][i] < 0){
				floyd_negative_cycle = 1;									// Looks for negative cycles
				fprintf(saida, "ciclo negativo\n");
				break;
			}		
		}
		
		if (!floyd_negative_cycle)	
			matriz_para_arquivo(custos, n, saida);							// If there isn't a negative cycle puts the matrix in the file
		
		break;
		
	case BELLMAND_FORD :
		custos = (int**)malloc(sizeof(int*[n]));
		for (int i = 0; i<n; i++)											// Allocates the matrix
			custos[i] = (int*) malloc(sizeof(int[n]));
		
		int *linha;
		int bellmand_negative_cycle = 0;
		for (int i=0; i<n; i++){
			linha = bellmand_ford(digrafo, i, &bellmand_negative_cycle);
			if (!bellmand_negative_cycle){
				for (int j=0; j<n; j++){									// Checks for negative cicles and puts line on matrix
					custos[i][j] = linha[j];
				}
			}else{
				fprintf(saida, "ciclo negativo\n");
				break;
			}
		}
		
		if (!bellmand_negative_cycle)
			matriz_para_arquivo(custos, n, saida);							// If there isn't a negative cycle puts the matrix in the file
		
		break;
		
	case DIJKSTRA :
		if (!tem_aresta_negativa(digrafo)){
			
			custos = (int**)malloc(sizeof(int*[n]));
			for (int i = 0; i<n; i++)											// Allocates the matrix
				custos[i] = (int*) malloc(sizeof(int[n]));
			
			int * linha;
			for (int i=0; i<n; i++){											// Runs Dijkstra for each node
				linha = dijkstra (digrafo, i);
				for (int j=0; j<n; j++){
					custos[i][j] = linha[j];									// Puts each line on matrix
				}
				free(linha);
			}
			
			matriz_para_arquivo(custos, n, saida);
		}else{
			fprintf(saida, "negativo\n");
		}
		
		break;
		
	case JOHNSON :
		custos = johnson(digrafo);
		if (custos == NULL)
			fprintf(saida, "negativo\n");
		else
			matriz_para_arquivo(custos, n, saida);
		
		break;
  }
  
  return 0;
}
