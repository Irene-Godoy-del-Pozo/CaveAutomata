#include "ConwayAutomata.h"
#include "PaperSpriteComponent.h"


// Sets default values
AConwayAutomata::AConwayAutomata()
{
	//Hacemos que se llame a Tick() solo una vez
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AConwayAutomata::BeginPlay()
{
	Super::BeginPlay();

	//Se obtiene el playerController
	controller = GEngine->GetFirstLocalPlayerController(GetWorld());

	//Permite que se reciba el input del jugador
	EnableInput(controller);

	//Establece métodos que serán llamados al recibir los eventos del Action Mapping
	InputComponent->BindAction("GenerateMap", IE_Pressed, this, &AConwayAutomata::InputKey);

	//Creamos el mapa de la mazmorra "en blanco"
	SetupMap();

	//Generamos una mazmorra proceduralmente
	GenerateMap();

}

// Called every frame
void AConwayAutomata::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

//Función llamada con input del teclado
void AConwayAutomata::InputKey()
{
	//Si el algoritmo ha terminado de crear una mazmorra genera un mapa nuevo
	if (automataState == AutomataState::IDLE)
	{
		GenerateMap();
	}
}



#pragma region Creacion del mapa base

//Crea el canvas 
void AConwayAutomata::SetupMap()
{
	//Establecemos el tamaño del TArray que representa el mapa
	map.AddDefaulted(width * height);

	//LLenamos el mapa de actores de clase ACellClass 
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			//Averiguamos su posicion y rotacion en el mundo
			FVector* location = new FVector((-width * 100) / 2 + (x * 100) + 50, 0, (-height * 100) / 2 + (y * 100) + 50);
			FRotator* rotation = new FRotator(0, 0, 0);
			FActorSpawnParameters params;

			// Spawneamos una celda con las propiedades calculadas anteriormente
			ACellClass* cell = (ACellClass*)GetWorld()->SpawnActor(genericCellPrefab, location, rotation, params);

			//Guardamos la posición en la que está la celda dentro del array 
			cell->info = FVector2D(x, y);

			//Lo añadimos a la lista celdas
			cellList.Add(cell);
		}
	}
}

//Genera el mapa
void AConwayAutomata::GenerateMap()
{
	//Cambiamos el estado para asegurarnos de que solo se pueda generar mazmorra cuando ha terminado
	automataState = AutomataState::CREATING;

	//LLeno aleatoriamente la mazmorra
	RandomMap();

	//Paso el "corrector" a la mazmorra generada por RandomFillCave
	for (int index = 0; index < numSteps; index++)
		MapRule();

	//Fase 2- Damos una ultima pasada al mapa para evitar que aparezca suelo aislado
	MapFixer();

	//Fase 3- Identificamos las regiones aisladas 
	GetSections();

	//Unimos cada seccion con la siguiente
	for (int j = 0; j < sectionsList.Num(); j++)
	{

		if ((j + 1) < sectionsList.Num())
		{
			UnirSecciones(sectionsList[j], sectionsList[j + 1]);
		}
	}
	//Coloreamos las paredes y el suelo
	ColorMap();

	//Devolvemos el estado a idle
	automataState = AutomataState::IDLE;
}

#pragma endregion 

#pragma region Fase 1 - Creacion de salas procedurales

//Llena aleatoriamente la mazmorra de suelos y paredes
void AConwayAutomata::RandomMap()
{
	//Recorremos el mapa
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			//Establecemos un "marco" a nuestro mapa para que no pueda generar habitaciones pegado a los bordes de este
			if (x == 0 || x == width - 1 || y == 0 || y == height - 1)
				map[(y * width) + x] = 1;
			else
			{
				//Si el numero aleatorio esta por debajo de nuestro límite será pared
				if (FMath::FRandRange(0, 100) < randomFillPercent)
					map[(y * width) + x] = 1;
				else
					map[(y * width) + x] = 0;
			}
		}
	}
}

// Reglas que por las que , apartir de un disposicion aleatoria, generaremos las habitaciones.
void AConwayAutomata::MapRule()
{
	//Recorremos el mapa
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			//Obtenemos las celdas de alrededor
			int neighbourCellTiles = GetNeighbour(x, y);

			//Si está sola será pared
			if (neighbourCellTiles == alone)
			{
				map[(width * y) + x] = 1;

			}
			//Si no
			else if (neighbourCellTiles < alone)
			{
				map[(width * y) + x] = 0;

			}
		}
	}
}

// Obtiene el número de paredes vecinas a la celda pasada
int AConwayAutomata::GetNeighbour(int cell_x, int cell_y)
{
	int cellCount = 0;

	//Recorre un grid de +-1 en sentido horizontal
	for (int neighbourX = cell_x - 1; neighbourX <= cell_x + 1; neighbourX++)
	{
		for (int neighbourY = cell_y - 1; neighbourY <= cell_y + 1; neighbourY++)
		{
			//Comprueba que no se salga de los límites del sujeto de prueba
			if (neighbourX >= 0 && neighbourX < width && neighbourY >= 0 && neighbourY < height)
			{
				//Comprobamos que no sea la misma celda pasada
				if (neighbourX != cell_x || neighbourY != cell_y)
					cellCount += map[(neighbourY * width) + neighbourX];
			}
			else
				cellCount++;
		}
	}
	return cellCount;
}

