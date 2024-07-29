#include "Calibration.hh"

////////////////////////////////////////////////////////////////////////////////
/// Constructs the GreatCalibration object. Initialises private variables, reads
/// the calibration input file, and sets up the root-finding algorithm for the
/// time walk correction.
/// \param[in] filename String containing the directory location of the input 
/// calibration file
/// \param[in] myset Pointer to the GreatSettings object
GreatCalibration::GreatCalibration( std::string filename, std::shared_ptr<GreatSettings> myset ) {

	SetFile( filename );
	set = myset;
	ReadCalibration();
	fRand = new TRandom();

}

////////////////////////////////////////////////////////////////////////////////
/// Reads the calibration input file and sets the values of all of the variables
/// required for calibrating data in Great format. Uses a TEnv environment to read the
/// contents of the file. Called in GreatCalibration::GreatCalibration( std::string filename, GreatSettings *myset )
void GreatCalibration::ReadCalibration() {

	TEnv *config = new TEnv( fInputFile.data() );
	
	// CAEN initialisation
	fCaenOffset.resize( set->GetNumberOfCAENModules() );
	fCaenGain.resize( set->GetNumberOfCAENModules() );
	fCaenGainQuadr.resize( set->GetNumberOfCAENModules() );
	fCaenThreshold.resize( set->GetNumberOfCAENModules() );
	fCaenTime.resize( set->GetNumberOfCAENModules() );
	fCaenType.resize( set->GetNumberOfCAENModules() );

	fCaenOffsetDefault = 0.0;
	fCaenGainDefault = 1.0;
	fCaenGainQuadrDefault = 0.0;

	// CAEN parameter read
	for( unsigned int mod = 0; mod < set->GetNumberOfCAENModules(); mod++ ){

		fCaenOffset[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenGain[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenGainQuadr[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenThreshold[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenTime[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenType[mod].resize( set->GetNumberOfCAENChannels() );

		for( unsigned int chan = 0; chan < set->GetNumberOfCAENChannels(); chan++ ){

			fCaenOffset[mod][chan] = config->GetValue( Form( "caen_%d_%d.Offset", mod, chan ), fCaenOffsetDefault );
			fCaenGain[mod][chan] = config->GetValue( Form( "caen_%d_%d.Gain", mod, chan ), fCaenGainDefault );
			fCaenGainQuadr[mod][chan] = config->GetValue( Form( "caen_%d_%d.GainQuadr", mod, chan ), fCaenGainQuadrDefault );
			fCaenThreshold[mod][chan] = config->GetValue( Form( "caen_%d_%d.Threshold", mod, chan ), 0 );
			fCaenTime[mod][chan] = config->GetValue( Form( "caen_%d_%d.Time", mod,  chan ), 0.0 );
			fCaenType[mod][chan] = config->GetValue( Form( "caen_%d_%d.Type", mod,  chan ), "Qlong" );

		}
		
	}
	
	delete config;
	
}


////////////////////////////////////////////////////////////////////////////////
/// Calculates the energy on a particular CAEN detector. Also adds a small random 
/// number to remove binning issues
/// \param[in] mod The number of the CAEN module
/// \param[in] chan The channel number on the CAEN module
/// \param[in] raw The raw energy recorded on this detector
/// \returns Calibrated energy (if parameters are set), the raw energy (if 
/// parameters are not set), or -1 (if the mod, asic, or channel are out of range)
float GreatCalibration::CaenEnergy( unsigned int mod, unsigned int chan, int raw ) {
	
	float energy, raw_rand;
	
	//std::cout << "mod=" << mod << "; chan=" << chan << std::endl;

	if( mod < set->GetNumberOfCAENModules() &&
	   chan < set->GetNumberOfCAENChannels() ) {

		raw_rand = raw + 0.5 - fRand->Uniform();

		energy = 0;
		energy =  fCaenGainQuadr[mod][chan] * raw_rand * raw_rand;
		energy += fCaenGain[mod][chan] * raw_rand;
		energy += fCaenOffset[mod][chan];

		// Check if we have defaults
		if( TMath::Abs( fCaenGainQuadr[mod][chan] ) < 1e-6 &&
		    TMath::Abs( fCaenGain[mod][chan] - 1.0 ) < 1e-6 &&
		    TMath::Abs( fCaenOffset[mod][chan] ) < 1e-6 )
			
			return raw;
		
		else return energy;
		
	}

	return -1;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Getter for the CAEN threshold
/// \param[in] mod The number of the CAEN module
/// \param[in] chan The channel number of the detector
unsigned int GreatCalibration::CaenThreshold( unsigned int mod, unsigned int chan ) {
	
	if( mod < set->GetNumberOfCAENModules() &&
	   chan < set->GetNumberOfCAENChannels() ) {

		return fCaenThreshold[mod][chan];
		
	}
	
	return -1;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Getter for the CAEN time
/// \param[in] mod The number of the CAEN module
/// \param[in] chan The channel number of the detector
long double GreatCalibration::CaenTime( unsigned int mod, unsigned int chan ){
	
	if( mod < set->GetNumberOfCAENModules() &&
	   chan < set->GetNumberOfCAENChannels() ) {

		return fCaenTime[mod][chan];
		
	}
	
	return 0;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Getter for the CAEN type = the type of 
/// \param[in] mod The number of the CAEN module
/// \param[in] chan The channel number of the detector
std::string GreatCalibration::CaenType( unsigned int mod, unsigned int chan ){
	
	if( mod < set->GetNumberOfCAENModules() &&
	   chan < set->GetNumberOfCAENChannels() ) {

		return fCaenType[mod][chan];
		
	}
	
	return "NULL";
	
}

////////////////////////////////////////////////////////////////////////////////
/// Prints the calibration to a specified output
/// \param[in] stream Determines where the calibration will be printed
/// \param[in] opt Determines which parameters in the calibration are going to be printed:
/// c = caen only
/// a = asic only
/// e = print only energy terms (i.e. not time, whether device is enabled, or type)
void GreatCalibration::PrintCalibration( std::ostream &stream, std::string opt ){
	
	// Check options for energy only
	bool caen_only = false;
	bool energy_only = false;
	if( opt.find("c") != std::string::npos || opt.find("C") != std::string::npos ) caen_only = true;
	if( opt.find("e") != std::string::npos || opt.find("E") != std::string::npos ) energy_only = true;

	
	// CAEN print
	for( unsigned int mod = 0; mod < set->GetNumberOfCAENModules(); mod++ ){
		
		for( unsigned int chan = 0; chan < set->GetNumberOfCAENChannels(); chan++ ){
			
			if( TMath::Abs( fCaenGainQuadr[mod][chan] - fCaenOffsetDefault ) > 1e-6 &&
			   TMath::Abs( fCaenGain[mod][chan] - fCaenGainDefault ) > 1e-6 &&
			   TMath::Abs( fCaenOffset[mod][chan] - fCaenGainQuadrDefault ) > 1e-6 ) {
				
				stream << Form( "caen_%d_%d.Offset: %f", mod, chan, fCaenOffset[mod][chan] ) << std::endl;
				stream << Form( "caen_%d_%d.Gain: %f", mod, chan, fCaenGain[mod][chan] ) << std::endl;
				stream << Form( "caen_%d_%d.GainQuadr: %f", mod, chan, fCaenGainQuadr[mod][chan] ) << std::endl;
				
			}
			
			if( fCaenThreshold[mod][chan] != 0 )
				stream << Form( "caen_%d_%d.Threshold: %u", mod, chan, fCaenThreshold[mod][chan] ) << std::endl;
			
			if( !energy_only && TMath::Abs( fCaenTime[mod][chan] ) > 1e-9 )
				stream << Form( "caen_%d_%d.Time: %Lf", mod, chan, fCaenTime[mod][chan] ) << std::endl;
			
			if( !energy_only && fCaenType[mod][chan] != 0 )
				stream << Form( "caen_%d_%d.Type: %s", mod, chan, fCaenType[mod][chan].data() ) << std::endl;
			
		} // chan
		
	} // mod
	
};
