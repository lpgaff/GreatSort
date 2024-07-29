#ifndef __REACTION_HH
#define __REACTION_HH

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <memory>

#include "TSystem.h"
#include "TEnv.h"
#include "TMath.h"
#include "TObject.h"
#include "TString.h"
#include "TFile.h"
#include "TCutG.h"
#include "TVector3.h"
#include "TAxis.h"
#include "TF1.h"
#include "TFormula.h"
#include "TError.h"
#include "TCanvas.h"
#include "TGraph.h"


// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// Make sure that the data and srim file are defined
#ifndef AME_FILE
# define AME_FILE "./data/mass_1.mas20"
#endif

const double p_mass  = 938272.08816;	///< mass of the proton in keV/c^2
const double n_mass  = 939565.42052;	///< mass of the neutron in keV/c^2
const double u_mass  = 931494.10242;	///< atomic mass unit in keV/c^2
const double T_to_mm =   299.792458;	///< in units of 1/mm
const double k_Si 	 =     2.88e-10;	///< k value - mm/e-h pair for PHD in silicon 
const double e0_Si 	 =     3.67e-03;	///< epsilon_0 for silicon for PHD in keV

// Element names
const std::vector<std::string> gElName = {
	"n","H","He","Li","Be","B","C","N","O","F","Ne","Na","Mg",
	"Al","Si","P","S","Cl","Ar","K","Ca","Sc","Ti","V","Cr",
	"Mn","Fe","Co","Ni","Cu","Zn","Ga","Ge","As","Se","Br","Kr",
	"Rb","Sr","Y","Zr","Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd",
	"In","Sn","Sb","Te","I","Xe","Cs","Ba","La","Ce","Pr","Nd",
	"Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb","Lu","Hf",
	"Ta","W","Re","Os","Ir","Pt","Au","Hg","Tl","Pb","Bi","Po",
	"At","Rn","Fr","Ra","Ac","Th","Pa","U","Np","Pu","Am","Cm",
	"Bk","Cf","Es","Fm","Md","No","Lr","Rf","Db","Sg","Bh","Hs",
	"Mt","Ds","Rg","Cn","Nh","Fl","Ms","Lv","Ts","Og","Uue","Ubn"
};///< Symbols for each element in the nuclear chart


///////////////////////////////////////////////////////////////////////////////
/*!
* \brief Stores information about individual particles in a given reaction
*
* Storage class primarily for information particular to a given particle. A 
* number of these come together to give information as part of the GreatReaction 
* class.
*
*/
class GreatParticle : public TObject {
	
public:
	
	// setup functions
	GreatParticle() {};///< Constructor
	~GreatParticle() {};///< Destructor

	// Getters
	inline int		GetA(){ return A; };///< Getter for A
	inline int		GetZ(){ return Z; };///< Getter for Z
	inline int		GetN(){ return A-Z; };///< Calculates N = A - Z
	inline double	GetBindingEnergy(){ return bindingE; };///< Getter for bindingE
	inline double	GetEnergyLab(){ return Elab; };///< Getter for the Elab; Ek = (γ-1)m0
	inline double	GetEnergyTotCM(){ return Ecm_tot; };///< Getter for the total energy in the CM frame
	inline double	GetThetaCM(){ return ThetaCM; };///< Getter for ThetaCM
	inline double	GetThetaLab(){ return ThetaLab; };///< Getter for ThetaLab
	inline double	GetEx(){ return Ex; };///< Getter for Ex
	
	// Setters
	inline void		SetA( int myA ){ A = myA; };///< Setter for A
	inline void		SetZ( int myZ ){ Z = myZ; };///< Setter for Z
	inline void		SetBindingEnergy( double myBE ){ bindingE = myBE; };///< Setter for bindingE
	inline void		SetEnergyLab( double myElab ){ Elab = myElab; };///< Setter for Elab
	inline void		SetEnergyTotCM( double myEcm ){ Ecm_tot = myEcm; };///< Setter for Ecm_tot
	inline void		SetThetaCM( double mytheta ){ ThetaCM = mytheta; };///< Setter for ThetaCM
	inline void		SetThetaLab( double mytheta ){ ThetaLab = mytheta; };///< Setter for ThetaLab
	inline void		SetEx( double myEx ){ Ex = myEx; };///< Setter for Ex
	
	// Calculate/produce properties of the particle
	inline double		GetMass_u(){
		return GetMass() / u_mass;
	};///< Returns mass in u

	inline double		GetMass(){
		double mass = (double)GetN() * n_mass;
		mass += (double)GetZ() * p_mass;
		mass -= (double)GetA() * bindingE;
		return mass;
	};///< Returns mass in keV/c^2

	inline std::string	GetIsotope(){
		return std::to_string( GetA() ) + gElName.at( GetZ() );
	};///< Produces the isotope symbol including mass number e.g. 208Pb

	inline double		GetEnergyTotLab(){
		return GetMass() + GetEnergyLab();
	};///< Calculates the total energy in the lab frame: Etot = Ek + m0 = γm0

	
	inline double		GetMomentumLab(){
		return TMath::Sqrt( TMath::Power( GetEnergyTotLab(), 2.0 ) - TMath::Power( GetMass(), 2.0 ) );
	};///< Calculates the total momentum in the Lab frame
	
