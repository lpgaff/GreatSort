#ifndef __CALIBRATION_HH
#define __CALIBRATION_HH

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <array>
#include <cstdlib>

#include "TSystem.h"
#include "TEnv.h"
#include "TRandom.h"
#include "TMath.h"
#include "TF1.h"
#include "TGraph.h"
#include "TFile.h"

// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

/*!
* \brief A class to read in the calibration file in ROOT's TConfig format.
*
* \details Each electronics channel can have offset, gain and quadratic terms.
* Each channel also has a threshold and time offset parameter
*/

class GreatCalibration {

public:

	GreatCalibration( std::string filename, std::shared_ptr<GreatSettings> myset );///< Constructor
	/// Destructor
	inline virtual ~GreatCalibration() {
		delete fRand;
	};

	void ReadCalibration();
	void PrintCalibration( std::ostream &stream, std::string opt );

	/// Setter for the location of the input file
	/// \param[in] filename The location of the input file
	void SetFile( std::string filename ){
		fInputFile = filename;
	}

	/// Getter for the calibration input file location
	const std::string InputFile(){
		return fInputFile;
	}

	float CaenEnergy( unsigned int mod, unsigned int chan, int raw );
	unsigned int CaenThreshold( unsigned int mod, unsigned int chan );
	long double CaenTime( unsigned int mod, unsigned int chan );
	std::string CaenType( unsigned int mod, unsigned int chan );

	/// Setter for the CAEN energy calibration parameters
	/// \param[in] mod The module in the CAEN DAQ
	/// \param[in] chan The channel number of the CAEN module
	/// \param[in] offset Constant in CAEN energy calculation
	/// \param[in] gain Linear term in CAEN energy calculation
	/// \param[in] gainquadr Quadratic term in CAEN energy calculation
	inline void SetCaenEnergyCalibration( unsigned int mod, unsigned int chan,
										 float offset, float gain, float gainquadr ){
		if( mod < set->GetNumberOfCAENModules() &&
		   chan < set->GetNumberOfCAENChannels() ) {
			fCaenOffset[mod][chan] = offset;
			fCaenGain[mod][chan] = gain;
			fCaenGainQuadr[mod][chan] = gainquadr;
		}
	};

	/// Setter for the CAEN threshold
	/// \param[in] mod The module in the CAEN DAQ
	/// \param[in] chan The channel number of the CAEN module
	/// \param[in] thres The threshold value
	inline void SetCaenThreshold( unsigned int mod, unsigned int chan,
								 unsigned int thres ){
		if( mod < set->GetNumberOfCAENModules() &&
		   chan < set->GetNumberOfCAENChannels() )
			fCaenThreshold[mod][chan] = thres;
	};

	/// Setter for the CAEN time
	/// \param[in] mod The module in the CAEN DAQ
	/// \param[in] chan The channel number of the CAEN module
	/// \param[in] time The time value
	inline void SetCaenTime( unsigned int mod, unsigned int chan,
							long double time ){
		if( mod < set->GetNumberOfCAENModules() &&
		   chan < set->GetNumberOfCAENChannels() )
			fCaenTime[mod][chan] = time;
	};

	/// Setter for the CAEN type
	/// \param[in] mod The module in the CAEN DAQ
	/// \param[in] chan The channel number of the CAEN module
	/// \param[in] thres The type (default = Qlong)
	inline void SetCaenType( unsigned int mod, unsigned int chan,
							std::string type ){
		if( mod < set->GetNumberOfCAENModules() &&
		   chan < set->GetNumberOfCAENChannels() )
			fCaenType[mod][chan] = type;
	};

private:

	std::string fInputFile;///< The location of the calibration input file
	
	std::shared_ptr<GreatSettings> set;///< Pointer to the GreatSettings object
	
	TRandom *fRand;///< Used to eliminate binning issues

	// Calibration file value storage
	std::vector< std::vector<float> > fCaenOffset;///< Constant in CAEN energy calculation
	std::vector< std::vector<float> > fCaenGain;///< Linear term in CAEN energy calculation
	std::vector< std::vector<float> > fCaenGainQuadr;///< Quadratic term in CAEN energy calibration
	std::vector< std::vector<unsigned int> > fCaenThreshold;///< Threshold for raw signals from detectors in the CAEN DAQ
	std::vector< std::vector<long double> > fCaenTime;///< Time offset for signals on a given detector in the CAEN DAQ
	std::vector< std::vector<std::string> > fCaenType;///< The type assigned to the CAEN signal

	float fCaenOffsetDefault;///< The default constant in CAEN energy calculations
	float fCaenGainDefault;///< The default linear term in CAEN energy calculations
	float fCaenGainQuadrDefault;///< The default quadratic term in CAEN energy calculations


	//ClassDef(GreatCalibration, 1)
   
};

#endif
