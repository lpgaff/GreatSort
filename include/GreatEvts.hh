#ifndef __GREATEVTS_HH
#define __GREATEVTS_HH

#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include "TVector2.h"
#include "TVector3.h"
#include "TObject.h"

class GreatTACEvt : public TObject {

public:
		
	// setup functions
	GreatTACEvt();
	~GreatTACEvt();
	
	void SetEvent( int mytactd, unsigned char mytacid,
				   double mytime );

	inline void SetTACTime( int t ){ tactd = t; };
	inline void SetID( unsigned char x ){ id = x; };
	inline void SetTime( double t ){ time = t; };

	inline int				GetTACTime(){ return tactd; };
	inline unsigned char	GetID(){ return id; };
	inline double			GetTime(){ return time; };

	
private:

	float			tactd;	///< TAC differences
	unsigned char	id;		///< TAC ID
	double			time;	///< time stamp of the TAC event
	
	ClassDef( GreatTACEvt, 1 );

};

class GreatGammaRayEvt : public TObject {

public:
		
	// setup functions
	GreatGammaRayEvt();
	~GreatGammaRayEvt();
	
	void SetEvent( float myenergy, unsigned char myid,
				  unsigned char mytype, double mytime );

	inline void SetEnergy( float e ){ energy = e; };
	inline void SetID( unsigned char i ){ id = i; };
	inline void SetType( unsigned char t ){ type = t; };
	inline void SetTime( double t ){ time = t; };

	inline float			GetEnergy(){ return energy; };
	inline unsigned char	GetID(){ return id; };
	inline unsigned char	GetType(){ return type; };
	inline double			GetTime(){ return time; };

	
private:

	float			energy;	///< Energy in the detector
	unsigned char	id;		///< Detector ID
	unsigned char	type;	///< Detector type: 0 - CeBr, 1 - ... HPGe?
	double			time;	///< time stamp of the event
	
	ClassDef( GreatGammaRayEvt, 1 );

};



class GreatEvts : public TObject {

public:
	
	// setup functions
	GreatEvts();
	~GreatEvts();
	
	void AddEvt( std::shared_ptr<GreatTACEvt> event );
	void AddEvt( std::shared_ptr<GreatGammaRayEvt> event );

	inline unsigned int GetTACMultiplicity(){ return tac_event.size(); };
	inline unsigned int GetGammaRayMultiplicity(){ return gamma_event.size(); };

	inline std::shared_ptr<GreatTACEvt> GetTACEvt( unsigned int i ){
		if( i < tac_event.size() ) return std::make_shared<GreatTACEvt>( tac_event.at(i) );
		else return nullptr;
	};
	inline std::shared_ptr<GreatGammaRayEvt> GetGammaRayEvt( unsigned int i ){
		if( i < gamma_event.size() ) return std::make_shared<GreatGammaRayEvt>( gamma_event.at(i) );
		else return nullptr;
	};

	void ClearEvt();
	double GetTime();

	
private:
	
	std::vector<GreatTACEvt> tac_event;
	std::vector<GreatGammaRayEvt> gamma_event;

	ClassDef( GreatEvts, 1 )
	
};

#endif

