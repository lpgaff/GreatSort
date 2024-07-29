#include "Reaction.hh"

ClassImp( GreatParticle )
ClassImp( GreatReaction )

///////////////////////////////////////////////////////////////////////////////
/// Parameterised constructor for the GreatReaction object
/// \param[in] filename A string holding the name of the reaction file
/// \param[in] myset A pointer to the GreatSettings object
/// \param[in] source A boolean to check if this run is a source run
GreatReaction::GreatReaction( std::string filename, std::shared_ptr<GreatSettings> myset, bool source ){
	
	// Read in mass tables
	ReadMassTables();
	
	// Check if it's a source run
	flag_source = source;
	
	// Get the info from the user input
	set = myset;
	SetFile( filename );
	ReadReaction();

}

///////////////////////////////////////////////////////////////////////////////
/// Adds a binding energy from a string from the mass table to the ame_be
/// private variable. Called in the GreatReaction::ReadMassTables() function
/// \param[in] Ai The mass number
/// \param[in] Zi The proton number
/// \param[in] ame_be_str The relevant line from the AME 2020 evaluation file
void GreatReaction::AddBindingEnergy( short Ai, short Zi, TString ame_be_str ) {
	
	// A key for the isotope
	std::string isotope_key;
	isotope_key = std::to_string( Ai ) + gElName.at( Zi );
	
	// Remove # from AME data and replace with decimal point
	if ( ame_be_str.Contains("#") )
		ame_be_str.ReplaceAll("#",".");
	
	// An * means there is no data, fill with a 0
	if ( ame_be_str.Contains("*") )
		ame_be.insert( std::make_pair( isotope_key, 0 ) );
	
	// Otherwise add the real data
	else
		ame_be.insert( std::make_pair( isotope_key, ame_be_str.Atof() ) );
	
	return;
	
}

///////////////////////////////////////////////////////////////////////////////
/// Stores the binding energies per nucleon for each nucleus from the AME
/// 2020 file
void GreatReaction::ReadMassTables() {
	
	// Input data file is in the source code
	// AME_FILE is passed as a definition at compilation time in Makefile
	std::ifstream input_file;
	input_file.open( AME_FILE );
	
	std::string line, BE_str, N_str, Z_str;
	std::string startline = "1N-Z";
	
	short Ai, Zi, Ni;
	
	// Loop over the file
	if( input_file.is_open() ){
		
		// Read first line
		std::getline( input_file, line );
		
		// Look for start of data
		while( line.substr( 0, startline.size() ) != startline ){
			
			// Read next line, but break if it's the end of the file
			if( !std::getline( input_file, line ) ){
				
				std::cout << "Can't read mass tables from ";
				std::cout << AME_FILE << std::endl;
				exit(1);
				
			}
			
		}
		
		// Read one more nonsense line with the units on
		std::getline( input_file, line );
		
		// Now process the data
		while( std::getline( input_file, line ) ){
			
			// Get mass excess from the line
			N_str = line.substr( 5, 5 );
			Z_str = line.substr( 9, 5 );
			BE_str = line.substr( 54, 13 );
			
			// Get N and Z
			Ni = std::stoi( N_str );
			Zi = std::stoi( Z_str );
			Ai = Ni + Zi;
			
			// Add mass value
			AddBindingEnergy( Ai, Zi, BE_str );
			
		}
		
	}
	
	else {
		
		std::cout << "Mass tables file doesn't exist: " << AME_FILE << std::endl;
		exit(1);
		
	}
	
	return;
	
}

///////////////////////////////////////////////////////////////////////////////
/// Reads the contents of the reaction file given via user input. Also calls
/// ReadStoppingPowers function for each of the nuclides going through the
/// different materials for later corrections.
void GreatReaction::ReadReaction() {
	
	TEnv *config = new TEnv( fInputFile.data() );
	
	std::string isotope_key;
	
	// Get particle properties
	Beam.SetA( config->GetValue( "BeamA", 30 ) );
	Beam.SetZ( config->GetValue( "BeamZ", 12 ) );
	if( Beam.GetZ() < 0 || Beam.GetZ() >= (int)gElName.size() ){
		
		std::cout << "Not a recognised element with Z = ";
		std::cout << Beam.GetZ() << " (beam)" << std::endl;
		exit(1);
		
	}
	Beam.SetBindingEnergy( ame_be.at( Beam.GetIsotope() ) );
	
	Eb = config->GetValue( "BeamE", 8520.0 ); // in keV/u
	Eb *= Beam.GetMass_u(); // keV
	Beam.SetEnergyLab( Eb ); // keV
	
	Target.SetA( config->GetValue( "TargetA", 2 ) );
	Target.SetZ( config->GetValue( "TargetZ", 1 ) );
	Target.SetEnergyLab( 0.0 );
	if( Target.GetZ() < 0 || Target.GetZ() >= (int)gElName.size() ){
		
		std::cout << "Not a recognised element with Z = ";
		std::cout << Target.GetZ() << " (target)" << std::endl;
		exit(1);
		
	}
	Target.SetBindingEnergy( ame_be.at( Target.GetIsotope() ) );
	
	Ejectile.SetA( config->GetValue( "EjectileA", 1 ) );
	Ejectile.SetZ( config->GetValue( "EjectileZ", 1 ) );
	if( Ejectile.GetZ() < 0 || Ejectile.GetZ() >= (int)gElName.size() ){
		
		std::cout << "Not a recognised element with Z = ";
		std::cout << Ejectile.GetZ() << " (ejectile)" << std::endl;
		exit(1);
		
	}
	Ejectile.SetBindingEnergy( ame_be.at( Ejectile.GetIsotope() ) );
	
	Recoil.SetA( config->GetValue( "RecoilA", 31 ) );
	Recoil.SetZ( config->GetValue( "RecoilZ", 12 ) );
	if( Recoil.GetZ() < 0 || Recoil.GetZ() >= (int)gElName.size() ){
		
		std::cout << "Not a recognised element with Z = ";
		std::cout << Recoil.GetZ() << " (recoil)" << std::endl;
		exit(1);
		
	}
	Recoil.SetBindingEnergy( ame_be.at( Recoil.GetIsotope() ) );
	
	
	// Some diagnostics and info
	//if( !flag_source ) {
	//
	//	//std::cout << std::endl << " +++  ";
	//	//std::cout << Beam.GetIsotope() << "(" << Target.GetIsotope() << ",";
	//	//std::cout << Ejectile.GetIsotope() << ")" << Recoil.GetIsotope();
	//	//std::cout << "  +++" << std::endl;
	//	//std::cout << "Q-value = " << GetQvalue()*0.001 << " MeV" << std::endl;
	//	//std::cout << "Incoming beam energy = ";
	//	//std::cout << Beam.GetEnergyLab()*0.001 << " MeV" << std::endl;
	//
	//}
	//else std::cout << std::endl << " +++  Source Run  +++" << std::endl;
	
	// Finished
	delete config;
	
}

