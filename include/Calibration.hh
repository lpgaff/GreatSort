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
#include "TRandom3.h"
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

class GreatMWD : public TObject {

public:

	// Constructor/destructor
	inline GreatMWD() {};
	virtual inline ~GreatMWD() {};

	// Main algorithm
	void DoMWD();

	// Set functions
	inline void SetTrace( std::vector<unsigned short> t ){ trace = t; };
	inline void SetRiseTime( unsigned int t ){ rise_time = t; }; // L
	inline void SetDecayTime( float t ){ decay_time = t; }; // torr
	inline void SetFlatTop( unsigned int t ){ flat_top = t; }; // not in James' code
	inline void SetBaseline( unsigned int t ){ baseline_length = t; }; // BLFL (baseline filter length?)
	inline void SetWindow( unsigned int t ){ window = t; }; // M
	inline void SetDelayTime( unsigned int t ){ cfd_delay = t; };
	inline void SetHoldOff( unsigned int t ){ cfd_hold = t; };
	inline void SetShapingTime( unsigned int t ){ cfd_shaping_time = t; };
	inline void SetIntegrationTime( unsigned int t ){ cfd_integration_time = t; };
	inline void SetThreshold( unsigned int t ){ threshold = t; };
	inline void SetFraction( float f ){ fraction = f; };

	// Get functions
	inline unsigned int NumberOfTriggers(){ return energy_list.size(); };
	inline float GetEnergy( unsigned int i ){
		if( i < energy_list.size() ) return energy_list.at(i);
		else return -99.9;
	};
	inline float GetCfdTime( unsigned int i ){
		if( i < cfd_list.size() ) return cfd_list.at(i);
		else return 0;
	};
	inline std::vector<float> GetEnergies(){ return energy_list; };
	inline std::vector<float> GetCfdTimes(){ return cfd_list; };
	inline std::vector<float> GetStage1(){ return stage1; };
	inline std::vector<float> GetStage2(){ return stage2; };
	inline std::vector<float> GetStage3(){ return stage3; };
	inline std::vector<float> GetStage4(){ return stage4; };
	inline std::vector<float> GetCfd(){ return cfd; };

	// Is it clipped?
	inline bool IsClipped(){ return clipped; };

	// Graphs
	inline TGraph* GetTraceGraph() {
		return GetGraph( trace );
	};
	inline TGraph* GetStage1Graph() {
		return GetGraph( stage1 );
	};
	inline TGraph* GetStage2Graph() {
		return GetGraph( stage2 );
	};
	inline TGraph* GetStage3Graph() {
		return GetGraph( stage3 );
	};
	inline TGraph* GetStage4Graph() {
		return GetGraph( stage4 );
	};
	inline TGraph* GetCfdGraph() {
		return GetGraph( cfd );
	};

private:

	// List of energies and time to return
	std::vector<float> energy_list;
	std::vector<float> cfd_list;

	// Vector to hold the trace
	std::vector<unsigned short> trace;

	// Initialise some vectors for holding the MWD and CFD etc.
	std::vector<float> stage1, stage2, stage3, stage4;
	std::vector<float> differential, shaper, cfd;

	// Values of MWD
	unsigned int rise_time, flat_top, window, baseline_length, decay_time;

	// Values for CFD
	unsigned int cfd_delay, cfd_hold, cfd_shaping_time, cfd_integration_time;
	int threshold;
	float fraction;

	// Are any of the samples clipped?
	bool clipped;

	// Graphs
	inline TGraph* GetGraph( std::vector<float> &t ) {
		std::vector<float> x;
		for( unsigned short i = 0; i < t.size(); ++i )
			x.push_back( i );
		std::unique_ptr<TGraph> g = std::make_unique<TGraph>(
															 t.size(), x.data(), t.data() );
		return (TGraph*)g.get()->Clone();
	};
	inline TGraph* GetGraph( std::vector<unsigned short> &t ) {
		std::vector<float> y;
		for( unsigned short i = 0; i < t.size(); ++i )
			y.push_back( static_cast<float>(t[i]) );
		return GetGraph(y);
	};

	ClassDef( GreatMWD, 1 );

};


class GreatCalibration {

public:

	GreatCalibration( std::string filename, std::shared_ptr<GreatSettings> myset );///< Constructor
	/// Destructor
	inline virtual ~GreatCalibration() {};

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
	GreatMWD DoMWD( unsigned char mod, unsigned char ch, std::vector<unsigned short> trace );

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

