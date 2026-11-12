#include "Calibration.hh"

// Once the parameters have been set, this will do the CFD+MWD algorithm
void GreatMWD::DoMWD() {

	// For now use James' convention from FEBEX
	// Can be optimised by Ethan during the project
	unsigned int L = rise_time;
	unsigned int M = window;
	unsigned int torr = decay_time;
	//unsigned int cfd_trig_delay = M + flat_top;

	// Get the trace length
	unsigned int trace_length = trace.size();

	// Baseline energy estimation
	float baseline_energy = 0.0;

	// resize vectors
	stage1.resize( trace_length, 0.0 );
	stage2.resize( trace_length, 0.0 );
	stage3.resize( trace_length, 0.0 );
	stage4.resize( trace_length, 0.0 );
	differential.resize( trace_length, 0.0 );
	shaper.resize( trace_length, 0.0 );
	cfd.resize( trace_length, 0.0 );

	// Initialise the clipped flag to false
	clipped = false;

	// skip first few samples?
	unsigned int skip = 8;

	// Loop over trace and analyse
	for( unsigned int i = 0; i < trace_length; ++i ) {

		// Check if we are clipped
		if( trace[i] == 0 || (trace[i] & 0x0000FFFF) == 0x0000FFFF )
			clipped = true;

		// Make some default values for derived pulses
		differential[i] = 0;
		shaper[i] = 0;
		cfd[i] = 0;

		// James' simple CFD currently on firmware
		shaper[i] = trace[i];
		fraction = 1.0;

		// Shaped pulse
		if( i >= cfd_shaping_time + skip && i >= cfd_integration_time + skip ) {

			// James - differential-integrating shaper
			differential[i] = trace[i] - trace[i-cfd_shaping_time];
			for( unsigned int j = 1; j <= cfd_integration_time; ++j )
				shaper[i] += differential[i-j];
			shaper[i] /= cfd_integration_time;


			// Liam - simple differential shaper
			//shaper[i] = trace[i] - trace[i-cfd_shaping_time];

		}

		// CFD trace, do triggering later
		if( i >= cfd_delay + skip ) {

			// James + Liam both the same here
			cfd[i]  = fraction * shaper[i];
			cfd[i] -= shaper[i-cfd_delay];

		}

		// Now we need to be longer than M
		if( i >= M + skip ) {

			// MWD stage 1 - difference
			// this is 'D' in James' MATLAB code
			stage1[i]  = (int)trace[i];
			stage1[i] -= (int)trace[i-M];


			// MWD stage 2 - remove decay and average
			// this is 'MA' in James' MATLAB code
			stage2[i] = 0;
			for( unsigned int j = 1; j <= M; ++j )
				stage2[i] += (int)trace[i-j];
			stage2[i] /= torr;

			// MWD stage 3 - moving average
			// this is 'MWD' in James' MATLAB code
			stage3[i] = stage1[i] + stage2[i];

		}

		// MWD stage 4 - energy averaging
		// This is 'T' in James' MWD code
		if( i >= L + skip ){

			for( unsigned int j = 1; j <= L; ++j )
				stage4[i] += stage3[i-j];

			stage4[i] /= L;

		}

	} // loop over trace


	// Loop now over the CFD trace until we trigger
	// This is not the same as James' trigger, but it's better
	for( unsigned int i = skip; i < trace_length; ++i ) {

		// Trigger when we pass the threshold on the CFD
		if( i > cfd_delay + skip &&
		   ( ( cfd[i] > threshold && threshold > 0 ) ||
			( cfd[i] < threshold && threshold < 0 ) ) ) {

			// Mark the arming threshold point
			unsigned int armed_at = i;

			// Find zero crossing - Liam version, but James effects the same thing
			bool xing = false;
			while( !xing && i < trace_length ) {

				// Move to the next sample
				i++;

				// Reject incorrect polarity - Liam version, but James effects the same thing
				if( threshold < 0 && cfd[i-1] > 0 ) continue;
				if( threshold > 0 && cfd[i-1] < 0 ) continue;

				// Found a zero-crossing
				if( cfd[i] * cfd[i-1] < 0 ) xing = true;

			}

			// If we didn't find a crossing, stop now
			if( !xing ) continue;


			// Check we have enough trace left to analyse
			if( trace_length - i < flat_top )
				break;

			// Mark the CFD time - James
			cfd_list.push_back( i );

			// Mark the CFD time - Liam
			//float cfd_time = (float)i / TMath::Abs(cfd[i]);
			//cfd_time += (float)(i-1) / TMath::Abs(cfd[i-1]);
			//cfd_time /= 1.0 / TMath::Abs(cfd[i]) + 1.0 / TMath::Abs(cfd[i-1]);
			//cfd_list.push_back( cfd_time );

			// Baseline estimation comes from averaged trace
			// Just in case a trigger comes before the baseline length, use whatever we can
			if( cfd_list.size() == 1 ) {

				// First trigger with good baseline
				if( i >= baseline_length )
					baseline_energy = stage4[i-baseline_length];

				// Or just use the first sample
				else baseline_energy = stage4[0];
			}

			else {

				// Not the first trigger but still with good baseline
				if( i >= baseline_length + cfd_list.back() )
					baseline_energy = stage4[i-baseline_length];

				// Otherwise don't bother updating baseline, assume its the same

			}

			// move to peak of the flat top
			i += flat_top;

			// assess the energy from stage 4 and push back
			energy_list.push_back( stage4[i] - baseline_energy );
			//energy_list.push_back( stage4[i] );

			// Move to the end of the whole thing
			//i += M + L - flat_top;

			// Check we are beyond the trigger hold off
			if( i < armed_at + cfd_hold )
				i = armed_at + cfd_hold;

		} // threshold passed

	} // loop over CFD

	return;

}

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
	fRand = std::make_unique<TRandom3>();

}