	inline double		GetMomentumCM(){
		return TMath::Sqrt( TMath::Power( GetEnergyTotCM(), 2.0 ) - TMath::Power( GetMass(), 2.0 ) );
	};///< Calculates the total momentum in the CM frame
	
	inline double		GetGamma(){ 
		return GetEnergyTotLab() / GetMass();
	};///< Calculates the gamma factor: Etot = γm0
	
	inline double GetBeta(){
		return TMath::Sqrt( 1.0 - 1.0 / TMath::Power( GetGamma(), 2.0 ) );
	};///< Calculates the beta factor


private:
	
	// Properties of reaction particles
	int		A;			///< Mass number, A, of the particle
	int		Z; 			///< Proton number, Z, of the particle
	double	bindingE;	///< Binding energy per nucleon in keV/c^2 (NOT keV/u!!)
	double	Ecm_tot;	///< Total  energy in the centre of mass frame
	double	Elab;		///< Energy in the laboratory system
	double	ThetaCM;	///< Theta in the centre of mass frame in radians
	double	ThetaLab;	///< Theta in the laboratory system in radians
	double	Ex=0;		///< Excitation energy in keV

	ClassDef( GreatParticle, 1 )
	
};


///////////////////////////////////////////////////////////////////////////////
/*!
* \brief Reads in the reaction file in ROOT's TConfig format. And also to do the physics stuff for the reaction.
*
* Holds all the physics information about a given reaction. Calculates relevant 
* kinematic quantities and is accessed for plotting histograms.
*
*/
class GreatReaction {
	
public:
	
	// setup functions
	GreatReaction( std::string filename, std::shared_ptr<GreatSettings> myset, bool source );///< Constructor
	//GreatReaction( GreatReaction &t ); ///< TODO: Copy constructor
	~GreatReaction(){};///< Destructor
	
	// Main functions
	void AddBindingEnergy( short Ai, short Zi, TString ame_be_str );///< Add a binding energy to the ame_be mass-table map
	void ReadMassTables();///< Reads the AME2020 mass tables
	void ReadReaction();///< Reads the reaction input file
	
	void SetFile( std::string filename ){
		fInputFile = filename;
	}///< Setter for the reaction file location
	
	const std::string InputFile(){
		return fInputFile;
	}///< Getter for the reaction file location
	

	// Some kinematics
	inline double GetQvalue(){
		return Beam.GetMass() + Target.GetMass() -
			Ejectile.GetMass() - Recoil.GetMass();
	};///< Calculates the Q value for the reaction
	
	inline double GetEnergyTotLab(){
		return Beam.GetEnergyTotLab() + Target.GetEnergyTotLab();
	};///< Calculates the total energy in the lab frame
	
	inline double GetEnergyTotCM(){
		double etot = TMath::Power( Beam.GetMass(), 2.0 );
		etot += TMath::Power( Target.GetMass(), 2.0 );
		etot += 2.0 * Beam.GetEnergyTotLab() * Target.GetMass();
		etot = TMath::Sqrt( etot );
		return etot;
	};///< Calculates the total energy in the CM frame
	
	inline double		GetGamma(){
		return GetEnergyTotLab() / GetEnergyTotCM();
	};///< Calculates the gamma factor for going between lab and CM frames
	
	inline double GetBeta(){
		return TMath::Sqrt( 1.0 - 1.0 / TMath::Power( GetGamma(), 2.0 ) );
	};///< Calculates the beta factor for going between lab and CM frames
	

	// It's a source only measurement
	inline void SourceOnly(){ flag_source = true; };///< Flags the measurement as source only
	inline bool IsSource(){ return flag_source; };
	
	// Get filename and other copy stuff
	inline std::string GetFileName(){ return fInputFile; };
	inline std::shared_ptr<GreatSettings> GetSettings(){ return set; };
	inline std::map< std::string, double > GetMassTables(){ return ame_be; };

	// Copiers for the particles
	inline GreatParticle CopyBeam(){ return Beam; };
	inline GreatParticle CopyTarget(){ return Target; };
	inline GreatParticle CopyEjectile(){ return Ejectile; };
	inline GreatParticle CopyRecoil(){ return Recoil; };

	// Getters for the particles
	inline GreatParticle* GetBeam(){ return &Beam; };
	inline GreatParticle* GetTarget(){ return &Target; };
	inline GreatParticle* GetEjectile(){ return &Ejectile; };
	inline GreatParticle* GetRecoil(){ return &Recoil; };

private:

	std::string fInputFile;///< The directory location of the input reaction file
	
	// Settings file
	std::shared_ptr<GreatSettings> set;///< Smart pointer to the GreatSettings object
	
	// Mass tables
	std::map< std::string, double > ame_be;///< List of binding energies from AME2020

	// Reaction partners
	GreatParticle Beam;			///< Beam particle
	GreatParticle Target;		///< Target particle
	GreatParticle Ejectile;		///< Ejectile particle
	GreatParticle Recoil;		///< Recoil particle
	
	// Initial properties from file
	double Eb;			///< Laboratory beam energy in keV/u
	
	// Flag in case it's an alpha source
	bool flag_source;	///< Flag in case it's an alpha source run

};

#endif