	// Set functions for MWD
	void SetMWDDecay( unsigned char mod, unsigned char ch, unsigned int decay );
	void SetMWDRise( unsigned char mod, unsigned char ch, unsigned int rise );
	void SetMWDFlatTop( unsigned char mod, unsigned char ch, unsigned int top );
	void SetMWDBaseline( unsigned char mod, unsigned char ch, unsigned int baseline_length );
	void SetMWDWindow( unsigned char mod, unsigned char ch, unsigned int window );
	void SetCFDFraction( unsigned char mod, unsigned char ch, float fraction );
	void SetCFDDelay( unsigned char mod, unsigned char ch, unsigned int delay );
	void SetCFDHoldOff( unsigned char mod, unsigned char ch, unsigned int hold );
	void SetCFDShapingTime( unsigned char mod, unsigned char ch, unsigned int shaping );
	void SetCFDIntegrationTime( unsigned char mod, unsigned char ch, unsigned int integration );
	void SetCFDThreshold( unsigned char mod, unsigned char ch, int threshold );

	// Get functions for MWD
	unsigned int GetMWDDecay( unsigned char mod, unsigned char ch );
	unsigned int GetMWDRise( unsigned char mod, unsigned char ch );
	unsigned int GetMWDFlatTop( unsigned char mod, unsigned char ch );
	unsigned int GetMWDBaseline( unsigned char mod, unsigned char ch );
	unsigned int GetMWDWindow( unsigned char mod, unsigned char ch );
	float GetCFDFraction( unsigned char mod, unsigned char ch );
	unsigned int GetCFDDelay( unsigned char mod, unsigned char ch );
	unsigned int GetCFDHoldOff( unsigned char mod, unsigned char ch );
	unsigned int GetCFDShapingTime( unsigned char mod, unsigned char ch );
	unsigned int GetCFDIntegrationTime( unsigned char mod, unsigned char ch );
	int GetCFDThreshold( unsigned char mod, unsigned char ch );


private:

	std::string fInputFile;///< The location of the calibration input file
	
	std::shared_ptr<GreatSettings> set;///< Pointer to the GreatSettings object
	
	std::unique_ptr<TRandom3> fRand;///< Used to eliminate binning issues

	// Calibration file value storage
	std::vector< std::vector<float> > fCaenOffset;///< Constant in CAEN energy calculation
	std::vector< std::vector<float> > fCaenGain;///< Linear term in CAEN energy calculation
	std::vector< std::vector<float> > fCaenGainQuadr;///< Quadratic term in CAEN energy calibration
	std::vector< std::vector<unsigned int> > fCaenThreshold;///< Threshold for raw signals from detectors in the CAEN DAQ
	std::vector< std::vector<long double> > fCaenTime;///< Time offset for signals on a given detector in the CAEN DAQ
	std::vector< std::vector<std::string> > fCaenType;///< The type assigned to the CAEN signal

	// MWD + CFD
	std::vector<std::vector<unsigned int>> fGreatMWD_Decay;
	std::vector<std::vector<unsigned int>> fGreatMWD_Rise;
	std::vector<std::vector<unsigned int>> fGreatMWD_Top;
	std::vector<std::vector<unsigned int>> fGreatMWD_Baseline;
	std::vector<std::vector<unsigned int>> fGreatMWD_Window;
	std::vector<std::vector<unsigned int>> fGreatCFD_Delay;
	std::vector<std::vector<unsigned int>> fGreatCFD_HoldOff;
	std::vector<std::vector<unsigned int>> fGreatCFD_Shaping;
	std::vector<std::vector<unsigned int>> fGreatCFD_Integration;
	std::vector<std::vector<int>> fGreatCFD_Threshold; // polarity of CFD selected by a negative threshold
	std::vector<std::vector<float>> fGreatCFD_Fraction;

	// Calibration defaults
	float fCaenOffsetDefault;///< The default constant in CAEN energy calculations
	float fCaenGainDefault;///< The default linear term in CAEN energy calculations
	float fCaenGainQuadrDefault;///< The default quadratic term in CAEN energy calculations
	unsigned int fCaenThresholdDefault;
	long double fCaenTimeDefault;
	std::string fCaenTypeDefault;

	// MWD defaults
	unsigned int default_GreatMWD_Decay;
	unsigned int default_GreatMWD_Rise;
	unsigned int default_GreatMWD_Top;
	unsigned int default_GreatMWD_Baseline;
	unsigned int default_GreatMWD_Window;
	float default_GreatCFD_Fraction;
	unsigned int default_GreatCFD_Delay;
	unsigned int default_GreatCFD_HoldOff;
	unsigned int default_GreatCFD_Shaping;
	unsigned int default_GreatCFD_Integration;
	int default_GreatCFD_Threshold;



	//ClassDef(GreatCalibration, 1)
   
};

#endif