////////////////////////////////////////////////////////////////////////////////
/// Reads the calibration input file and sets the values of all of the variables
/// required for calibrating data in Great format. Uses a TEnv environment to read the
/// contents of the file. Called in GreatCalibration::GreatCalibration( std::string filename, GreatSettings *myset )
void GreatCalibration::ReadCalibration() {

	TEnv *config = new TEnv( fInputFile.data() );
	
	// Default calibration parameters
	fCaenOffsetDefault = 0.0;
	fCaenGainDefault = 1.0;
	fCaenGainQuadrDefault = 0.0;
	fCaenThresholdDefault = 0;
	fCaenTimeDefault = 0;
	fCaenTypeDefault = "Qlong";

	// Default MWD parameters
	default_GreatMWD_Decay			= 5000;
	default_GreatMWD_Rise			= 780; // L
	default_GreatMWD_Top			= 870; // mwd_cfd_trig_delay
	default_GreatMWD_Baseline		= 60;  // delay MWD in James' firmware
	default_GreatMWD_Window			= 880; // M
	default_GreatCFD_Delay			= 30;
	default_GreatCFD_HoldOff		= 100; // prevent double triggering?
	default_GreatCFD_Shaping		= 15;
	default_GreatCFD_Integration	= 10;
	default_GreatCFD_Threshold		= 500;
	default_GreatCFD_Fraction		= 0.3;

	// CAEN initialisation
	fCaenOffset.resize( set->GetNumberOfCAENModules() );
	fCaenGain.resize( set->GetNumberOfCAENModules() );
	fCaenGainQuadr.resize( set->GetNumberOfCAENModules() );
	fCaenThreshold.resize( set->GetNumberOfCAENModules() );
	fCaenTime.resize( set->GetNumberOfCAENModules() );
	fCaenType.resize( set->GetNumberOfCAENModules() );
	fGreatMWD_Decay.resize( set->GetNumberOfCAENModules() );
	fGreatMWD_Rise.resize( set->GetNumberOfCAENModules() );
	fGreatMWD_Top.resize( set->GetNumberOfCAENModules() );
	fGreatMWD_Baseline.resize( set->GetNumberOfCAENModules() );
	fGreatMWD_Window.resize( set->GetNumberOfCAENModules() );
	fGreatCFD_Delay.resize( set->GetNumberOfCAENModules() );
	fGreatCFD_HoldOff.resize( set->GetNumberOfCAENModules() );
	fGreatCFD_Shaping.resize( set->GetNumberOfCAENModules() );
	fGreatCFD_Integration.resize( set->GetNumberOfCAENModules() );
	fGreatCFD_Threshold.resize( set->GetNumberOfCAENModules() );
	fGreatCFD_Fraction.resize( set->GetNumberOfCAENModules() );

	// Global parameters
	fCaenOffsetDefault = (double)config->GetValue( "caen.Offset", (double)fCaenOffsetDefault );
	fCaenGainDefault = (double)config->GetValue( "caen.Gain", fCaenGainDefault );
	fCaenGainQuadrDefault = (double)config->GetValue( "caen.GainQuadr", (double)fCaenGainQuadrDefault );
	fCaenThresholdDefault = (unsigned int)config->GetValue( "febex.Threshold", (double)fCaenThresholdDefault );
	fCaenTimeDefault = (long)config->GetValue( "caen.Time", (double)fCaenTimeDefault );
	fCaenTypeDefault = config->GetValue( "caen.Type", TString(fCaenTypeDefault) );
	default_GreatMWD_Decay = (unsigned int)config->GetValue( "caen.MWD.DecayTime", (double)default_GreatMWD_Decay );
	default_GreatMWD_Rise = (unsigned int)config->GetValue( "caen.MWD.RiseTime", (double)default_GreatMWD_Rise );
	default_GreatMWD_Top = (unsigned int)config->GetValue( "caen.MWD.FlatTop", (double)default_GreatMWD_Top );
	default_GreatMWD_Baseline = (unsigned int)config->GetValue( "caen.MWD.Baseline", (double)default_GreatMWD_Baseline );
	default_GreatMWD_Window = (unsigned int)config->GetValue( "caen.MWD.Window", (double)default_GreatMWD_Window );
	default_GreatCFD_Delay = (unsigned int)config->GetValue( "caen.CFD.DelayTime", (double)default_GreatCFD_Delay );
	default_GreatCFD_HoldOff = (unsigned int)config->GetValue( "caen.CFD.HoldOff", (double)default_GreatCFD_HoldOff );
	default_GreatCFD_Shaping = (unsigned int)config->GetValue( "caen.CFD.ShapingTime", (double)default_GreatCFD_Shaping );
	default_GreatCFD_Integration = (unsigned int)config->GetValue( "caen.CFD.IntegrationTime", (double)default_GreatCFD_Integration );
	default_GreatCFD_Threshold = (int)config->GetValue( "caen.CFD.Threshold", (double)default_GreatCFD_Threshold );
	default_GreatCFD_Fraction = (float)config->GetValue( "caen.CFD.Fraction", (double)default_GreatCFD_Fraction );


	// CAEN parameter read
	for( unsigned int mod = 0; mod < set->GetNumberOfCAENModules(); mod++ ){

		fCaenOffset[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenGain[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenGainQuadr[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenThreshold[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenTime[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenType[mod].resize( set->GetNumberOfCAENChannels() );
		fGreatMWD_Decay[mod].resize( set->GetNumberOfCAENChannels() );
		fGreatMWD_Rise[mod].resize( set->GetNumberOfCAENChannels() );
		fGreatMWD_Top[mod].resize( set->GetNumberOfCAENChannels() );
		fGreatMWD_Baseline[mod].resize( set->GetNumberOfCAENChannels() );
		fGreatMWD_Window[mod].resize( set->GetNumberOfCAENChannels() );
		fGreatCFD_Delay[mod].resize( set->GetNumberOfCAENChannels() );
		fGreatCFD_HoldOff[mod].resize( set->GetNumberOfCAENChannels() );
		fGreatCFD_Shaping[mod].resize( set->GetNumberOfCAENChannels() );
		fGreatCFD_Integration[mod].resize( set->GetNumberOfCAENChannels() );
		fGreatCFD_Threshold[mod].resize( set->GetNumberOfCAENChannels() );
		fGreatCFD_Fraction[mod].resize( set->GetNumberOfCAENChannels() );

		float fCaenOffsetModule = config->GetValue( Form( "caen_%d.Offset", mod ), (double)fCaenOffsetDefault );
		float fCaenGainModule = config->GetValue( Form( "caen_%d.Gain", mod ), (double)fCaenGainDefault );
		float fCaenGainQuadrModule = config->GetValue( Form( "caen_%d.GainQuadr", mod ), (double)fCaenGainQuadrDefault );
		unsigned int fCaenThresholdModule = config->GetValue( Form( "caen_%d.Threshold", mod ), (int)fCaenThresholdDefault );
		long double fCaenTimeModule = config->GetValue( Form( "caen_%d.Time", mod ), (double)fCaenTimeDefault );
		std::string fCaenTypeModule = config->GetValue( Form( "caen_%d.Type", mod ), TString(fCaenTypeDefault) );
		unsigned int fGreatMWD_DecayModule = (unsigned int)config->GetValue( Form( "caen_%d.MWD.DecayTime", mod ), (double)default_GreatMWD_Decay );
		unsigned int fGreatMWD_RiseModule = (unsigned int)config->GetValue( Form( "caen_%d.MWD.RiseTime", mod ), (double)default_GreatMWD_Rise );
		unsigned int fGreatMWD_TopModule = (unsigned int)config->GetValue( Form( "caen_%d.MWD.FlatTop", mod ), (double)default_GreatMWD_Top );
		unsigned int fGreatMWD_BaselineModule = (unsigned int)config->GetValue( Form( "caen_%d.MWD.Baseline", mod ), (double)default_GreatMWD_Baseline );
		unsigned int fGreatMWD_WindowModule = (unsigned int)config->GetValue( Form( "caen_%d.MWD.Window", mod ), (double)default_GreatMWD_Window );
		unsigned int fGreatCFD_DelayModule = (unsigned int)config->GetValue( Form( "caen_%d.CFD.DelayTime", mod ), (double)default_GreatCFD_Delay );
		unsigned int fGreatCFD_HoldOffModule = (unsigned int)config->GetValue( Form( "caen_%d.CFD.HoldOff", mod ), (double)default_GreatCFD_HoldOff );
		unsigned int fGreatCFD_ShapingModule = (unsigned int)config->GetValue( Form( "caen_%d.CFD.ShapingTime", mod ), (double)default_GreatCFD_Shaping );
		unsigned int fGreatCFD_IntegrationModule = (unsigned int)config->GetValue( Form( "caen_%d.CFD.IntegrationTime", mod ), (double)default_GreatCFD_Integration );
		int fGreatCFD_ThresholdModule = (int)config->GetValue( Form( "caen_%d.CFD.Threshold", mod ), (int)default_GreatCFD_Threshold );
		float fGreatCFD_FractionModule = config->GetValue( Form( "caen_%d.CFD.Fraction", mod ), (double)default_GreatCFD_Fraction );

		for( unsigned int chan = 0; chan < set->GetNumberOfCAENChannels(); chan++ ){

			fCaenOffset[mod][chan] = config->GetValue( Form( "caen_%d_%d.Offset", mod, chan ), (double)fCaenOffsetModule );
			fCaenGain[mod][chan] = config->GetValue( Form( "caen_%d_%d.Gain", mod, chan ), (double)fCaenGainModule );
			fCaenGainQuadr[mod][chan] = config->GetValue( Form( "caen_%d_%d.GainQuadr", mod, chan ), (double)fCaenGainQuadrModule );
			fCaenThreshold[mod][chan] = config->GetValue( Form( "caen_%d_%d.Threshold", mod, chan ), (int)fCaenThresholdModule );
			fCaenTime[mod][chan] = config->GetValue( Form( "caen_%d_%d.Time", mod, chan ), (double)fCaenTimeModule );
			fCaenType[mod][chan] = config->GetValue( Form( "caen_%d_%d.Type", mod, chan ), TString(fCaenTypeModule) );
			fGreatMWD_Decay[mod][chan] = (unsigned int)config->GetValue( Form( "caen_%d_%d.MWD.DecayTime", mod, chan ), (double)fGreatMWD_DecayModule );
			fGreatMWD_Rise[mod][chan] = (unsigned int)config->GetValue( Form( "caen_%d_%d.MWD.RiseTime", mod, chan ), (double)fGreatMWD_RiseModule );
			fGreatMWD_Top[mod][chan] = (unsigned int)config->GetValue( Form( "caen_%d_%d.MWD.FlatTop", mod, chan ), (double)fGreatMWD_TopModule );
			fGreatMWD_Baseline[mod][chan] = (unsigned int)config->GetValue( Form( "caen_%d_%d.MWD.Baseline", mod, chan ), (double)fGreatMWD_BaselineModule );
			fGreatMWD_Window[mod][chan] = (unsigned int)config->GetValue( Form( "caen_%d_%d.MWD.Window", mod, chan ), (double)fGreatMWD_WindowModule );
			fGreatCFD_Delay[mod][chan] = (unsigned int)config->GetValue( Form( "caen_%d_%d.CFD.DelayTime", mod, chan ), (double)fGreatCFD_DelayModule );
			fGreatCFD_HoldOff[mod][chan] = (unsigned int)config->GetValue( Form( "caen_%d_%d.CFD.HoldOff", mod, chan ), (double)fGreatCFD_HoldOffModule );
			fGreatCFD_Shaping[mod][chan] = (unsigned int)config->GetValue( Form( "caen_%d_%d.CFD.ShapingTime", mod, chan ), (double)fGreatCFD_ShapingModule );
			fGreatCFD_Integration[mod][chan] = (unsigned int)config->GetValue( Form( "caen_%d_%d.CFD.IntegrationTime", mod, chan ), (double)fGreatCFD_IntegrationModule );
			fGreatCFD_Threshold[mod][chan] = (int)config->GetValue( Form( "caen_%d_%d.CFD.Threshold", mod, chan ), (int)fGreatCFD_ThresholdModule );
			fGreatCFD_Fraction[mod][chan] = (float)config->GetValue( Form( "caen_%d_%d.CFD.Fraction", mod, chan ), (double)fGreatCFD_FractionModule );

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
/// Make the MWD object can return it for a specific channel
GreatMWD GreatCalibration::DoMWD( unsigned char mod, unsigned char ch, std::vector<unsigned short> trace ) {

	// Create a FebexMWD class to hold the info
	GreatMWD mwd;

	// Check if it's a valid event first
	if( mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		// Set the parameters of the MWD
		mwd.SetTrace( trace );
		mwd.SetRiseTime( fGreatMWD_Rise[mod][ch] );
		mwd.SetDecayTime( fGreatMWD_Decay[mod][ch] );
		mwd.SetFlatTop( fGreatMWD_Top[mod][ch] );
		mwd.SetBaseline( fGreatMWD_Baseline[mod][ch] );
		mwd.SetWindow( fGreatMWD_Window[mod][ch] );
		mwd.SetDelayTime( fGreatCFD_Delay[mod][ch] );
		mwd.SetHoldOff( fGreatCFD_HoldOff[mod][ch] );
		mwd.SetShapingTime( fGreatCFD_Shaping[mod][ch] );
		mwd.SetIntegrationTime( fGreatCFD_Integration[mod][ch] );
		mwd.SetThreshold( fGreatCFD_Threshold[mod][ch] );
		mwd.SetFraction( fGreatCFD_Fraction[mod][ch] );

		// Run the MWD
		mwd.DoMWD();

	}

	return mwd;

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

// MWD
void GreatCalibration::SetMWDDecay( unsigned char mod, unsigned char ch, unsigned int decay ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		fGreatMWD_Decay[mod][ch] = decay;
		return;

	}

	else return;

}

void GreatCalibration::SetMWDRise( unsigned char mod, unsigned char ch, unsigned int rise ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		fGreatMWD_Rise[mod][ch] = rise;
		return;

	}

	else return;

}

void GreatCalibration::SetMWDFlatTop( unsigned char mod, unsigned char ch, unsigned int top ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		fGreatMWD_Top[mod][ch] = top;
		return;

	}

	else return;

}

void GreatCalibration::SetMWDBaseline( unsigned char mod, unsigned char ch, unsigned int baseline_length ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		fGreatMWD_Baseline[mod][ch] = baseline_length;
		return;

	}

	else return;

}

void GreatCalibration::SetMWDWindow( unsigned char mod, unsigned char ch, unsigned int window ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		fGreatMWD_Window[mod][ch] = window;
		return;

	}

	else return;

}

void GreatCalibration::SetCFDFraction( unsigned char mod, unsigned char ch, float fraction ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		fGreatCFD_Fraction[mod][ch] = fraction;
		return;

	}

	else return;

}

void GreatCalibration::SetCFDDelay( unsigned char mod, unsigned char ch, unsigned int delay ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		fGreatCFD_Delay[mod][ch] = delay;
		return;

	}

	else return;

}

void GreatCalibration::SetCFDHoldOff( unsigned char mod, unsigned char ch, unsigned int hold ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		fGreatCFD_HoldOff[mod][ch] = hold;
		return;

	}

	else return;

}

void GreatCalibration::SetCFDShapingTime( unsigned char mod, unsigned char ch, unsigned int shaping ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		fGreatCFD_Shaping[mod][ch] = shaping;
		return;

	}

	else return;

}

