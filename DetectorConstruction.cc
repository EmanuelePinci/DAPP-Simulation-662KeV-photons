//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file DetectorConstruction.cc
/// \brief Implementation of the DetectorConstruction class
//
//
// $Id: DetectorConstruction.cc 101905 2016-12-07 11:34:39Z gunter $
//
// 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

/* Nel DetectorConstruction di AnaEx01 il calorimetro era un parallelepipedo diviso a strati. Per 
   il nostro progetto non ne abbiamo bisogno perché il rivelatore è un tronco di piramide. Dunque 
   innanzitutto modifichiamo la forma del detector commentando tutte le righe di codice relative 
   alla costruzione del vecchio calorimetro sostituendole con il solido G4Trd. G4Trd è un trapezoide
   i cui parametri rappresentano la semimisura delle dimensioni. */

#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4Box.hh" // Questa classe viene laciata perché è utilizzata per costruire il WORLD
#include "G4Trd.hh" // Includiamo la classe per costruire il trapezoide
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"

#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4SystemOfUnits.hh"
#include "G4RunManager.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction()
:G4VUserDetectorConstruction(),
 fAbsorberMaterial(0),fGapMaterial(0),fDefaultMaterial(0),
 fSolidWorld(0),fLogicWorld(0),fPhysiWorld(0),
 fSolidCalor(0),fLogicCalor(0),fPhysiCalor(0),
 fSolidLayer(0),fLogicLayer(0),fPhysiLayer(0),
 fSolidAbsorber(0),fLogicAbsorber(0),fPhysiAbsorber(0),
 fSolidGap (0),fLogicGap (0),fPhysiGap (0),
 fDetectorMessenger(0)
{
  // default parameter values of the calorimeter
  fAbsorberThickness = 10.*mm;
  fGapThickness      =  5.*mm;
  fNbOfLayers        = 10;
  fCalorSizeYZ       = 10.*cm;
  ComputeCalorParameters();
  
  // materials
  DefineMaterials();
  SetAbsorberMaterial("G4_Pb");
  SetGapMaterial("G4_lAr");
  
  // create commands for interactive definition of the calorimeter
  fDetectorMessenger = new DetectorMessenger(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{ delete fDetectorMessenger;}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  return ConstructCalorimeter();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::DefineMaterials()
{ 
// use G4-NIST materials data base
//
G4NistManager* man = G4NistManager::Instance();
// Il detector è un cristallo di NaI(Tl), definiamo solo il NaI poiché il Tl è presente 
// in percentuali molto basse (0.1%) e serve più che altro per la rivelazione dei gamma dai PMTs 
fDefaultMaterial = man->FindOrBuildMaterial("G4_Galactic");
fAbsorberMaterial = man->FindOrBuildMaterial("G4_SODIUM_IODIDE"); 
/*man->FindOrBuildMaterial("G4_Pb");
man->FindOrBuildMaterial("G4_lAr");*/

// print table
//
G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::ConstructCalorimeter()
{

  // Clean old geometry, if any
  //
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();

  // complete the Calor parameters definition
  ComputeCalorParameters();
   
  //     
  // World
  //
  fSolidWorld = new G4Box("World",                                //its name
                   fWorldSizeX/2,fWorldSizeYZ/2,fWorldSizeYZ/2);  //its size
                         
  fLogicWorld = new G4LogicalVolume(fSolidWorld,            //its solid
                                   fDefaultMaterial,        //its material
                                   "World");                //its name
                                   
  fPhysiWorld = new G4PVPlacement(0,                        //no rotation
                                 G4ThreeVector(),           //at (0,0,0)
                                 fLogicWorld,             //its logical volume
                                 "World",                   //its name
                                 0,                         //its mother  volume
                                 false,                  //no boolean operation
                                 0);                        //copy number
  

  // Le prossime righe riguardano il vecchio detector per il momento le commentiamo 

  /*                               
  //                               
  // Calorimeter
  //  
  fSolidCalor=0; fLogicCalor=0; fPhysiCalor=0;
  fSolidLayer=0; fLogicLayer=0; fPhysiLayer=0;
  
  if (fCalorThickness > 0.)  
    { fSolidCalor = new G4Box("Calorimeter",                //its name
                    fCalorThickness/2,fCalorSizeYZ/2,fCalorSizeYZ/2);//size
                                 
      fLogicCalor = new G4LogicalVolume(fSolidCalor,        //its solid
                                        fDefaultMaterial,   //its material
                                        "Calorimeter");     //its name
                                           
      fPhysiCalor = new G4PVPlacement(0,                    //no rotation
                                     G4ThreeVector(),       //at (0,0,0)
                                     fLogicCalor,           //its logical volume
                                     "Calorimeter",         //its name
                                     fLogicWorld,           //its mother  volume
                                     false,              //no boolean operation
                                     0);                    //copy number
  
  //                                 
  // Layer
  //
      fSolidLayer = new G4Box("Layer",                        //its name
                       fLayerThickness/2,fCalorSizeYZ/2,fCalorSizeYZ/2); //size
                       
      fLogicLayer = new G4LogicalVolume(fSolidLayer,        //its solid
                                       fDefaultMaterial,    //its material
                                       "Layer");            //its name
      if (fNbOfLayers > 1)                                      
        fPhysiLayer = new G4PVReplica("Layer",              //its name
                                     fLogicLayer,           //its logical volume
                                     fLogicCalor,           //its mother
                                     kXAxis,              //axis of replication
                                     fNbOfLayers,           //number of replica
                                     fLayerThickness);      //witdth of replica
      else
        fPhysiLayer = new G4PVPlacement(0,                  //no rotation
                                     G4ThreeVector(),       //at (0,0,0)
                                     fLogicLayer,           //its logical volume
                                     "Layer",               //its name
                                     fLogicCalor,           //its mother  volume
                                     false,               //no boolean operation
                                     0);                    //copy number     
    }                                   
  
  //                               
  // Absorber
  //
  fSolidAbsorber=0; fLogicAbsorber=0; fPhysiAbsorber=0;  
  
  if (fAbsorberThickness > 0.) 
    { fSolidAbsorber = new G4Box("Absorber",                //its name
                          fAbsorberThickness/2,fCalorSizeYZ/2,fCalorSizeYZ/2); 
                          
      fLogicAbsorber = new G4LogicalVolume(fSolidAbsorber,    //its solid
                                            fAbsorberMaterial, //its material
                                            fAbsorberMaterial->GetName());//name
                                                
      fPhysiAbsorber = new G4PVPlacement(0,                   //no rotation
                          G4ThreeVector(-fGapThickness/2,0.,0.),  //its position
                                        fLogicAbsorber,     //its logical volume
                                        fAbsorberMaterial->GetName(), //its name
                                        fLogicLayer,          //its mother
                                        false,               //no boulean operat
                                        0);                   //copy number
                                        
    }
  
  //                                 
  // Gap
  //
  fSolidGap=0; fLogicGap=0; fPhysiGap=0; 
  
  if (fGapThickness > 0.)
    { fSolidGap = new G4Box("Gap",
                               fGapThickness/2,fCalorSizeYZ/2,fCalorSizeYZ/2);
                               
      fLogicGap = new G4LogicalVolume(fSolidGap,
                                           fGapMaterial,
                                           fGapMaterial->GetName());
                                           
      fPhysiGap = new G4PVPlacement(0,                      //no rotation
               G4ThreeVector(fAbsorberThickness/2,0.,0.),   //its position
                                   fLogicGap,               //its logical volume
                                   fGapMaterial->GetName(), //its name
                                   fLogicLayer,             //its mother
                                   false,                   //no boulean operat
                                   0);                      //copy number
    }
  */ 

  // Costruiamo il nuovo detector

  // Dobbiamo ricordare che il constructor di G4Trd prende in input le semilunghezze delle dimensioni
  G4double pdx1 = (11.7*0.5)*cm, pdy1 = (8.3*0.5)*cm; // <-- Base maggiore
  G4double pdx2 = (1.8*0.5)*cm, pdy2 = (1.2*0.5)*cm; // <-- Base minore
  G4double h = (24.4*0.5)*cm; // <-- Altezza totale del detector

  // Ora instanziamo il detector

  G4Trd* solidDetector = new G4Trd("Solid_Detector", pdx1, pdx2, pdy1, pdy2, h);

  // Creiamo lo spazio logico dove posizioneremo il detector

  G4LogicalVolume* logicDetector = new G4LogicalVolume(
      solidDetector, // Passiamo le seguenti informazioni: detector,
      fAbsorberMaterial, // il materiale con cui è fatto il detector
      "Logic_Detector" // una stringa che rappresenta il nome dello spazio logico
  );

  // Ora dobbiamo posizionare il detector nello spazio fisico

  // Inizialmente il detector ha l'asse || all'asse z, per convenienza vogliamo che sia || a y

  G4RotationMatrix* rotationMatrix = new G4RotationMatrix(); // <-- Applichiamo una rotazione
  rotationMatrix->rotateX(90.0*CLHEP::deg);

  fPhysiAbsorber = new G4PVPlacement(
      rotationMatrix, // Il primo parametro rappresenta una rotazione --> c'è rotazione di 90° da z a t
      G4ThreeVector(0,0,0), // Posizione del centro del SdR
      logicDetector, // Lo spazio logico
      "Physical_Detector", // Nome dello spazio fisico
      fLogicWorld, // Universo della simulazione 
      false, // Operazioni booleane per creare solidi con intersezioni o unioni booleane
      0, // Copy number per definire il numero di copie del detector che devono essere poste
      true // Controlla che non ci siano sovrapposizioni
  );


  PrintCalorParameters();     
  
  //                                        
  // Visualization attributes
  //
  fLogicWorld->SetVisAttributes (G4VisAttributes::GetInvisible());

  G4VisAttributes* simpleBoxVisAtt= new G4VisAttributes(G4Colour(1.0,1.0,1.0));
  simpleBoxVisAtt->SetVisibility(true);
  logicDetector->SetVisAttributes(simpleBoxVisAtt);

  //
  //always return the physical World
  //
  return fPhysiWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::PrintCalorParameters()
{
  /*G4cout << "\n------------------------------------------------------------"
         << "\n---> The calorimeter is " << fNbOfLayers << " layers of: [ "
         << fAbsorberThickness/mm << "mm of " << fAbsorberMaterial->GetName() 
         << " + "
         << fGapThickness/mm << "mm of " << fGapMaterial->GetName() << " ] " 
         << "\n------------------------------------------------------------\n";*/
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetAbsorberMaterial(G4String materialChoice)
{
  /*// search the material by its name   
  G4Material* pttoMaterial =
  G4NistManager::Instance()->FindOrBuildMaterial(materialChoice);      
  if (pttoMaterial)
  {
      fAbsorberMaterial = pttoMaterial;
      if ( fLogicAbsorber )
      {
          fLogicAbsorber->SetMaterial(fAbsorberMaterial);
          G4RunManager::GetRunManager()->PhysicsHasBeenModified();
      }
  }*/
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetGapMaterial(G4String materialChoice)
{
  /*// search the material by its name
  G4Material* pttoMaterial =  
  G4NistManager::Instance()->FindOrBuildMaterial(materialChoice);   
  if (pttoMaterial)
  {
      fGapMaterial = pttoMaterial;
      if ( fLogicGap )
      {
          fLogicGap->SetMaterial(fGapMaterial);
          G4RunManager::GetRunManager()->PhysicsHasBeenModified();
      }
  }*/
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetAbsorberThickness(G4double val)
{
  /*// change Absorber thickness and recompute the calorimeter parameters
  fAbsorberThickness = val;
  G4RunManager::GetRunManager()->ReinitializeGeometry();*/
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetGapThickness(G4double val)
{
  /*// change Gap thickness and recompute the calorimeter parameters
  fGapThickness = val;
  G4RunManager::GetRunManager()->ReinitializeGeometry();*/
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetCalorSizeYZ(G4double val)
{
  /*// change the transverse size and recompute the calorimeter parameters
  fCalorSizeYZ = val;
  G4RunManager::GetRunManager()->ReinitializeGeometry();*/
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetNbOfLayers(G4int val)
{
  /*fNbOfLayers = val;
  G4RunManager::GetRunManager()->ReinitializeGeometry();*/
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