//Colorea las paredes y suelos
void AConwayAutomata::ColorMap()
{
	//Recorre el mapa
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			//Crea un array de sprites
			TArray<UActorComponent*> sprites = cellList[(y * width) + x]->GetComponentsByClass(UPaperSpriteComponent::StaticClass());

			if (sprites.Num() > 0)
			{
				//Cambia el color del Sprite al color de las paredes
				if (map[(width * y) + x] == 1)
				{
					((UPaperSpriteComponent*)sprites[0])->SetSpriteColor(wallColor);
				}
				//Cambia el color del Sprite al color del suelo
				else
				{
					((UPaperSpriteComponent*)sprites[0])->SetSpriteColor(floorColor);
				}
			}
		}
	}
}

#pragma endregion

#pragma region Fase 2 - Correccion del mapa

//Hace una ultima pasada y convierte las celdas solitarias al tipo contrario
void AConwayAutomata::MapFixer()
{
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			//Obtiene la población de células vecinas
			int neighbourCellTiles = GetNeighbour(x, y);

			if (neighbourCellTiles > alone)
				map[(width * y) + x] = 1;
			else if (neighbourCellTiles < alone)
				map[(width * y) + x] = 0;
		}
	}
}


#pragma endregion

#pragma region Fase 3 - Union de salas aisladas

//Identifica las secciones
void AConwayAutomata::GetSections()
{
	//Limpiamos la lista de secciones
	sectionsList.Empty();

	//Recorre el mapa
	for (int x = 1; x < width - 1; x++)
	{
		for (int y = 1; y < height - 1; y++)
		{
			//Convertirmos en vector con la x e y dadas
			FVector2D aux = FVector2D(x, y);

			//Comprobamos que no este en un seccion ya			
			if (IsOnAnySection(aux) == false)
			{
				//Si es suelo
				if (map[(width * y) + x] == 0)
				{
					//Limpiamos la lista de auxiliares
					auxList.Empty();

					//Averiguamos la seccion en la que esta esa celda
					Section(x, y);

					//Si la lista de auxList es mayor que dos y no esta en la lista de secciones
					if (auxList.Num() > 2 && !sectionsList.Contains(auxList))
						sectionsList.Add(auxList);

					//Debug el tamaño de celdas de una sección
					FString a;
					a.AppendInt(auxList.Num());
					GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Green, a);
				}
			}
		}
	}

	//Debug el tamaño de celdas de una sección
	FString b;
	b.AppendInt(sectionsList.Num());
	GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Red, b);
}

//Une las secciones proporcionadas
void AConwayAutomata::UnirSecciones(TArray<FVector2D> secA, TArray<FVector2D> secB)
{
	//Averiguar el centro de cada zona
	FVector2D centroA;
	FVector2D centroB;

	if (secA.Num() % 2 == 0)
		centroA = secA[(secA.Num() / 2) - 1];
	else
		centroA = secA[(secA.Num() - 1) / 2];

	if (secA.Num() % 2 == 0)
		centroB = secB[(secB.Num() / 2) - 1];
	else
		centroB = secB[(secB.Num() - 1) / 2];

	//Une los centros haciendo una escalera con limite la x y la y de la segunda sección
	do
	{
		//Si la x de la segunda seccion es mayor sumamos, sino, restamos
		if (centroA.X < centroB.X)
			centroA.X++;
		else if (centroA.X > centroB.X)
			centroA.X--;

		//Cambiamos esa celda a suelo
		if (!map[(centroA.Y * width) + centroA.X] == 0)
			map[(centroA.Y * width) + centroA.X] = 0;

		//Si la y de la segunda seccion es mayor sumamos, sino, restamos
		if (centroA.Y < centroB.Y)
			centroA.Y++;
		else if (centroA.Y > centroB.Y)
			centroA.Y--;

		//Cambiamos esa celda a suelo
		if (!map[(centroA.Y * width) + centroA.X] == 0)
			map[(centroA.Y * width) + centroA.X] = 0;

	} while (centroA != centroB);
}

//Comprobamos si una casilla esta en una seccion
bool AConwayAutomata::IsOnAnySection(FVector2D aux)
{
	//Recorremos la lista de secciones
	for (int j = 0; j < sectionsList.Num(); j++)
	{
		//Recorremos la lista de celdas de esa seccion
		for (int i = 0; i < sectionsList[j].Num(); i++)
		{
			//Si es igual que el vector dado devuelve true
			if (sectionsList[j][i].Equals(aux))
				return true;
		}

	}
	return false;
}

//Función recursiva que averigua y colecciona las celdas en una lista
void AConwayAutomata::Section(int cell_x, int cell_y)
{
	//Recorre las casillas que rodean a la celda proporcionada
	for (int neighbourX = cell_x - 1; neighbourX <= cell_x + 1; neighbourX++)
	{
		for (int neighbourY = cell_y - 1; neighbourY <= cell_y + 1; neighbourY++)
		{
			//Creamos un vector 2d con la x e y del vecino
			FVector2D auxCell = FVector2D(neighbourX, neighbourY);
			//Comprobamos que sea suelo y no esté en la lista ya
			if (map[(neighbourY * width) + neighbourX] == 0 && !auxList.Contains(auxCell))
			{
				//Añadimos a la lista
				auxList.Add(auxCell);

				//Llamamos a la misma función 
				Section(neighbourX, neighbourY);
			}
		}
	}
}
#pragma endregion




