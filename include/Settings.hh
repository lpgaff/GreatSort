#ifndef __SETTINGS_HH
#define __SETTINGS_HH

#include <iostream>
#include <fstream>
#include <string>

#include "TSystem.h"
#include "TEnv.h"

/*! \brief Class to implement user "settings" to the MIDAS GREAT formwat data sort code
*
* A class to read in the settings file in ROOT's TConfig format. The GreatSettings class contains all the information about detector layout, data settings, and defines which detector is which. These can be changed directly in the settings file that is fed into GreatSort using the "-s" flag.
*
*/ 

class GreatSettings {

public:

	GreatSettings( std::string filename );
	inline virtual ~GreatSettings() {};
	
	void ReadSettings();
	void PrintSettings();
	void SetFile( std::string filename ){
		fInputFile = filename;
	}
	const std::string InputFile(){
		return fInputFile;
	}
	
	
	// CAEN settings
	inline unsigned char GetNumberOfCAENModules(){ return n_caen_mod; };
	inline unsigned char GetNumberOfCAENChannels(){ return n_caen_ch; };
	inline unsigned int GetCAENModel( unsigned char i ){
		if( i < n_caen_mod )
			return caen_model[i];
		else return 1725;
	};
	inline std::string GetCAENFirmware( unsigned char i ){
		if( i < n_caen_mod )
			return caen_fw[i];
		else return "";
	};
	
	
	// Info settings
	inline unsigned char GetSyncCode(){ return sync_code; };
	inline unsigned char GetTimestampCode(){ return thsb_code; };


	// Event builder
	inline double GetEventWindow(){ return event_window; };
	inline double GetCeBr3HitWindow(){ return cebr3_hit_window; }
	inline double GetHPGeHitWindow(){ return hpge_hit_window; }

	
	// Data settings
	inline unsigned int GetBlockSize(){ return block_size; };
	inline bool IsCAENOnly(){ return flag_caen_only; };


	// TACs
	inline unsigned short GetNumberOfTACs(){ return n_tacs; };
	short GetTACID( unsigned char mod, unsigned char ch );
	bool IsTAC( unsigned char mod, unsigned char ch );

	// CeBr3 detectors
	inline unsigned short GetNumberOfCeBr3Detectors(){ return n_cebr3_detector; };
	short GetCeBr3Detector( unsigned char mod, unsigned char ch );
	bool IsCeBr3( unsigned char mod, unsigned char ch );

	// HPGe detectors
	inline unsigned short GetNumberOfHPGeDetectors(){ return n_hpge_detector; };
	short GetHPGeDetector( unsigned char mod, unsigned char ch );
	short GetHPGeSegment( unsigned char mod, unsigned char ch );
	bool IsHPGe( unsigned char mod, unsigned char ch );


private:

	std::string fInputFile;

	// CAEN settings
	unsigned char n_caen_mod;
	unsigned char n_caen_ch;
	std::vector<unsigned int> caen_model;
	std::vector<std::string> caen_fw;
	
	// Info code settings
	unsigned char sync_code;			///< Medium significant bits of the timestamp are here
	unsigned char thsb_code;			///< Highest significant bits of the timestamp are here

	
	// Event builder
	double event_window;			///< Event builder time window in ns
	double cebr3_hit_window;		///< Time window in ns for correlating CeBr3 (addback?)
	double hpge_hit_window;			///< Time window in ns for correlating HPGe hits (addback?)

	
	// Data format
	unsigned int block_size;		///< not yet implemented, needs C++ style reading of data files
	bool flag_caen_only;			///< when there is only CAEN data in the file

	
	// TACs
	unsigned short n_tacs;						///< Number of TAC modules
	std::vector<unsigned char> tac_mod;			///< Module number of each TAC input
	std::vector<unsigned char> tac_ch;			///< Channel number of each TAC input
	std::vector<std::vector<short>> tac_id;		///< A channel map for the TAC modules

	
	// CeBr3 detectors
	unsigned short n_cebr3_detector;				///< Number of CeBr gamma-ray detectors
	std::vector<unsigned char> cebr3_mod;		///< A list of module numbers for each CeBr gamma-ray  detectors
	std::vector<unsigned char> cebr3_ch;		///< A list of channel numbers for each CeBr gamma-ray  detectors
	std::vector<std::vector<short>> cebr3_id;	///< A channel map for the CeBr gamma-ray  detectors (-1 if not an CeBr gamma-ray detector)


	// HPGe detectors
	unsigned short n_hpge_detector;						///< Number of HPGe gamma-ray detectors
	unsigned short n_hpge_segments;						///< Number of HPGe gamma-ray segments
	std::vector<std::vector<unsigned char>> hpge_mod;	///< A list of module numbers for each HPGe gamma-ray detector and segment
	std::vector<std::vector<unsigned char>> hpge_ch;	///< A list of channel numbers for each HPGe gamma-ray detector and segment
	std::vector<std::vector<short>> hpge_id;				///< A channel map for the HPGe gamma-ray detectors (-1 if not an HPGe gamma-ray detector)
	std::vector<std::vector<short>> hpge_seg;			///< A channel map for the HPGe gamma-ray segment (-1 if not an HPGe gamma-ray detector)


};

#endif
