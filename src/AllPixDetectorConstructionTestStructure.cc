/**
 * Author: John Idarraga <idarraga@cern.ch> , 2010
 *
 */

#include "AllPixDetectorConstruction.hh"

#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4Element.hh"
#include "G4ElementTable.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Orb.hh"
#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4PVPlacement.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4VPhysicalVolume.hh"

#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"

#include "G4ThreeVector.hh"

#include "G4GDMLParser.hh"

#include "G4NistManager.hh"

#include "TString.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void AllPixDetectorConstruction::BuildTestStructure(int){

	switch (m_TestStructure_type) {
	case 0:
	{
		//Beampipe

		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Berylium = nistman->FindOrBuildMaterial("G4_Be");
		//G4cout << Berylium << G4endl;
		G4Tubs * beampipe = new G4Tubs("beampipe",
				//3.0*mm, //innerRadiusOfTheTube,
				27.4*mm, //innerRadiusOfTheTube,
				28.0*mm, //outerRadiusOfTheTube,
				130*mm, //hightOfTheTube,
				0.*deg, //startAngleOfTheTube,
				360.*deg); //spanningAngleOfTheTube)

		m_TestStructure_log = new G4LogicalVolume(beampipe,
				//Bone,
				Berylium,
				"Berylium",
				0,0,0);

		//Vacuum

		G4double pressure    = 1e-6*bar;
		G4double temperature = STP_Temperature;                      //from PhysicalConstants.h

		// PV=nRT -> P= (n/V) *RT -> n/V=P/RT
		G4double density     = ((pressure/(hep_pascal))/(286.9*temperature/kelvin))*(kg/m3);
		G4Material* beam = new G4Material("Beam ", density, 2,kStateGas,temperature,pressure);

		//G4cout << "[BeamPipe] Beampipe material with " << TString::Format("P=%3.5f bar rho= %3.3f mg/cm3",pressure/bar,density/(mg/cm3)) << endl;

		// air Material
		density = 1.290*mg/cm3;
		G4Material* Air = new G4Material("Air", density,2);

		//elements
		G4double a = 14.01*g/mole;
		G4Element* elN  = new G4Element("Nitrogen","N" , 7., a);
		a = 16.00*g/mole;
		G4Element* elO  = new G4Element("Oxygen"  ,"O" , 8., a);

		Air->AddElement(elN, 0.7);
		Air->AddElement(elO, 0.3);

		beam->AddElement(elN, 0.7);
		beam->AddElement(elO, 0.3);

		G4Tubs * innerbeampipe = new G4Tubs("innerbeampipe",
				//3.0*mm, //innerRadiusOfTheTube,
				0.0*mm, //innerRadiusOfTheTube,
				27.4*mm, //outerRadiusOfTheTube,
				150*mm, //heightOfTheTube,
				0.*deg, //startAngleOfTheTube,
				360.*deg); //spanningAngleOfTheTube)

		m_TestStructure_log2 = new G4LogicalVolume(innerbeampipe,
				//Bone,
				beam,
				"vacuum",
				0,0,0);

		//beampipe
		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color(1,0,1,1));
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);
		G4RotationMatrix* matrix = new G4RotationMatrix();
		matrix->rotateX(0.*deg);

		m_TestStructure_phys = new G4PVPlacement(matrix,
				G4ThreeVector(0,0,0),
				m_TestStructure_log,
				"beampipe",
				expHall_log,
				false,
				0);
		//G4cout << "Beampipe !" << G4endl	;

		// Vacuum
		G4VisAttributes * visAtt_bp2 = new G4VisAttributes(G4Color::Black());
		visAtt_bp2->SetLineWidth(1);
		visAtt_bp2->SetForceSolid(true);
		m_TestStructure_log2->SetVisAttributes(G4VisAttributes::Invisible);
		G4RotationMatrix* matrix2 = new G4RotationMatrix();
		matrix2->rotateX(0.*deg);

		m_TestStructure_phys2 = new G4PVPlacement(matrix2,
				G4ThreeVector(0,0,0),
				m_TestStructure_log2,
				"vacuum",
				expHall_log,
				false,
				0);
		//G4cout << "Vacuum !" << G4endl;
		break;
	}
	case 1:
	{
#ifdef _EUTELESCOPE
		////////////////////////////////////////////////////////////////////////
		// Get GDML volume

		// Parsing
		G4GDMLParser parser;
		G4LogicalVolume * eud_log;

		//parser.Read("models/test_beam_telescope.gdml");
		parser.Read("share/GDML_EUDETAlHolder/EUDETAlHolder.gdml");

		//eud_log = parser.GetVolume("EUD0");
		eud_log = parser.GetVolume("Structure_105681480");

		G4VisAttributes * invisibleVisAtt = new G4VisAttributes(G4Color(1.0, 0.65, 0.0, 0.1));
		invisibleVisAtt->SetVisibility(false);

		int nDaugh = eud_log->GetNoDaughters();
		G4cout << "    Read volume has " << nDaugh << " daughters" << G4endl;

		G4VisAttributes * AlFoilVisAtt = new G4VisAttributes(G4Color(1, 1, 1, 1.0));
		AlFoilVisAtt->SetLineWidth(1);
		AlFoilVisAtt->SetVisibility(false);
		AlFoilVisAtt->SetForceSolid(true);
		eud_log->SetVisAttributes(AlFoilVisAtt);

		for(int dItr = 0 ; dItr < nDaugh ; dItr++){
			G4VPhysicalVolume * tempPhys = eud_log->GetDaughter(dItr);
			G4cout << tempPhys->GetName() << G4endl;
		}

		// I will extract the wrapper from this volume
		G4VSolid * eudAlPlaneSolid = eud_log->GetSolid();
		G4VSolid * wrapperSolid;
		G4SubtractionSolid * subtractionSolid;

		TString physName = "";

		map<int, G4ThreeVector>::iterator testStructItr = m_posVectorTestStructure.begin();

		for( ; testStructItr != m_posVectorTestStructure.end() ; testStructItr++){

			G4ThreeVector posRel = m_posVector[m_detectorLinkTestStructure[(*testStructItr).first]] - (*testStructItr).second;
			G4RotationMatrix * rotRel = m_rotVector[m_detectorLinkTestStructure[(*testStructItr).first]];
			wrapperSolid = m_wrapper_log[m_detectorLinkTestStructure[(*testStructItr).first]]->GetSolid(); // match to an specific detector
			subtractionSolid = new G4SubtractionSolid("EUDETAlPlane",
					eudAlPlaneSolid,
					wrapperSolid,
					rotRel,
					posRel); // match to an specific detector
			eud_log->SetSolid(subtractionSolid);

			physName = "test_phys_";
			physName += (*testStructItr).first;
			new G4PVPlacement(
					m_rotVectorTestStructure[(*testStructItr).first],
					(*testStructItr).second,
					eud_log,                // Logical volume
					physName.Data(),        // Name
					expHall_log,            // Mother volume logical
					false,                  // Unused boolean
					0,                      // copy number
					true);                  // overlap

			//eud_log->SetVisAttributes(AlFoilVisAtt);
		}


#endif
		break;
	}
	case 2:
	{
		//Timepix Telescope box

		//G4cout << "Structure type: " << m_TestStructure_type << G4endl;

		// materials
		G4NistManager* nistman = G4NistManager::Instance();

		G4Material * Alu = nistman->FindOrBuildMaterial("G4_Al");

		G4Box *box1=new G4Box("BoxTT1",75*mm,150*mm,216*mm);
		G4Box *box2=new G4Box("nBoxTT1",71*mm,146*mm,212*mm);
		G4Box *box3=new G4Box("window",30*mm,100*mm,215.960*mm);

		G4SubtractionSolid *boxTT1tmp = new G4SubtractionSolid("BoxTT1-nBoxTT1",box1,box2);
		G4SubtractionSolid *boxTT1 = new G4SubtractionSolid("BoxTT1tmp-window",boxTT1tmp,box3);

		m_TestStructure_log = new G4LogicalVolume(boxTT1,Alu,"box1_log",0,0,0);

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color(0.5, 0.5, 0.5,0.05));
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);

		m_TestStructure_log->SetVisAttributes(visAtt_bp);

		m_TestStructure_phys = new G4PVPlacement(0,
				G4ThreeVector(0,5*cm,21.6*cm),
				m_TestStructure_log,
				"box1_phys",
				expHall_log,
				false,
				0);

		m_TestStructure_phys2 = new G4PVPlacement(0,
				G4ThreeVector(0,5*cm,82.7*cm),
				m_TestStructure_log,
				"box2_phys",
				expHall_log,
				false,
				1);

		///////////////////////////////////////
		// DUT Aluminum box

		// Get GDML volume
		G4GDMLParser parser;
		G4LogicalVolume * world_log;

		// Read structure
		parser.Read("models/clicpix_box.gdml");
		world_log = parser.GetVolume("Structure_144055344");

		G4VisAttributes * visAtt = new G4VisAttributes(G4Color(0.5, 0.5, 0.5,0.25));
		visAtt->SetLineWidth(1);
		visAtt->SetForceSolid(true);

		// Aluminum sheet for windows
		G4Box * aluSheet = new G4Box("aluSheet",199*mm/2,40*mm/2,20*um/2);
		G4LogicalVolume * aluSheet_log = new G4LogicalVolume(aluSheet,Alu,"aluSheet_log",0,0,0);
		aluSheet_log->SetVisAttributes(visAtt);

		// DUT Wrapper ThreeVector (detID==504)
		G4ThreeVector wrapper_pos = m_wrapper_phys[504]->GetObjectTranslation();
		//G4RotationMatrix * wrapper_rot = m_wrapper_phys[504]->GetObjectRotation();

		// Loop on daughter volumes i.e. various parts of the Alu box
		int nDaugh = world_log->GetNoDaughters();
		G4cout << "Read volume has " << nDaugh << " daughters" << G4endl;
		for(int dItr = 0 ; dItr < nDaugh ; dItr++)
		{
			G4VPhysicalVolume * temp_phys = world_log->GetDaughter(dItr);
			string name = temp_phys->GetName();
			G4ThreeVector pos = temp_phys->GetObjectTranslation();   // Volume position as in GDML
			G4RotationMatrix * rot = temp_phys->GetObjectRotation(); // Volume rotation as in GDML

			// Move the box down to align window with DUT based on macro value
			map<int, G4ThreeVector>::iterator testStructItr = m_posVectorTestStructure.begin();
			for( ; testStructItr != m_posVectorTestStructure.end() ; testStructItr++)
			{
				G4ThreeVector posRel = (*testStructItr).second;
				pos.setY(pos.getY()+posRel.getY());
			}

			pos.setZ(pos.getZ()+wrapper_pos.getZ()); // Translate w.r.t. wrapper Z position

			G4LogicalVolume * temp_log = temp_phys->GetLogicalVolume();
			new G4PVPlacement(
					rot,
					pos,                    // Position of physical volume
					temp_log,               // Logical volume
					name,                   // Name
					expHall_log,            // Mother volume logical
					false,                  // Unused boolean
					0,                      // copy number
					true);                  // overlap
			temp_log->SetVisAttributes(visAtt);

			// Alu sheets
			if (dItr == 5 || dItr == 11) // Side plates
			{
				pos.setY(pos.getY()-65);
				if (dItr == 5)  pos.setZ(pos.getZ()-1*mm); // move it inside
				if (dItr == 11) pos.setZ(pos.getZ()+1*mm); // move it inside

				new G4PVPlacement(
						rot,
						pos,
						aluSheet_log,
						"aluSheet_phys",
						expHall_log,
						false,
						1,
						true);
			}

		}

		break;
	}
	case 3:
	{
		//Am241 gamma Calibration source

		G4NistManager* nistman = G4NistManager::Instance();
		//G4Material * Alu = nistman->FindOrBuildMaterial("G4_PLEXIGLASS");
		G4Material * Pb = nistman->FindOrBuildMaterial("G4_Pb");

		// Define elements from NIST
		nistman->FindOrBuildElement("H");
		nistman->FindOrBuildElement("Be");
		G4Element* C  = nistman->FindOrBuildElement("C");
		nistman->FindOrBuildElement("N");
		nistman->FindOrBuildElement("O");
		nistman->FindOrBuildElement("Al");
		G4Element* Si = nistman->FindOrBuildElement("Si");
		nistman->FindOrBuildElement("Ti");
		G4Element* Cr = nistman->FindOrBuildElement("Cr");
		G4Element* Mn = nistman->FindOrBuildElement("Mn");
		G4Element* Fe = nistman->FindOrBuildElement("Fe");
		G4Element* Ni = nistman->FindOrBuildElement("Ni");
		nistman->FindOrBuildElement("W");
		nistman->FindOrBuildElement("Au");
		nistman->FindOrBuildElement("Pb");
		// Define pure NIST materials
		nistman->FindOrBuildMaterial("G4_Al");
		nistman->FindOrBuildMaterial("G4_Ti");
		nistman->FindOrBuildMaterial("G4_W");
		nistman->FindOrBuildMaterial("G4_Au");
		// Define other NIST materials
		nistman->FindOrBuildMaterial("G4_WATER");
		nistman->FindOrBuildMaterial("G4_KAPTON");
		//G4Material* Air = nistman->FindOrBuildMaterial("G4_AIR");
		// Define materials not in NIST
		G4double density;
		G4int ncomponents;
		G4double fractionmass;
		G4Material* StainlessSteel = new G4Material("StainlessSteel", density= 8.06*g/cm3, ncomponents=6);
		StainlessSteel->AddElement(C, fractionmass=0.001);
		StainlessSteel->AddElement(Si, fractionmass=0.007);
		StainlessSteel->AddElement(Cr, fractionmass=0.18);
		StainlessSteel->AddElement(Mn, fractionmass=0.01);
		StainlessSteel->AddElement(Fe, fractionmass=0.712);
		StainlessSteel->AddElement(Ni, fractionmass=0.09);

		G4Element * Cu = nistman->FindOrBuildElement("Cu");
		G4Element * Zn = nistman->FindOrBuildElement("Zn");
		G4Material * Brass = new G4Material("Brass", density=8.5*g/cm3, ncomponents=2);
		Brass->AddElement(Cu, fractionmass=0.7);
		Brass->AddElement(Zn, fractionmass=0.3);

		G4Tubs *HolderPrim = new G4Tubs("supportTop",
				0.0*mm, //innerRadiusOfTheTube,
				29.0*mm, //outerRadiusOfTheTube,
				27*mm/2, //heightOfTheTube,
				0.*deg, //startAngleOfTheTube,
				360.*deg); //spanningAngleOfTheTube)

		G4Tubs *hole = new G4Tubs("hole",
				0.0*mm, //innerRadiusOfTheTube,
				2.5*mm, //outerRadiusOfTheTube,
				10*mm/2, //heightOfTheTube,
				0.*deg, //startAngleOfTheTube,
				360.*deg); //spanningAngleOfTheTube)

		G4SubtractionSolid *holder = new G4SubtractionSolid("HolderPrim-hole",HolderPrim,hole,0,G4ThreeVector(0,-22.5*mm,-13*mm));

		G4Box *box1=new G4Box("BoxTT1",50*mm,100*mm,25*mm);

		m_TestStructure_log = new G4LogicalVolume(holder,
				Brass,
				"box1_log",
				0,0,0);

		m_TestStructure_log2 = new G4LogicalVolume(box1,
				Pb,
				"box2_log",
				0,0,0);

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color(0.5, 0.5, 0.5,0.25));
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);

		G4VisAttributes * visAtt_bp2 = new G4VisAttributes(G4Color(0.5, 0.1, 0.3,0.25));
		visAtt_bp2->SetLineWidth(1);
		visAtt_bp2->SetForceSolid(true);

		m_TestStructure_log2->SetVisAttributes(visAtt_bp2);

		m_TestStructure_phys = new G4PVPlacement(0,
				G4ThreeVector(0,0*mm,31*mm),
				m_TestStructure_log,
				"box1_phys",
				expHall_log,
				false,
				0);

		m_TestStructure_phys2 = new G4PVPlacement(0,
				G4ThreeVector(0,0*mm,-30.0*mm),
				//G4ThreeVector(0,-22.5*mm,14.0*mm),
				m_TestStructure_log2,
				"box2_phys",
				expHall_log,
				false,
				0);
		break;
	}
	case 4:
	{
		//Fe55 Calibration source

		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Plexi = nistman->FindOrBuildMaterial("G4_PLEXIGLASS");
		G4Material * Pb = nistman->FindOrBuildMaterial("G4_Pb");

		G4Tubs *HolderPrim = new G4Tubs("supportTop",
				0.0*mm, //innerRadiusOfTheTube,
				29.0*mm, //outerRadiusOfTheTube,
				27*mm/2, //heightOfTheTube,
				0.*deg, //startAngleOfTheTube,
				360.*deg); //spanningAngleOfTheTube)

		G4Tubs *hole = new G4Tubs("hole",
				0.0*mm, //innerRadiusOfTheTube,
				5.0*mm, //outerRadiusOfTheTube,
				10*mm/2, //heightOfTheTube,
				0.*deg, //startAngleOfTheTube,
				360.*deg); //spanningAngleOfTheTube)

		G4SubtractionSolid *holder = new G4SubtractionSolid("HolderPrim-hole",HolderPrim,hole,0,G4ThreeVector(0,-22.5*mm,-10*mm));

		G4Box *box1=new G4Box("BoxTT1",50*mm,100*mm,25*mm);

		m_TestStructure_log = new G4LogicalVolume(holder,
				Plexi,
				"box1_log",
				0,0,0);

		m_TestStructure_log2 = new G4LogicalVolume(box1,
				Pb,
				"box2_log",
				0,0,0);

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color(0.5, 0.5, 0.5,0.25));
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);

		G4VisAttributes * visAtt_bp2 = new G4VisAttributes(G4Color(0.5, 0.1, 0.3,0.25));
		visAtt_bp2->SetLineWidth(1);
		visAtt_bp2->SetForceSolid(true);

		m_TestStructure_log2->SetVisAttributes(visAtt_bp2);

		m_TestStructure_phys = new G4PVPlacement(0,
				G4ThreeVector(0,0*mm,31*mm),
				m_TestStructure_log,
				"box1_phys",
				expHall_log,
				false,
				0);

		m_TestStructure_phys2 = new G4PVPlacement(0,
				G4ThreeVector(0,0*mm,-30.0*mm),
				//G4ThreeVector(0,-22.5*mm,14.0*mm),
				m_TestStructure_log2,
				"box2_phys",
				expHall_log,
				false,
				0);
		break;
	}
	case 5:
	{
		//Cd109 Calibration source

		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Alu = nistman->FindOrBuildMaterial("G4_Al");
		G4Material * Pb = nistman->FindOrBuildMaterial("G4_Pb");

		G4Tubs *HolderPrim = new G4Tubs("supportTop",
				0.0*mm, //innerRadiusOfTheTube,
				29.0*mm, //outerRadiusOfTheTube,
				27*mm/2, //heightOfTheTube,
				0.*deg, //startAngleOfTheTube,
				360.*deg); //spanningAngleOfTheTube)

		G4Tubs *hole = new G4Tubs("hole",
				0.0*mm, //innerRadiusOfTheTube,
				2.5*mm, //outerRadiusOfTheTube,
				10*mm/2, //heightOfTheTube,
				0.*deg, //startAngleOfTheTube,
				360.*deg); //spanningAngleOfTheTube)

		G4SubtractionSolid *holder = new G4SubtractionSolid("HolderPrim-hole",HolderPrim,hole,0,G4ThreeVector(0,-22.5*mm,-13*mm));

		G4Box *box1=new G4Box("BoxTT1",50*mm,100*mm,25*mm);

		m_TestStructure_log = new G4LogicalVolume(holder,
				Alu,
				"box1_log",
				0,0,0);

		m_TestStructure_log2 = new G4LogicalVolume(box1,
				Pb,
				"box2_log",
				0,0,0);

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color(0.5, 0.5, 0.5,0.25));
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);

		G4VisAttributes * visAtt_bp2 = new G4VisAttributes(G4Color(0.5, 0.1, 0.3,0.25));
		visAtt_bp2->SetLineWidth(1);
		visAtt_bp2->SetForceSolid(true);

		m_TestStructure_log2->SetVisAttributes(visAtt_bp2);

		m_TestStructure_phys = new G4PVPlacement(0,
				G4ThreeVector(0,0*mm,31*mm),
				m_TestStructure_log,
				"box1_phys",
				expHall_log,
				false,
				0);

		m_TestStructure_phys2 = new G4PVPlacement(0,
				G4ThreeVector(0,0*mm,-30.0*mm),
				m_TestStructure_log2,
				"box2_phys",
				expHall_log,
				false,
				0);
		break;
	}
	case 6:
	{
		//Am241 alpha Calibration source

		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Pb = nistman->FindOrBuildMaterial("G4_Pb");

		// Define elements from NIST
		G4Element* C  = nistman->FindOrBuildElement("C");
		G4Element* Si = nistman->FindOrBuildElement("Si");
		G4Element* Cr = nistman->FindOrBuildElement("Cr");
		G4Element* Mn = nistman->FindOrBuildElement("Mn");
		G4Element* Fe = nistman->FindOrBuildElement("Fe");
		G4Element* Ni = nistman->FindOrBuildElement("Ni");
		// Define materials not in NIST
		G4double density;
		G4int ncomponents;
		G4double fractionmass;
		G4Material* StainlessSteel = new G4Material("StainlessSteel", density= 8.06*g/cm3, ncomponents=6);
		StainlessSteel->AddElement(C, fractionmass=0.001);
		StainlessSteel->AddElement(Si, fractionmass=0.007);
		StainlessSteel->AddElement(Cr, fractionmass=0.18);
		StainlessSteel->AddElement(Mn, fractionmass=0.01);
		StainlessSteel->AddElement(Fe, fractionmass=0.712);
		StainlessSteel->AddElement(Ni, fractionmass=0.09);

		G4Tubs *disk = new G4Tubs("disk",
				0.0*mm, //innerRadiusOfTheTube,
				9.0*mm, //outerRadiusOfTheTube,
				5*mm/2, //heightOfTheTube,
				0.*deg, //startAngleOfTheTube,
				360.*deg); //spanningAngleOfTheTube)

		G4Box *box1=new G4Box("BoxTT1",50*mm,100*mm,25*mm);

		m_TestStructure_log = new G4LogicalVolume(disk,
				StainlessSteel,
				"box1_log",
				0,0,0);

		m_TestStructure_log2 = new G4LogicalVolume(box1,
				Pb,
				"box2_log",
				0,0,0);

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color(0.5, 0.5, 0.5,0.25));
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);

		G4VisAttributes * visAtt_bp2 = new G4VisAttributes(G4Color(0.5, 0.1, 0.3,0.25));
		visAtt_bp2->SetLineWidth(1);
		visAtt_bp2->SetForceSolid(true);

		m_TestStructure_log2->SetVisAttributes(visAtt_bp2);

		m_TestStructure_phys = new G4PVPlacement(0,
				G4ThreeVector(0,-22.25*mm,22*mm),
				m_TestStructure_log,
				"box1_phys",
				expHall_log,
				false,
				0);

		m_TestStructure_phys2 = new G4PVPlacement(0,
				G4ThreeVector(0,0*mm,-30.0*mm),
				//G4ThreeVector(0,-22.5*mm,14.0*mm),
				m_TestStructure_log2,
				"box2_phys",
				expHall_log,
				false,
				0);
		break;
	}
	case 7:
	{
		// Test GDML

		// Get GDML volume
		G4GDMLParser parser;
		G4LogicalVolume * world_log;

		// Read structure
		parser.Read("models/clicpix_box.gdml");
		world_log = parser.GetVolume("Structure_11624736");

		G4VisAttributes * visAtt = new G4VisAttributes( G4Color(0.5, 0.5, 0.5,0.25) );
		visAtt->SetLineWidth(1);
		visAtt->SetForceSolid(true);

		// Loop on daughter volumes i.e. various parts of the Alu box
		int nDaugh = world_log->GetNoDaughters();
		G4cout << "Read volume has " << nDaugh << " daughters" << G4endl;
		for(int dItr = 0 ; dItr < nDaugh ; dItr++){
			G4VPhysicalVolume * temp_phys = world_log->GetDaughter(dItr);
			string name = temp_phys->GetName();
			G4LogicalVolume * temp_log = temp_phys->GetLogicalVolume();
			new G4PVPlacement(0,
					temp_phys->GetObjectTranslation(), // Position of physical volume
					temp_log,               // Logical volume
					name,                   // Name
					expHall_log,            // Mother volume logical
					false,                  // Unused boolean
					0,                      // copy number
					true);                  // overlap
			temp_log->SetVisAttributes(visAtt);
		}

		break;
	}

	case 8:
	{

		G4NistManager * nistman = G4NistManager::Instance();
		G4Material * mylar = nistman->FindOrBuildMaterial("G4_MYLAR");
		TString physName = "";
		TString fr_name = "" ;
		TString bk_name = "" ;
		TString fr_name_log = "" ;
		TString bk_name_log = "" ;



		G4VisAttributes * scintAtt = new G4VisAttributes(G4Color(1,1,1,0.5));
		scintAtt->SetLineWidth(1);
		scintAtt->SetForceSolid(true);



		map<int, G4ThreeVector>::iterator testStructItr = m_posVectorTestStructure.begin();

		for( ; testStructItr != m_posVectorTestStructure.end() ; testStructItr++){

			G4ThreeVector posRel = (*testStructItr).second;

			fr_name = "fr_box_";
			fr_name += (*testStructItr).first;

			G4Box* FrontWindow = new G4Box(fr_name.Data(),
					22.5*mm,
					32.5*mm,
					25*um);

			bk_name = "bk_box_";
			bk_name += (*testStructItr).first;

			G4Box* BackWindow = new G4Box(bk_name.Data(),
					22.5*mm,
					32.5*mm,
					25*um);



			// logical
			fr_name_log = "fr_log_";
			fr_name_log += (*testStructItr).first;

			G4LogicalVolume * window_log_fr = new G4LogicalVolume(
					FrontWindow,
					mylar,
					fr_name_log.Data());
			window_log_fr->SetVisAttributes(scintAtt);


			bk_name_log = "fr_log_";
			bk_name_log += (*testStructItr).first;

			G4LogicalVolume * window_log_bk = new G4LogicalVolume(
					BackWindow,
					mylar,
					bk_name_log.Data());
			window_log_bk->SetVisAttributes(scintAtt);


			//Phyisical

			physName = "test_fr_phys_";
			physName += (*testStructItr).first;

			new G4PVPlacement( 0,
					G4ThreeVector(posRel[0],posRel[1],posRel[2]-3.0125*mm),
					window_log_fr,
					physName.Data(),
					expHall_log,
					false,
					0,
					true);


			physName = "test_bk_phys_";
			physName += (*testStructItr).first;

			new G4PVPlacement( 0,
					G4ThreeVector(posRel[0],posRel[1],posRel[2]+12.0125*mm),
					window_log_bk,
					physName.Data(),
					expHall_log,
					false,
					0,
					true);
		}


		break;
	}

	case 9:
	{

		map<int, G4ThreeVector>::iterator testStructItr = m_posVectorTestStructure.begin();
		G4ThreeVector posRel = (*testStructItr).second;

		///////////////////////////////////////////////////////////
		// Collimator
		BuildCollimator( expHall_log, posRel );

		///////////////////////////////////////////////////////////
		// Box
		G4NistManager* nistManager = G4NistManager::Instance();
		G4Material * Pb = nistManager->FindOrBuildMaterial("G4_Pb");
		G4Material * Ca = nistManager->FindOrBuildMaterial("G4_Ca");

		double boxThickness = 3*mm;
		double h_box_x = 15*cm;
		double h_box_y = 15*cm;
		double h_box_z = 25*cm;

		G4Box * theBox_1 = new G4Box("XRayBox", h_box_x, h_box_y, h_box_z);
		G4Box * theBox_2 = new G4Box("XRayBox",
				h_box_x - boxThickness,
				h_box_y - boxThickness,
				h_box_z - boxThickness);
		G4SubtractionSolid * theBox = new G4SubtractionSolid("XRayBox", theBox_1, theBox_2, 0x0, G4ThreeVector(0,0,0) );
		G4LogicalVolume * theBoxLogic = new G4LogicalVolume(
				theBox,    				// its solid
				Pb,      						// its material
				"XRayBoxLogic");  				// its name
		new G4PVPlacement(
				0,					// no rotation
				G4ThreeVector(0,0,20*cm),
				theBoxLogic,	// its logical volume
				"XRayBox",        // its name
				expHall_log,            // its mother  volume
				false,           	// no boolean operations
				0,               	// copy number
				true); 				// checking overlaps

		// mass of this box
		G4double boxVolume = theBox->GetCubicVolume() / cm3;
		G4double leadDensity = 1.13500E+01; // g/cm^3
		G4double boxMass = boxVolume * leadDensity;
		G4cout << "Box mass = " << boxMass/1000.0 << " kilograms" << G4endl;

		///////////////////////////////////////////////////////////
		// Test object
		G4Tubs * testObj = new G4Tubs("testObj", 0, 2*cm, 5*cm, pi, 2*pi);
		G4LogicalVolume * testObjLogic = new G4LogicalVolume(
				testObj,    				// its solid
				Ca,      					// its material
				"testObjLogic");
		G4RotationMatrix * pRot = new G4RotationMatrix;
		pRot->rotateX( 90*deg );

		new G4PVPlacement(
				pRot,					// no rotation
				G4ThreeVector(0,0,0*cm),
				testObjLogic,		// its logical volume
				"testObj",       	// its name
				theBoxLogic,        // its mother  volume
				false,           	// no boolean operations
				0,               	// copy number
				true); 				// checking overlaps

		G4VisAttributes* visAtt = new G4VisAttributes( G4Colour(1,0.5,1, 1) );
		visAtt->SetForceSolid( true );
		//CollVisAtt->SetForceWireframe( true );
		//visAtt->SetForceAuxEdgeVisible( true );
		testObjLogic->SetVisAttributes( visAtt );

		break;
	}
	
	case 10 :
	 {
	
		G4cout << "Building X-Ray Cabinet" << endl ;
		// materials
		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Lead = nistman->FindOrBuildMaterial("G4_Pb");

		G4Box *box1=new G4Box("OuterBox",(730./2)*mm,(730./2)*mm,(1140/2)*mm);
		G4Box *box2=new G4Box("InnerBox",(650./2)*mm,(650./2)*mm,(1060/2)*mm);

		G4SubtractionSolid *cabinet_box = new G4SubtractionSolid("OuterBox-InnerBox",box1,box2);

		m_TestStructure_log = new G4LogicalVolume(cabinet_box,Lead,"box1_log",0,0,0);

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color(0.5, 0.5, 0.5,0.05));
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(false);

		m_TestStructure_log->SetVisAttributes(visAtt_bp);

		m_TestStructure_phys = new G4PVPlacement(0,
				G4ThreeVector(0,0,0),
				m_TestStructure_log,
				"cabinet_phys",
				expHall_log,
				false,
				0);
	
	}
	case 101:
	{
		//Two cubes (purpose X0 measurement)
                G4cout << "Building cube with cutout for X0 measurements" << G4endl;

		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Aluminum = nistman->FindOrBuildMaterial("G4_Al");

		G4double box1Size = 3.0*mm;
		G4Box * box1 = new G4Box("Box1", box1Size, box1Size, box1Size);
	        			 
		//m_TestStructure_log = new G4LogicalVolume(box1, Aluminum, "Box1");


		G4double box2Size = box1Size/2.;
		
		G4Box * box2 = new G4Box("Box2", box2Size, box2Size, box2Size);

		G4SubtractionSolid *cutBox = new G4SubtractionSolid("cutBox",box1,box2,0,G4ThreeVector(0,0,box1Size));
		m_TestStructure_log = new G4LogicalVolume(cutBox, Aluminum, "cutBox");
		
		//m_TestStructure_log2 = new G4LogicalVolume(box2, Aluminum, "Box2");

		// Move the box down to align window with DUT based on macro value
		map<int, G4ThreeVector>::iterator testStructItr = m_posVectorTestStructure.begin();

		// FIXME: Assuming that there is only one TestStructure
		G4ThreeVector posRel;
		for( ; testStructItr != m_posVectorTestStructure.end() ; testStructItr++){
		  posRel = (*testStructItr).second;
		}


		G4RotationMatrix * rotMatrix;
		map<int, G4RotationMatrix*>::iterator testStructItr2 = m_rotVectorTestStructure.begin();

		for( ; testStructItr2 != m_rotVectorTestStructure.end() ; testStructItr2++){
		  rotMatrix = (*testStructItr2).second;
		}

		G4cout << "Cube tilted by deg: " << rotMatrix[0].getTheta()/deg << G4endl;

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color::Blue());
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);
		
		m_TestStructure_phys = new G4PVPlacement(rotMatrix,
				posRel,
				m_TestStructure_log,
				"Box1",
				expHall_log,
				false,
				0);
		
		break;
	}
	case 102:
	{
		//Two cubes with spherical holes inside (purpose X0 measurement)
                G4cout << "Building cube with cutout and spherical holes for X0 measurements" << G4endl;

		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Aluminum = nistman->FindOrBuildMaterial("G4_Al");

		G4double box1Size = 3.0*mm;
		G4Box * box1 = new G4Box("Box1", box1Size, box1Size, box1Size);
	        			 
		//m_TestStructure_log = new G4LogicalVolume(box1, Aluminum, "Box1");


		G4double box2Size = box1Size/2.;
		G4Box * box2 = new G4Box("Box2", box2Size, box2Size, box2Size);

		G4SubtractionSolid *cutBox = new G4SubtractionSolid("cutBox",box1,box2,0,G4ThreeVector(0,0,box1Size));
		//m_TestStructure_log = new G4LogicalVolume(cutBox, Aluminum, "cutBox");

		// Define multiple spheres and subtract them from the unit

		G4double s1size = 0.5*mm;
		G4double s1trans = 1.5*mm;
		G4Sphere * sphere1 = new G4Sphere("Sphere1", 0., s1size, 0.*deg, 360.*deg, 0.*deg, 180.*deg);
		G4SubtractionSolid *cutBoxS1 = new G4SubtractionSolid("cutBoxS1",cutBox,sphere1,0,G4ThreeVector(0,s1trans,-s1trans));

		G4double s2size = 0.2*mm;
		G4double s2trans = 1.5*mm;
		G4Sphere * sphere2 = new G4Sphere("Sphere2", 0., s2size, 0.*deg, 360.*deg, 0.*deg, 180.*deg);
		G4SubtractionSolid *cutBoxS2 = new G4SubtractionSolid("cutBoxS2",cutBoxS1,sphere2,0,G4ThreeVector(0,-s2trans,-s2trans));
		
		G4double s3size = 0.1*mm;
		G4double s3trans = 0.*mm;
		G4Sphere * sphere3 = new G4Sphere("Sphere3", 0., s3size, 0.*deg, 360.*deg, 0.*deg, 180.*deg);
		G4SubtractionSolid *cutBoxS3 = new G4SubtractionSolid("cutBoxS3",cutBoxS2,sphere3,0,G4ThreeVector(0,s3trans,-s3trans));

		G4double s4size = 0.05*mm;
		G4double s4trans = 1.5*mm;
		G4Sphere * sphere4 = new G4Sphere("Sphere4", 0., s4size, 0.*deg, 360.*deg, 0.*deg, 180.*deg);
		G4SubtractionSolid *cutBoxS4 = new G4SubtractionSolid("cutBoxS4",cutBoxS3,sphere4,0,G4ThreeVector(0,-s4trans,0));

		m_TestStructure_log = new G4LogicalVolume(cutBoxS4, Aluminum, "cutBoxS4");
		
		//m_TestStructure_log2 = new G4LogicalVolume(box2, Aluminum, "Box2");

		// Move the box down to align window with DUT based on macro value
		map<int, G4ThreeVector>::iterator testStructItr = m_posVectorTestStructure.begin();

		// FIXME: Assuming that there is only one TestStructure
		G4ThreeVector posRel;
		for( ; testStructItr != m_posVectorTestStructure.end() ; testStructItr++){
		  posRel = (*testStructItr).second;
		}


		G4RotationMatrix * rotMatrix;
		map<int, G4RotationMatrix*>::iterator testStructItr2 = m_rotVectorTestStructure.begin();

		for( ; testStructItr2 != m_rotVectorTestStructure.end() ; testStructItr2++){
		  rotMatrix = (*testStructItr2).second;
		}

		G4cout << "Cube tilted by deg: " << rotMatrix[0].getTheta()/deg << G4endl;

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color::Blue());
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);
		
		m_TestStructure_phys = new G4PVPlacement(rotMatrix,
				posRel,
				m_TestStructure_log,
				"Box1",
				expHall_log,
				false,
				0);
		
		break;
	}
	case 103:
	{
		//Two cubes with spherical holes inside (purpose X0 measurement)
                G4cout << "Building cube with cutout and cylindrical and rectangular holes  for X0 measurements" << G4endl;

		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Aluminum = nistman->FindOrBuildMaterial("G4_Al");

		G4double box1Size = 3.0*mm;
		G4Box * box1 = new G4Box("Box1", box1Size, box1Size, box1Size);
	        			 
		//m_TestStructure_log = new G4LogicalVolume(box1, Aluminum, "Box1");


		G4double box2Size = box1Size/2.;
		G4Box * box2 = new G4Box("Box2", box2Size, box2Size, box2Size);

		G4SubtractionSolid *cutBox = new G4SubtractionSolid("cutBox",box1,box2,0,G4ThreeVector(0,0,box1Size));
		//m_TestStructure_log = new G4LogicalVolume(cutBox, Aluminum, "cutBox");

		// Define multiple spheres and subtract them from the unit

		G4RotationMatrix * rotTub = new G4RotationMatrix;
		rotTub->rotateY( 90*deg );

		G4double s1size = 0.5*mm;
		G4double s1trans = 1.5*mm;
		G4Tubs * tub1 = new G4Tubs("Tub1", 0., s1size, box1Size*1.2, 0.*deg, 360.*deg);
		G4SubtractionSolid *cutBoxS1 = new G4SubtractionSolid("cutBoxS1",cutBox,tub1,rotTub,G4ThreeVector(0,s1trans,-s1trans));

		G4double s2size = 0.2*mm;
		G4double s2trans = 1.5*mm;
		G4Tubs * tub2 = new G4Tubs("Tub2", 0., s2size, box1Size*1.2, 0.*deg, 360.*deg);
		G4SubtractionSolid *cutBoxS2 = new G4SubtractionSolid("cutBoxS2",cutBoxS1,tub2,rotTub,G4ThreeVector(0,-s2trans,-s2trans));
		
		G4double s3size = 0.1*mm;
		G4double s3trans = 0.*mm;
		G4Tubs * tub3 = new G4Tubs("Tub3", 0., s3size, box1Size*1.2, 0.*deg, 360.*deg);
		G4SubtractionSolid *cutBoxS3 = new G4SubtractionSolid("cutBoxS3",cutBoxS2,tub3,rotTub,G4ThreeVector(0,s3trans,-s3trans));

		G4double s4size = 0.05*mm;
		G4double s4trans = 1.5*mm;
		G4Tubs * tub4 = new G4Tubs("Tub4", 0., s4size, box1Size*1.2, 0.*deg, 360.*deg);
		G4SubtractionSolid *cutBoxS4 = new G4SubtractionSolid("cutBoxS4",cutBoxS3,tub4,rotTub,G4ThreeVector(0,-s4trans,0));

		G4double box3Size = 0.2*mm;
		G4double box3trans = 1.5*mm;
		G4Box * box3 = new G4Box("Box3", box1Size*1.2, box3Size, box3Size);
		G4SubtractionSolid *cutBoxS5 = new G4SubtractionSolid("cutBoxS5",cutBoxS4,box3,0,G4ThreeVector(0,0,-box3trans));

		G4double box4Size = 0.1*mm;
		G4double box4trans = 1.5*mm;
		G4Box * box4 = new G4Box("Box4", box1Size*1.2, box4Size, box4Size);
		G4SubtractionSolid *cutBoxS6 = new G4SubtractionSolid("cutBoxS6",cutBoxS5,box4,0,G4ThreeVector(0,box4trans,0));


		m_TestStructure_log = new G4LogicalVolume(cutBoxS6, Aluminum, "cutBoxS6");
		
		//m_TestStructure_log2 = new G4LogicalVolume(box2, Aluminum, "Box2");

		// Move the box down to align window with DUT based on macro value
		map<int, G4ThreeVector>::iterator testStructItr = m_posVectorTestStructure.begin();

		// FIXME: Assuming that there is only one TestStructure
		G4ThreeVector posRel;
		for( ; testStructItr != m_posVectorTestStructure.end() ; testStructItr++){
		  posRel = (*testStructItr).second;
		}


		G4RotationMatrix * rotMatrix;
		map<int, G4RotationMatrix*>::iterator testStructItr2 = m_rotVectorTestStructure.begin();

		for( ; testStructItr2 != m_rotVectorTestStructure.end() ; testStructItr2++){
		  rotMatrix = (*testStructItr2).second;
		}

		G4cout << "Cube tilted by deg: " << rotMatrix[0].getTheta()/deg << G4endl;

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color::Blue());
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);
		
		m_TestStructure_phys = new G4PVPlacement(rotMatrix,
				posRel,
				m_TestStructure_log,
				"Box1",
				expHall_log,
				false,
				0);
		
		break;
	}
	case 104:
	{
		G4cout << "Building aluminum plate for X0 measurements" << G4endl;

		map<int, G4ThreeVector>::iterator testStructItr3 = m_parVectorTestStructure.begin();

		G4ThreeVector par;
		for( ; testStructItr3 != m_parVectorTestStructure.end() ; testStructItr3++){
		  par = (*testStructItr3).second;
		}

		G4cout << "Parameter vector: " << par << G4endl;

		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Aluminum = nistman->FindOrBuildMaterial("G4_Al");

		G4double thickness;
		if(par[0] != 0.){
		  thickness = par[0]/2.*mm;
		}else{
		  thickness = 0.5*mm;
		}
		G4double length = 20.*mm;
		G4Box * box1 = new G4Box("Box1", length, length, thickness);
		
		m_TestStructure_log = new G4LogicalVolume(box1, Aluminum, "Box1");
		G4cout << "Thickness of the plate: " << thickness*2. << G4endl;

		
		// Move the box down to align window with DUT based on macro value
		map<int, G4ThreeVector>::iterator testStructItr = m_posVectorTestStructure.begin();

		G4ThreeVector posRel;
		for( ; testStructItr != m_posVectorTestStructure.end() ; testStructItr++){
		  posRel = (*testStructItr).second;
		}
		posRel[2] += thickness;
		G4cout << "                   --------------------          posRel=" << posRel << G4endl;

		G4RotationMatrix * rotMatrix;
		map<int, G4RotationMatrix*>::iterator testStructItr2 = m_rotVectorTestStructure.begin();

		for( ; testStructItr2 != m_rotVectorTestStructure.end() ; testStructItr2++){
		  rotMatrix = (*testStructItr2).second;
		}

		//G4cout << "Cube tilted by deg: " << rotMatrix[0].getTheta()/deg << G4endl;

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color::Blue());
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);
		
		m_TestStructure_phys = new G4PVPlacement(rotMatrix,
				posRel,
				m_TestStructure_log,
				"Box1",
				expHall_log,
				false,
				0);
		
		break;
	}
	case 105:
	{
		G4cout << "Building 2 aluminum plates for 2D contrast tests" << G4endl;

		map<int, G4ThreeVector>::iterator testStructItr3 = m_parVectorTestStructure.begin();

		G4ThreeVector par;
		for( ; testStructItr3 != m_parVectorTestStructure.end() ; testStructItr3++){
		  par = (*testStructItr3).second;
		}

		G4cout << "Parameter vector: " << par << G4endl;

		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Aluminum = nistman->FindOrBuildMaterial("G4_Al");

		G4double thickness0 = 5./2.*mm;
		G4double thickness1 = 2.5/2.*mm;

		G4double width = 20.*mm;
		G4double height = 10.*mm;
		G4Box * box0 = new G4Box("Box0", width, height, thickness0);
		G4Box * box1 = new G4Box("Box1", width, height, thickness1);

		G4UnionSolid *bothBox = new G4UnionSolid("bothBox",box0,box1,0,G4ThreeVector(0.,20.,0.));
		
		m_TestStructure_log = new G4LogicalVolume(bothBox, Aluminum, "bothBox");

		
		// Move the box down to align window with DUT based on macro value
		map<int, G4ThreeVector>::iterator testStructItr = m_posVectorTestStructure.begin();

		G4ThreeVector posRel;
		for( ; testStructItr != m_posVectorTestStructure.end() ; testStructItr++){
		  posRel = (*testStructItr).second;
		}
		G4cout << "                   --------------------          posRel=" << posRel << G4endl;

		G4RotationMatrix * rotMatrix;
		map<int, G4RotationMatrix*>::iterator testStructItr2 = m_rotVectorTestStructure.begin();

		for( ; testStructItr2 != m_rotVectorTestStructure.end() ; testStructItr2++){
		  rotMatrix = (*testStructItr2).second;
		}

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color::Blue());
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);
		
		m_TestStructure_phys = new G4PVPlacement(rotMatrix,
				posRel,
				m_TestStructure_log,
				"bothBox",
				expHall_log,
				false,
				0);
		
		break;
	}
	case 106:
	{
		G4cout << "Building 2 LEAD (!!!) plates for 2D contrast tests" << G4endl;

		map<int, G4ThreeVector>::iterator testStructItr3 = m_parVectorTestStructure.begin();

		G4ThreeVector par;
		for( ; testStructItr3 != m_parVectorTestStructure.end() ; testStructItr3++){
		  par = (*testStructItr3).second;
		}

		G4cout << "Parameter vector: " << par << G4endl;

		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Lead = nistman->FindOrBuildMaterial("G4_Pb");

		G4double thickness0 = 5./2.*mm;
		G4double thickness1 = 2.5/2.*mm;

		G4double width = 20.*mm;
		G4double height = 10.*mm;
		G4Box * box0 = new G4Box("Box0", width, height, thickness0);
		G4Box * box1 = new G4Box("Box1", width, height, thickness1);

		G4UnionSolid *bothBox = new G4UnionSolid("bothBox",box0,box1,0,G4ThreeVector(0.,20.,0.));
		
		m_TestStructure_log = new G4LogicalVolume(bothBox, Lead, "bothBox");

		
		// Move the box down to align window with DUT based on macro value
		map<int, G4ThreeVector>::iterator testStructItr = m_posVectorTestStructure.begin();

		G4ThreeVector posRel;
		for( ; testStructItr != m_posVectorTestStructure.end() ; testStructItr++){
		  posRel = (*testStructItr).second;
		}
		G4cout << "                   --------------------          posRel=" << posRel << G4endl;

		G4RotationMatrix * rotMatrix;
		map<int, G4RotationMatrix*>::iterator testStructItr2 = m_rotVectorTestStructure.begin();

		for( ; testStructItr2 != m_rotVectorTestStructure.end() ; testStructItr2++){
		  rotMatrix = (*testStructItr2).second;
		}

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color::Blue());
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);
		
		m_TestStructure_phys = new G4PVPlacement(rotMatrix,
				posRel,
				m_TestStructure_log,
				"bothBox",
				expHall_log,
				false,
				0);
		
		break;
	}
	case 107:
	{
		//Two cubes with spherical holes inside (purpose X0 measurement)
                G4cout << "Building cube with cutout and cylindrical and rectangular holes  for X0 measurements" << G4endl;

		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Nickel = nistman->FindOrBuildMaterial("G4_Ni");

		G4double box1Size = 3.0*mm;
		G4Box * box1 = new G4Box("Box1", box1Size, box1Size, box1Size);
	        			 
		//m_TestStructure_log = new G4LogicalVolume(box1, Aluminum, "Box1");


		G4double box2Size = box1Size/2.;
		G4Box * box2 = new G4Box("Box2", box2Size, box2Size, box2Size);

		G4SubtractionSolid *cutBox = new G4SubtractionSolid("cutBox",box1,box2,0,G4ThreeVector(0,0,box1Size));
		//m_TestStructure_log = new G4LogicalVolume(cutBox, Aluminum, "cutBox");

		// Define multiple spheres and subtract them from the unit

		G4RotationMatrix * rotTub = new G4RotationMatrix;
		rotTub->rotateY( 90*deg );

		G4double s1size = 0.5*mm;
		G4double s1trans = 1.5*mm;
		G4Tubs * tub1 = new G4Tubs("Tub1", 0., s1size, box1Size*1.2, 0.*deg, 360.*deg);
		G4SubtractionSolid *cutBoxS1 = new G4SubtractionSolid("cutBoxS1",cutBox,tub1,rotTub,G4ThreeVector(0,s1trans,-s1trans));

		G4double s2size = 0.2*mm;
		G4double s2trans = 1.5*mm;
		G4Tubs * tub2 = new G4Tubs("Tub2", 0., s2size, box1Size*1.2, 0.*deg, 360.*deg);
		G4SubtractionSolid *cutBoxS2 = new G4SubtractionSolid("cutBoxS2",cutBoxS1,tub2,rotTub,G4ThreeVector(0,-s2trans,-s2trans));
		
		G4double s3size = 0.1*mm;
		G4double s3trans = 0.*mm;
		G4Tubs * tub3 = new G4Tubs("Tub3", 0., s3size, box1Size*1.2, 0.*deg, 360.*deg);
		G4SubtractionSolid *cutBoxS3 = new G4SubtractionSolid("cutBoxS3",cutBoxS2,tub3,rotTub,G4ThreeVector(0,s3trans,-s3trans));

		G4double s4size = 0.05*mm;
		G4double s4trans = 1.5*mm;
		G4Tubs * tub4 = new G4Tubs("Tub4", 0., s4size, box1Size*1.2, 0.*deg, 360.*deg);
		G4SubtractionSolid *cutBoxS4 = new G4SubtractionSolid("cutBoxS4",cutBoxS3,tub4,rotTub,G4ThreeVector(0,-s4trans,0));

		G4double box3Size = 0.2*mm;
		G4double box3trans = 1.5*mm;
		G4Box * box3 = new G4Box("Box3", box1Size*1.2, box3Size, box3Size);
		G4SubtractionSolid *cutBoxS5 = new G4SubtractionSolid("cutBoxS5",cutBoxS4,box3,0,G4ThreeVector(0,0,-box3trans));

		G4double box4Size = 0.1*mm;
		G4double box4trans = 1.5*mm;
		G4Box * box4 = new G4Box("Box4", box1Size*1.2, box4Size, box4Size);
		G4SubtractionSolid *cutBoxS6 = new G4SubtractionSolid("cutBoxS6",cutBoxS5,box4,0,G4ThreeVector(0,box4trans,0));


		m_TestStructure_log = new G4LogicalVolume(cutBoxS6, Nickel, "cutBoxS6");
		
		//m_TestStructure_log2 = new G4LogicalVolume(box2, Aluminum, "Box2");

		// Move the box down to align window with DUT based on macro value
		map<int, G4ThreeVector>::iterator testStructItr = m_posVectorTestStructure.begin();

		// FIXME: Assuming that there is only one TestStructure
		G4ThreeVector posRel;
		for( ; testStructItr != m_posVectorTestStructure.end() ; testStructItr++){
		  posRel = (*testStructItr).second;
		}


		G4RotationMatrix * rotMatrix;
		map<int, G4RotationMatrix*>::iterator testStructItr2 = m_rotVectorTestStructure.begin();

		for( ; testStructItr2 != m_rotVectorTestStructure.end() ; testStructItr2++){
		  rotMatrix = (*testStructItr2).second;
		}

		G4cout << "Cube tilted by deg: " << rotMatrix[0].getTheta()/deg << G4endl;

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color::Blue());
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);
		
		m_TestStructure_phys = new G4PVPlacement(rotMatrix,
				posRel,
				m_TestStructure_log,
				"Box1",
				expHall_log,
				false,
				0);
		
		break;
	}
	case 108:
	{
		//Two cubes with spherical holes inside (purpose X0 measurement)
                G4cout << "Building cube with cutout and cylindrical and rectangular holes  for X0 measurements" << G4endl;

		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Lead = nistman->FindOrBuildMaterial("G4_Pb");

		G4double box1Size = 3.0*mm;
		G4Box * box1 = new G4Box("Box1", box1Size, box1Size, box1Size);
	        			 
		//m_TestStructure_log = new G4LogicalVolume(box1, Aluminum, "Box1");


		G4double box2Size = box1Size/2.;
		G4Box * box2 = new G4Box("Box2", box2Size, box2Size, box2Size);

		G4SubtractionSolid *cutBox = new G4SubtractionSolid("cutBox",box1,box2,0,G4ThreeVector(0,0,box1Size));
		//m_TestStructure_log = new G4LogicalVolume(cutBox, Aluminum, "cutBox");

		// Define multiple spheres and subtract them from the unit

		G4RotationMatrix * rotTub = new G4RotationMatrix;
		rotTub->rotateY( 90*deg );

		G4double s1size = 0.5*mm;
		G4double s1trans = 1.5*mm;
		G4Tubs * tub1 = new G4Tubs("Tub1", 0., s1size, box1Size*1.2, 0.*deg, 360.*deg);
		G4SubtractionSolid *cutBoxS1 = new G4SubtractionSolid("cutBoxS1",cutBox,tub1,rotTub,G4ThreeVector(0,s1trans,-s1trans));

		G4double s2size = 0.2*mm;
		G4double s2trans = 1.5*mm;
		G4Tubs * tub2 = new G4Tubs("Tub2", 0., s2size, box1Size*1.2, 0.*deg, 360.*deg);
		G4SubtractionSolid *cutBoxS2 = new G4SubtractionSolid("cutBoxS2",cutBoxS1,tub2,rotTub,G4ThreeVector(0,-s2trans,-s2trans));
		
		G4double s3size = 0.1*mm;
		G4double s3trans = 0.*mm;
		G4Tubs * tub3 = new G4Tubs("Tub3", 0., s3size, box1Size*1.2, 0.*deg, 360.*deg);
		G4SubtractionSolid *cutBoxS3 = new G4SubtractionSolid("cutBoxS3",cutBoxS2,tub3,rotTub,G4ThreeVector(0,s3trans,-s3trans));

		G4double s4size = 0.05*mm;
		G4double s4trans = 1.5*mm;
		G4Tubs * tub4 = new G4Tubs("Tub4", 0., s4size, box1Size*1.2, 0.*deg, 360.*deg);
		G4SubtractionSolid *cutBoxS4 = new G4SubtractionSolid("cutBoxS4",cutBoxS3,tub4,rotTub,G4ThreeVector(0,-s4trans,0));

		G4double box3Size = 0.2*mm;
		G4double box3trans = 1.5*mm;
		G4Box * box3 = new G4Box("Box3", box1Size*1.2, box3Size, box3Size);
		G4SubtractionSolid *cutBoxS5 = new G4SubtractionSolid("cutBoxS5",cutBoxS4,box3,0,G4ThreeVector(0,0,-box3trans));

		G4double box4Size = 0.1*mm;
		G4double box4trans = 1.5*mm;
		G4Box * box4 = new G4Box("Box4", box1Size*1.2, box4Size, box4Size);
		G4SubtractionSolid *cutBoxS6 = new G4SubtractionSolid("cutBoxS6",cutBoxS5,box4,0,G4ThreeVector(0,box4trans,0));


		m_TestStructure_log = new G4LogicalVolume(cutBoxS6, Lead, "cutBoxS6");
		
		//m_TestStructure_log2 = new G4LogicalVolume(box2, Aluminum, "Box2");

		// Move the box down to align window with DUT based on macro value
		map<int, G4ThreeVector>::iterator testStructItr = m_posVectorTestStructure.begin();

		// FIXME: Assuming that there is only one TestStructure
		G4ThreeVector posRel;
		for( ; testStructItr != m_posVectorTestStructure.end() ; testStructItr++){
		  posRel = (*testStructItr).second;
		}


		G4RotationMatrix * rotMatrix;
		map<int, G4RotationMatrix*>::iterator testStructItr2 = m_rotVectorTestStructure.begin();

		for( ; testStructItr2 != m_rotVectorTestStructure.end() ; testStructItr2++){
		  rotMatrix = (*testStructItr2).second;
		}

		G4cout << "Cube tilted by deg: " << rotMatrix[0].getTheta()/deg << G4endl;

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color::Blue());
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);
		
		m_TestStructure_phys = new G4PVPlacement(rotMatrix,
				posRel,
				m_TestStructure_log,
				"Box1",
				expHall_log,
				false,
				0);
		
		break;
	}
	case 110:
	{
                G4cout << "Building cube for X0 measurements" << G4endl;

		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Aluminum = nistman->FindOrBuildMaterial("G4_Al");

		G4double box1Size = 3.0*mm;
		G4Box * box1 = new G4Box("Box1", box1Size, box1Size, box1Size);
	        			 
		m_TestStructure_log = new G4LogicalVolume(box1, Aluminum, "Box1");

		// Move the box down to align window with DUT based on macro value
		map<int, G4ThreeVector>::iterator testStructItr = m_posVectorTestStructure.begin();

		// FIXME: Assuming that there is only one TestStructure
		G4ThreeVector posRel;
		for( ; testStructItr != m_posVectorTestStructure.end() ; testStructItr++){
		  posRel = (*testStructItr).second;
		}


		G4RotationMatrix * rotMatrix;
		map<int, G4RotationMatrix*>::iterator testStructItr2 = m_rotVectorTestStructure.begin();

		for( ; testStructItr2 != m_rotVectorTestStructure.end() ; testStructItr2++){
		  rotMatrix = (*testStructItr2).second;
		}

		G4cout << "Cube tilted by deg: " << rotMatrix[0].getTheta()/deg << G4endl;

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color::Blue());
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);
		
		m_TestStructure_phys = new G4PVPlacement(rotMatrix,
				posRel,
				m_TestStructure_log,
				"Box1",
				expHall_log,
				false,
				0);
		
		break;
	}

	case 111:
	{
                G4cout << "Building cube for X0 measurements" << G4endl;

		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Nickel = nistman->FindOrBuildMaterial("G4_Ni");

		G4double box1Size = 3.0*mm;
		G4Box * box1 = new G4Box("Box1", box1Size, box1Size, box1Size);
	        			 
		m_TestStructure_log = new G4LogicalVolume(box1, Nickel, "Box1");

		// Move the box down to align window with DUT based on macro value
		map<int, G4ThreeVector>::iterator testStructItr = m_posVectorTestStructure.begin();

		// FIXME: Assuming that there is only one TestStructure
		G4ThreeVector posRel;
		for( ; testStructItr != m_posVectorTestStructure.end() ; testStructItr++){
		  posRel = (*testStructItr).second;
		}


		G4RotationMatrix * rotMatrix;
		map<int, G4RotationMatrix*>::iterator testStructItr2 = m_rotVectorTestStructure.begin();

		for( ; testStructItr2 != m_rotVectorTestStructure.end() ; testStructItr2++){
		  rotMatrix = (*testStructItr2).second;
		}

		G4cout << "Cube tilted by deg: " << rotMatrix[0].getTheta()/deg << G4endl;

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color::Blue());
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);
		
		m_TestStructure_phys = new G4PVPlacement(rotMatrix,
				posRel,
				m_TestStructure_log,
				"Box1",
				expHall_log,
				false,
				0);
		
		break;
	}

	case 112:
	{
                G4cout << "Building cube for X0 measurements" << G4endl;

		G4NistManager* nistman = G4NistManager::Instance();
		G4Material * Lead = nistman->FindOrBuildMaterial("G4_Pb");

		G4double box1Size = 3.0*mm;
		G4Box * box1 = new G4Box("Box1", box1Size, box1Size, box1Size);
	        			 
		m_TestStructure_log = new G4LogicalVolume(box1, Lead, "Box1");

		// Move the box down to align window with DUT based on macro value
		map<int, G4ThreeVector>::iterator testStructItr = m_posVectorTestStructure.begin();

		// FIXME: Assuming that there is only one TestStructure
		G4ThreeVector posRel;
		for( ; testStructItr != m_posVectorTestStructure.end() ; testStructItr++){
		  posRel = (*testStructItr).second;
		}


		G4RotationMatrix * rotMatrix;
		map<int, G4RotationMatrix*>::iterator testStructItr2 = m_rotVectorTestStructure.begin();

		for( ; testStructItr2 != m_rotVectorTestStructure.end() ; testStructItr2++){
		  rotMatrix = (*testStructItr2).second;
		}

		G4cout << "Cube tilted by deg: " << rotMatrix[0].getTheta()/deg << G4endl;

		G4VisAttributes * visAtt_bp = new G4VisAttributes(G4Color::Blue());
		visAtt_bp->SetLineWidth(1);
		visAtt_bp->SetForceSolid(true);
		m_TestStructure_log->SetVisAttributes(visAtt_bp);
		
		m_TestStructure_phys = new G4PVPlacement(rotMatrix,
				posRel,
				m_TestStructure_log,
				"Box1",
				expHall_log,
				false,
				0);
		
		break;
	}

	default:
	{
		G4cout << "Unknown TestStructure Type" << G4endl;
		break;
	}
	}