void GreatCalibration::SetCFDIntegrationTime( unsigned char mod, unsigned char ch, unsigned int integration ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		fGreatCFD_Integration[mod][ch] = integration;
		return;

	}

	else return;

}

void GreatCalibration::SetCFDThreshold( unsigned char mod, unsigned char ch, int threshold ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		fGreatCFD_Threshold[mod][ch] = threshold;
		return;

	}

	else return;

}

unsigned int GreatCalibration::GetMWDDecay( unsigned char mod, unsigned char ch ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		return fGreatMWD_Decay[mod][ch];

	}

	else return 0;

}

unsigned int GreatCalibration::GetMWDRise( unsigned char mod, unsigned char ch ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		return fGreatMWD_Rise[mod][ch];

	}

	else return 0;

}

unsigned int GreatCalibration::GetMWDFlatTop( unsigned char mod, unsigned char ch ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		return fGreatMWD_Top[mod][ch];

	}

	else return 0;

}

unsigned int GreatCalibration::GetMWDBaseline( unsigned char mod, unsigned char ch ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		return fGreatMWD_Baseline[mod][ch];

	}

	else return 0;

}

unsigned int GreatCalibration::GetMWDWindow( unsigned char mod, unsigned char ch ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		return fGreatMWD_Window[mod][ch];

	}

	else return 0;

}

float GreatCalibration::GetCFDFraction( unsigned char mod, unsigned char ch ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		return fGreatCFD_Fraction[mod][ch];

	}

	else return 0;

}

unsigned int GreatCalibration::GetCFDDelay( unsigned char mod, unsigned char ch ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		return fGreatCFD_Delay[mod][ch];

	}

	else return 0;

}

unsigned int GreatCalibration::GetCFDHoldOff( unsigned char mod, unsigned char ch ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		return fGreatCFD_HoldOff[mod][ch];

	}

	else return 0;

}

unsigned int GreatCalibration::GetCFDShapingTime( unsigned char mod, unsigned char ch ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		return fGreatCFD_Shaping[mod][ch];

	}

	else return 0;

}

unsigned int GreatCalibration::GetCFDIntegrationTime( unsigned char mod, unsigned char ch ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		return fGreatCFD_Integration[mod][ch];

	}

	else return 0;

}

int GreatCalibration::GetCFDThreshold( unsigned char mod, unsigned char ch ){

	if(   mod < set->GetNumberOfCAENModules() &&
	   ch < set->GetNumberOfCAENChannels() ) {

		return fGreatCFD_Threshold[mod][ch];

	}

	else return 0;

}

