#pragma once

#include "CoreMinimal.h"
#include "CellClass.h"
#include "GameFramework/Actor.h"
#include "ConwayAutomata.generated.h"

UCLASS()
class CONWAY_CELL_AUTOMATA_API AConwayAutomata : public AActor
{
	GENERATED_BODY()
	
public:	

	UPROPERTY(Category = Transform, EditAnywhere)
		class USceneComponent* sceneComponent;

	//Prefab de una celda
	UPROPERTY(Category = Spawning, EditAnywhere, BlueprintReadWrite)
		UClass* genericCellPrefab;

	//Es el porcentaje que representa cuanto se llenará inicialmente la mazmorra
	UPROPERTY(Category = Spawning, EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = "0" , ClampMax = "100"))
		int randomFillPercent;

	//Ancho del mapa de la mazmorra
	UPROPERTY(Category = Spawning, EditAnywhere, BlueprintReadWrite)
		int width;

	//Altura del mapa de la mazmorra
	UPROPERTY(Category = Spawning, EditAnywhere, BlueprintReadWrite)
		int height;

	//Color del suelo
	UPROPERTY(Category = Spawning, EditAnywhere, BlueprintReadWrite)
		FLinearColor floorColor;

	//Color de pared
	UPROPERTY(Category = Spawning, EditAnywhere, BlueprintReadWrite)
		FLinearColor wallColor;

private:
	
	//Array que representa el mapa de la mazmorra
	TArray<int> map;

	//Lista de secciones que contendran las casillas que componen cada seccion
	TArray<TArray<FVector2D>> sectionsList;

	//Lista auxiliar que recogerá la casillas de una seccion
	TArray<FVector2D> auxList;

	//Número de veces que se aplicarán las reglas
	const int numSteps =5;

	//Numero de paredes a tu alrededor para considerarte solo
	const int alone = 4;

	//Estado del generador de mapa
	enum AutomataState { IDLE, CREATING };
	AutomataState automataState;

	//Lista de celdas del mapa
	TArray<ACellClass*> cellList;

	//Referencia al player Controller
	APlayerController* controller;

	//Constructor
	AConwayAutomata();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	// Reglas del mapa 
	void MapRule();

	//Corrector del mapa
	void MapFixer();

	//Almacena las secciones
	void GetSections();

	//Une dos secciones
	void UnirSecciones(TArray<FVector2D> secA, TArray<FVector2D> secB);

	//Identifica una seccion
	void Section(int cell_x, int cell_y);

	// Obtiene el número de celdas vecinas de tipo pared de una posición pasada como parámetro
	int GetNeighbour(int cell_x, int cell_y);

	//Comprueba que una celda no este en ninguna seccion
	bool IsOnAnySection(FVector2D aux);

	//Genera el mapa de la mazmorra
	void GenerateMap();

	//Establecemos la base para la creacion del mapa
	void SetupMap();

	//Colorea las paredes y suelos de su color
	void ColorMap();

	//Rellena aleatoriamente el mapa con paredes y suelos
	void RandomMap();

	//Intercepta pulsación tecla
	UFUNCTION()
		void InputKey();
};