#ifdef _EUTELESCOPE

	//////////////////////////////////////////////////////////
	// Scintillators for EUDET
	// Materials

	if( m_scintPos.empty() ) {
		G4cout << "[ERROR] no scintillators defined.  In the macro use the command"
				<< "       /allpix/eudet/scint1Pos 0.0  0.0  -24.0 mm"
				<< "       Can't recover ... giving up."
				<< G4endl;
		exit(1);
	}else{
		G4cout << "Building scintillators..." << G4endl;
	}

	// first plane at 0. mm
	//	G4double z1 = -24*mm;
	//	G4double z2 = -18*mm;
	// last plane at 490 mm
	//	G4double z3 = 523*mm;
	// G4double z4 = 529*mm;

	G4NistManager * nistman = G4NistManager::Instance();
	// Scintillator
	G4Material * scplastic = nistman->FindOrBuildMaterial("G4_POLYSTYRENE");
	// Scintillators
	G4Box* scintb = new G4Box("scintb",
			11.0*mm,
			5.4*mm,
			3*mm); // scintillators 6mm thick

	G4VisAttributes * scintAtt = new G4VisAttributes(G4Color(1,0,1,1));
	scintAtt->SetLineWidth(1);
	scintAtt->SetForceSolid(true);

	// Place scintillators
	vector<G4ThreeVector>::iterator scintItr = m_scintPos.begin();

	TString labelLog = "";
	TString labelPlacement = "";
	TString labelSD = "";
	Int_t cntr = 1;
	G4SDManager * SDman = G4SDManager::GetSDMpointer();

	for( ; scintItr != m_scintPos.end() ; scintItr++) {
		labelLog = "scint";
		labelLog += cntr;
		labelLog += "_log";

		labelPlacement = "Scint";
		labelPlacement += cntr;

		G4LogicalVolume * scint_log = new G4LogicalVolume(
				scintb,
				scplastic,
				labelLog.Data());
		scint_log->SetVisAttributes(scintAtt);
		G4RotationMatrix* matrix_s = new G4RotationMatrix();
		matrix_s->rotateX(0.*deg);

		new G4PVPlacement( matrix_s,
				(*scintItr),
				scint_log,
				labelPlacement.Data(),
				expHall_log,
				false,
				0,
				true);

		labelSD = "sdscint";
		labelSD += cntr;
		AllPixTrackerSD * scintTrack = new AllPixTrackerSD( labelSD.Data(), (*scintItr), 0);
		SDman->AddNewDetector( scintTrack );
		scint_log->SetSensitiveDetector( scintTrack );
		cntr++;
	}
#endif

}
